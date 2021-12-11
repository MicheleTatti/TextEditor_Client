#include "client.h"

Client::Client(QObject* parent) : QObject(parent), counter(0)
{
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(socket, SIGNAL(disconnected()), this, SIGNAL(closed()));

    socket->connectToHost("127.0.0.1", 49002);
    //socket->connectToHost("25.69.115.147", 49002); //Andrea
    //socket->connectToHost("25.69.120.225", 49002);   //Michele

    // we need to wait...
    if (!socket->waitForConnected(5000))
        qDebug() << "Error: " << socket->errorString();
    this->fileIndexOpened = -1; //ancora nessun file è stato aperto
}

void Client::onConnected(){
    if (socket->state() != QAbstractSocket::ConnectedState)	return;

    std::cout << "Connesso al server" << std::endl;

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

int Client::getSiteId(){
    return this->siteId;
}

/**
 * @brief funzione che gestisce tutti i casi di comunicazione in arrivo da parte del server
 */
void Client::onReadyRead(){
    while (socket->bytesAvailable() != 0) {
        QVector<int> position;
        int counter, recSiteId, alignment, textSize, insert;  //INSERT: 1 se inserimento, 0 se cancellazione
        QString color, font, text, nickname;
        QChar value;
        bool isBold, isItalic, isUnderlined;
        std::shared_ptr<Symbol> s;
        QMap<int, QString> owners;
        if (socket->state() != QAbstractSocket::ConnectedState)	return;
        int operation, dim;
        QByteArray in_buf;
        QDataStream in(&in_buf, QIODevice::ReadOnly);
        QByteArray bufOut;
        QDataStream out(&bufOut, QIODevice::WriteOnly);//stream per la trasmissione
        int byteReceived = 0;

        //fondamentale per aspettare di avere ricevuto tutta un'informazione
        if (socket->bytesAvailable() < (qint64)sizeof(int)) {
            socket->waitForReadyRead();
        }

        in_buf = socket->read((qint64)sizeof(int));
        in >> dim;
        while (byteReceived < dim) {
            if (!socket->bytesAvailable()) {
                socket->waitForReadyRead();
            }
            in_buf.append(socket->read((qint64)dim - (qint64)byteReceived));
            byteReceived = in_buf.size() - sizeof(int);
        }

        in >> operation;
        switch(operation){
        //caso di login, scopro se è riuscito o fallito ed emetto le signal corrispondenti
        case 0:
            int status;
            in >> status;
            if(status == 0){
                login_failed();
            }else if(status == 1){
                int op;
                in >> op;
                if(op == 1) {
                    in >> this->username >> this->nickname;
                }
                else if(op == 2) {
                    QImage image;
                    in >> this->username >> this->nickname >> image;
                    this->image.convertFromImage(image);
                    haveImage = true;
                }
                login_successful();
            }
            break;
        //caso per la registrazione, scopro se è riuscita o fallita ed emetto le signal corrispondenti
        case 1:
            std::cout<< "registration\n";
            int statusReg;
            in >> statusReg;
            if(statusReg!=1){
                registration_failed(statusReg);
            }else{
                in >> this->siteId >> this->username >> this->nickname;
                registration_successful();
            }
            break;
        //caso per la gestione di inserimento o cancellazione di uno o più simboli
        case 3:
        {
            int siteIdSender=-1;
            //qDebug() << "3)Mandato dal server dopo l'inserimento o la cancellazione di un simbolo";
            int n_sym;
            QVector<Message> messages;
            in >> insert;
            if(insert==0){
                in >> siteIdSender;
            }
            /*if(insert==2){ //arrivo ack, richiesta blocco successivo
                QVector<Message> messages;
                onMessageReady(messages, fileIndexOpened);
                break;
            }*/
            in >> n_sym;
            for(int i=0; i<n_sym; i++){
                in >> recSiteId >> counter >> position >> value >>  isBold >> isItalic >> isUnderlined >> alignment >> textSize >> color >> font;
                s = std::make_shared<Symbol>(Symbol(position, counter, recSiteId, value, isBold, isItalic, isUnderlined, alignment, textSize, color, font));
                if(insert==1){ //nel caso sia un inserimento
                    if( recSiteId != this->siteId){ //il simbolo non l'ho aggiunto io.
                        Message m{'i', s};
                        messages.push_back(m);
                     //   message_from_server(m, siteIdSender); // ****FORSE QUI SAREBBE MEGLIO AGGIUNGERE IL FILENAME PER ESSERE SICURI DELL'INSERIMENTO*****
                    }
                }else{ //nel caso sia una cancellazione
                    Message m{'d', s};
                    messages.push_back(m);
                    //message_from_server(m, siteIdSender);
                }
            }
            if(messages.size() > 0){
              messages_from_server(messages, siteIdSender);
            }
            break;
        }
        //caso per la ricezione di un file già esistente alla sua apertura
        case 4:
            //qDebug() <<  "4)Dobbiamo gestire la ricezione di un file già scritto.";
            sVector.clear();
            int fileSize;
            int alreadyConnected;
            int siteId;
            int otherOwners;
            in >> fileSize;
            for(int i=0; i<fileSize; i++){
                in  >>insert >> position >> counter >> recSiteId >> value >> isBold >> isItalic >> isUnderlined >> alignment >> textSize >> color >> font;
                s = std::make_shared<Symbol>(Symbol(position, counter, recSiteId, value, isBold, isItalic, isUnderlined, alignment, textSize,color, font));
                sVector.push_back(s);
            }
            if(fileSize!=0){
                emit file_ready(sVector);
            }
            //ottengo i dati sugli utenti attualmente connessi a questo file
            in >> alreadyConnected;
            for(int i = 0; i<alreadyConnected; i++){
                in >> siteId >> nickname;
                emit signal_connection(siteId, nickname, 1);
            }
            //ottengo i dati sugli utenti che possono lavorare a questo file
            in >> otherOwners;
            for(int i=0; i<otherOwners; i++){
                in >> siteId >> nickname;
                owners.insert(siteId, nickname);
            }
            emit signal_owners(owners);
            break;
        //caso per la ricezione dei nomi di file disponibili ad essere editati dall'utente nella filesSelection
        case 6:
            int numFiles;
            in >> status;
            if(status == 1){  //riceviamo i file.
                in >>this->siteId >> numFiles;
                files.clear();
                for(int i=0; i<numFiles; i++){
                    QString filename;
                    QString usernameOwner; //riceve lo username del creatore
                    QString nicknameOwner;
                    in >> filename >> usernameOwner >> nicknameOwner;
                    std::shared_ptr<FileInfo> file (new FileInfo(filename, usernameOwner, nicknameOwner));
                    files.append(file);
                }
                files_list_refreshed(files);
            }
            break;
        //caso per la gestione del ritorno della richiesta di condivisione con me di un file
        case 7:
            int operation;
            in >> operation;
            if(operation == 1){  //La condivisione è andata a buon fine, quindi aggiungo il nuovo file alla lista
                QString filename;
                QString usernameOwner; //riceve lo username del creatore
                QString nicknameOwner;
                in >> filename >> usernameOwner >> nicknameOwner;
                std::shared_ptr<FileInfo> file (new FileInfo(filename, usernameOwner, nicknameOwner));
                files.append(file);
                files_list_refreshed(files);
            }  else if(operation == 2){
                QString uri;
                in >> uri;
                URI_Ready(uri);
            }
            else if (operation == 3 || operation == 4) {
                uri_error(operation);
            }
            break;
        //caso per la gestione di una nuova connessione o disconnessione di un altro utente allo stesso file che ho io aperto
        case 8:
            int ins;
            in >> siteId >> nickname >> ins; //ins 0 rimuovi, 1 inserisci
            emit signal_connection(siteId, nickname, ins);
            break;
        //caso per gestire la notifica di avvenuta cancellazione di un file da parte di un utente
        case 9:
            in >> status;
            if(status == 1) { //cancellazione riuscita (lato server), eliminiamo e chiudiamo (se era aperto) il file
                QString filename;
                QString usernameOwner;
                in >> filename >> usernameOwner;
                for(std::shared_ptr<FileInfo> f : files){
                    if( f.get()->getFileName() == filename && f.get()->getUsername() == usernameOwner){
                        int index = files.indexOf(f);
                        files.removeOne(f);
                        file_erased(index);
                        break;
                    }
                }
            }
            else {
                erase_file_error();
            }
            break;
        //caso per la gestione di cambio nickname
        case 10:
        {
            int op;
            QString oldNick;
            in >> op >> oldNick;
            if(op == 1) {
                QString newNick;
                in >> newNick;
                for(std::shared_ptr<FileInfo> file : files) {
                    if(file->getNickname() == oldNick) {
                        file->setNickname(newNick);
                    }
                }
                if(this->nickname == oldNick) {
                    this->nickname = newNick;
                }
                files_list_refreshed(files);
                refresh_text_edit(oldNick, newNick);
            }
            else if(op == 2) {
                nickname_error(oldNick);
            }
            break;
        }
        //caso per gestire il cambiamento di posizione del puntatore di un altro utente connesso al mio stesso file
        case 11:
        {
            int siteIdSender, cursorIndex;
            QString filepath;
            in >> filepath >> cursorIndex >> siteIdSender;
            if(this->fileIndexOpened != -1 && filepath == this->files[this->fileIndexOpened]->getFilePath())
                remote_cursor_changed(cursorIndex, siteIdSender);
            break;
        }
        default: break;
        }
    }

}
/**
 * @brief notifico il server che ho chiuso l'editor con un certo file aperto
 * @param fileIndex
 */
void Client::closeFile(int fileIndex){
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
    out << 5 /*# operazione*/ << files[fileIndex].get()->getFileName() << files[fileIndex].get()->getUsername();
    out_stream << buf;
    socket->write(bufOut);
    socket->flush();
    fileIndexOpened = -1;
    files_list_refreshed(files);
    disconnect_URI();
}

Client::~Client(){

}
/**
 * @brief manda al server la richiesta di login da parte di un certo utente
 * @param username: username dell'utente
 * @param password: password dell'utente
 */
void Client::login(QString username, QString password){
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
    out << 0 << username << password;
    out_stream << buf;
    socket->write(bufOut);
    socket->flush();
}

/**
 * @brief manda al server la richiesta di registrazione da parte di un certo utente
 * @param username: username scelto dall'utente
 * @param password: password scelta dall'utente
 * @param nickName: nickname scelto dall'utente
 */
void Client::registration(QString username, QString password, QString nickName){
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
    out << 1 << username << password << nickName;
    out_stream << buf;
    socket->write(bufOut);
    socket->flush();
}

/**
 * @brief richiesta al server dei file editabili dall'utente loggato
 */
void Client::getFiles(){
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
    out << 6;
    out_stream << buf;
    socket->write(bufOut);
    socket->flush();
    return; // i file vengono inviati dalla signal list_files_refreshed
}


QVector<std::shared_ptr<FileInfo>> Client::getMyFileList(){
    return this->files;
}


/*
 * La addFile
 */
/**
 * @brief aggiunge alla lista di files un nuovo file e richiama la funzione per la ricezione di un file dal server
 * @param file: file da aggiungere
 */
void Client::addFile(std::shared_ptr<FileInfo> file){
    files.append(file);
    int size = files.size() - 1;
    getFile(size);
}

void Client::setFileIndex(int index){
    fileIndexOpened = index;
}

/**
 * @brief comunica al server la decisione di cancellare un file
 * @param fileIndex: indice del file da eliminare
 */
void Client::eraseFile(int fileIndex) {
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
    std::shared_ptr<FileInfo> file = files[fileIndex];
    out << 9 << file.get()->getFileName() << file.get()->getUsername();
    out_stream << buf;
    socket->write(bufOut);
}

/**
 * @brief notifica al server il cambiamento di informazioni di profilo da parte dell'utente, se cambio anche la foto
 * @param nickname: nickname dell'utente
 * @param image: immagine di profilo dell'utente
 */
void Client::profileChanged(QString nickname, QPixmap image) {
    if(this->nickname != nickname || this->image != image) {
        this->image = image;
        this->haveImage = true;
        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        QByteArray bufOut;
        QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
        QImage img = image.toImage();
        out << 10 << 1 << username << nickname << img;
        out_stream << buf;
        socket->write(bufOut);
    }
}
/** notifica al server il cambiamento di informazioni di profilo da parte dell'utente, se cambio solo il nickname
 * @brief Client::profileChanged
 * @param nickname
 */
void Client::profileChanged(QString nickname) {
    if(this->nickname != nickname) {
        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        QByteArray bufOut;
        QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
        out << 10 << 2 << username << nickname;
        out_stream << buf;
        socket->write(bufOut);
    }
}


/**
 * @brief richiede al server un file
 * @param fileIndex: indice del file richiesto
 */
void Client::getFile(int fileIndex){

    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
    std::shared_ptr<FileInfo> file = files[fileIndex];
    out << 4 << file->getFileName() << file->getUsername() << siteId;
    out_stream << buf;
    fileIndexOpened = fileIndex;
    socket->write(bufOut);
}

void Client::disconnectFromServer(){
    socket->disconnectFromHost();
}

/**
 * @brief manda al server la coda di messaggi contenente caratteri da inserire o rimuovere
 * @param messages: messaggi da inviare
 * @param fileIndex: indice del file a cui si riferiscono i messaggi
 */
void Client::onMessageReady(QVector<Message> messages, int fileIndex){
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
    std::shared_ptr<FileInfo> fi = files[fileIndex];
    out << 3;
    if(messages[0].getAction()=='i'){
        out << 1 << fi->getFileName() << fi->getUsername() << messages.size();
        for(int i=0; i< messages.size(); i++){
            std::shared_ptr<Symbol> s = messages[i].getSymbol();
            out << s ->getSiteId() << s->getCounter() << s->getPosition() << s->getValue() << s->isBold()
                << s->isItalic() << s->isUnderlined() << s->getAlignment() << s->getTextSize() << s->getColor() << s->getFont();
        }
    }else{
        if(messages[0].getAction()=='d'){
          out <<  0 << fi->getFileName() << fi->getUsername() << messages.size();
            for(int i=0; i< messages.size(); i++){
                std::shared_ptr<Symbol> s = messages[i].getSymbol();
                out << s->getSiteId() << s->getCounter()  << s->getPosition();
            }
        }
    }
    out_stream << buf;
    socket->write(bufOut);
    socket->flush();
}


    /*    messagesReady.append(messages);
    if(messagesReady.size()==0){
        return;
    }

    FileInfo * fi = files[fileIndex];
    QByteArray buf_header;
    QDataStream out_header(&buf_header, QIODevice::WriteOnly);
    QByteArray buf_payload;
    QDataStream out_payload(&buf_payload, QIODevice::WriteOnly);
    QByteArray tot;
    int counter = 0;
    if(messagesReady[0].getAction()=='i'){
       while(!messagesReady.isEmpty()){
            if(messagesReady[0].getAction()!='i'){ //non stiamo ancora trattando inserimenti
                return;
            }
            counter++;
            Symbol *s = messagesReady[0].getSymbol();
            out_payload << s ->getSiteId() << s->getCounter() << s->getPosition() << s->getValue() << s->isBold()
                            << s->isItalic() << s->isUnderlined() << s->getAlignment() << s->getTextSize() << s->getColor() << s->getFont();
            if(buf_payload.size() > 50000){ //max buff 65532
                 out_header << 3 << 1 << fi->getFileName() << fi->getUsername() << counter;
                 tot.append(buf_header);
                 tot.append(buf_payload);
                 socket->write(tot);
                 socket->flush();
                 return;
            }
            messagesReady.remove(0);
       }
       out_header << 3 << 1 << fi->getFileName() << fi->getUsername() << counter;
       tot.append(buf_header);
       tot.append(buf_payload);
       socket->write(tot);
       socket->flush();
    }else{
        if(messages[0].getAction()=='d'){
            while(!messagesReady.isEmpty()){
                 if(messagesReady[0].getAction()!='d'){ //non stiamo ancora trattando cancellazioni
                     return;
                  }
                 counter++;
                 Symbol *s = messagesReady[0].getSymbol();
                 out_payload << s->getSiteId() << s->getCounter()  << s->getPosition();

                if(buf_payload.size() > 50000){ //max buff 65532
                    out_header << 3 << 0 << fi->getFileName() << fi->getUsername() << counter;
                    tot.append(buf_header);
                    tot.append(buf_payload);
                    socket->write(tot);
                    socket->flush();
                    return;
                }
                messagesReady.remove(0);
           }
           out_header << 3 << 0 << fi->getFileName() << fi->getUsername() << counter;
           tot.append(buf_header);
           tot.append(buf_payload);
           socket->write(tot);
           socket->flush();
        }
    }
}*/




QTcpSocket* Client::getSocket(){
    return socket;
}

/**
 * @brief richiedo al server l'URI di un file
 * @param fileIndex: indice del file di cui chiedo l'URI
 */
void Client::requestURI(int fileIndex){
   // qDebug() << files[fileIndex].get();
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);

    out << 7 << 2 << files[fileIndex]->getFileName() <<files[fileIndex]->getUsername();
    out_stream << buf;
    socket->write(bufOut);
    socket->flush();
}

/**
 * @brief invio al server la posizione aggiornata del mio cursore
 * @param index: posizione del cursore
 */
void Client::onMyCursorPositionChanged(int index){
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);

    out << 11 << index;
    out_stream << buf;
    socket->write(bufOut);
    socket->flush();
}

/**
 * @brief chiedo al server un file tramite la sua Uri
 * @param uri: uri del file
 */
void Client::getFileFromURI(QString uri) {
    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    QByteArray bufOut;
    QDataStream out_stream(&bufOut, QIODevice::WriteOnly);
    out << 7 << 1 << uri;
    out_stream << buf;
    socket->write(bufOut);
}

QString Client::getNickname(){
    return nickname;
}

QString Client::getUsername(){
    return username;
}

bool Client::getHavePixmap() {
    return haveImage;
}

QPixmap Client::getPixmap() {
    return image;
}
