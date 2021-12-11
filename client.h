#ifndef CLIENT_H
#define CLIENT_H
#include <QtNetwork>
#include <symbol.h>
#include <iostream>
#include <message.h>
#include "fileinfo.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject* parent = 0);
    ~Client();
    void login(QString username, QString password);
    void registration(QString username, QString password, QString nick);
    void getFiles();
    void getFile(int fileIndex);
    void closeFile(int fileIndex);
    int getSiteId();
    QTcpSocket* getSocket();
    void requestURI(int fileIndex);
    void getFileFromURI(QString uri);
    void addFile(std::shared_ptr<FileInfo> file);
    void eraseFile(int fileIndex);
    QString getUsername();
    QString getNickname();
    bool getHavePixmap();
    QPixmap getPixmap();
    QVector<std::shared_ptr<FileInfo>> getMyFileList();
    void setFileIndex(int index);
    void profileChanged(QString nickname, QPixmap image);
    void profileChanged(QString nickname);  

signals:
    void login_successful();
    void login_failed();
    void registration_successful();
    void registration_failed(int status);
    //void message_from_server(Message m, int siteIdSender);
    void messages_from_server(QVector<Message> messages, int siteIdSender);
    void files_list_refreshed(QVector<std::shared_ptr<FileInfo>> files);
    void file_ready(QVector<std::shared_ptr<Symbol>> s);
    void URI_Ready(QString uri);
    void disconnect_URI();
    void signal_connection(int siteId, QString nickname, int ins);
    void signal_owners(QMap<int, QString> owners);
    void remote_cursor_changed(int index, int siteIdSender);
    void uri_error(int operation);
    void file_erased(int index);
    void erase_file_error();
    void nickname_error(QString oldNick);
    void refresh_text_edit(QString oldNick, QString newNick);

public slots:
    void onMessageReady(QVector<Message> messages, int fileIndex);
    void onMyCursorPositionChanged(int index);
    void disconnectFromServer();

private slots:
    void onConnected();
    void onReadyRead();

private:
    QTcpSocket* socket;     //socket di connessione
    QString username;       //username del connesso
    QString nickname;       //nickname del connesso
    QString password;       //password del connesso
    bool haveImage = false; //flag che indica se possiede un'immagine di profilo
    QPixmap image;          //immagine di profilo
    int siteId;             //siteId associato all'utente
    int counter;            //contatore del numero di caratteri inseriti nel file aperto dall'utente
    int fileIndexOpened;    //indice del file che è stato aperto nella lista dei file posseduti, -1 se nessun file è aperto
    QVector<std::shared_ptr<FileInfo>> files;   //files dell'utente loggato
    QVector<std::shared_ptr<Symbol>> sVector;   //vettore di simboli usato al caricamento di un nuovo file
    QVector<Message> messagesReady;             //vettore di messaggi contenenti simboli da spedire

};

#endif // CLIENT_H
