#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    client = std::make_shared<Client>();
    QObject::connect(client.get(),  SIGNAL(login_successful()), this, SLOT(onLoginSuccess()));
    QObject::connect(client.get(),  SIGNAL(login_failed()), this, SLOT(onLoginFailed()));
    QObject::connect(client.get(),  SIGNAL(registration_successful()), this, SLOT(onRegistrationSuccess()));
    QObject::connect(client.get(),  &Client::registration_failed, this, &MainWindow::onRegistrationFailed);
    QObject::connect(this,  SIGNAL(closing()), client.get(), SLOT(disconnectFromServer()));

}

MainWindow::~MainWindow()
{
    delete ui;
}
/**
 * @brief setta le variabili inserite nel form e chiama la login da parte del client
 */
void MainWindow::on_loginButton_clicked()
{

    QString username = ui->loginUsername->text();
    QString password = ui->loginPassword->text();
    if(username.isEmpty() || password.isEmpty()){
        QMessageBox::information(this,"Login","tutti i campi del form devono essere compilati");
        return;
    }
    client.get()->login(username, password);

}
/**
 * @brief se la login ha avuto successo crea la finestra di selezione file, la mostra e nasconde la finestra di login
 */
void MainWindow::onLoginSuccess(){
    hide();
    FilesSelection *fs = new FilesSelection(nullptr, client);
    fs->show();
}
/**
 * @brief se la registrazione ha avuto successo crea la finestra di selezione file, la mostra e nasconde la finestra di login
 */
void MainWindow::onRegistrationSuccess(){
    hide();
    FilesSelection *fs = new FilesSelection(nullptr, client);
    fs->show();
}
/**
 * @brief setta le variabili inserite nel form e chiama la registrazione da parte del client
 */
void MainWindow::on_registrationButton_clicked()
{
    QString username = ui->registrationUsername->text();
    QString nick = ui->registrationNickname->text();
    QString password = ui->registrationFirstPassword->text();
    QString password2 = ui->registrationSecondPassword->text();
    //Gestione di errori di password ripetute errate
    if(password != password2){
       QMessageBox::information(this,"Registrazione","le password non coincidono");
       return;
    }
    //Errore se non ho compilato tutti i campi
    if(password.isEmpty() || password2.isEmpty() || username.isEmpty() || nick.isEmpty()){
       QMessageBox::information(this,"Registrazione","tutti i campi del form devono essere compilati");
       return;
    }
    //Non accetto caratteri strani per il campo username
    if(username.contains("/") || username.contains("\\") || username.contains(":") ||
       username.contains("*") || username.contains("?") || username.contains("\"") ||
       username.contains("<") || username.contains(">") || username.contains("|") || username.contains(" ")){
        QMessageBox::warning(this,"Registrazione","L'username non può contenere i seguenti caratteri: / \\ : * ? \" < > | 'spazio'");
        return;
    }

    client->registration(username,password,nick);
}
/**
 * @brief se la login non ha avuto successo mostro un messaggio a schermo e pulisco il campo password
 */
void MainWindow::onLoginFailed(){
    QMessageBox::information(this,"Login","Operazione di login non riuscita");
    ui->loginPassword->clear();
    return;
}
/**
 * @brief se la registrazione è fallita, mostro l'errore corrispondente e pulisco i campi appositi
 * @param status
 */
void MainWindow::onRegistrationFailed(int status){

    if(status == 2){
        QMessageBox::information(this,"Registrazione","Username già esistente!");
        ui->registrationFirstPassword->clear();
        ui->registrationSecondPassword->clear();
        ui->registrationNickname->clear();
        ui->registrationUsername->clear();
        return;
    }else if(status == 3){
        QMessageBox::information(this,"Registrazione","Nickname già esistente!");
        ui->registrationFirstPassword->clear();
        ui->registrationSecondPassword->clear();
        ui->registrationNickname->clear();
        return;
    }else{
        QMessageBox::information(this,"Registrazione","Operazione di registrazione non riuscita");
        ui->registrationFirstPassword->clear();
        ui->registrationSecondPassword->clear();
        ui->registrationNickname->clear();
        ui->registrationUsername->clear();
        return;

    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
   emit closing();
   event->accept();
}
