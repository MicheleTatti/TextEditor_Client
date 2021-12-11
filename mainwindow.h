#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <client.h>
#include <filesselection.h>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void closing();

private slots:
    void on_loginButton_clicked();
    void on_registrationButton_clicked();
    void onLoginSuccess();
    void onLoginFailed();
    void onRegistrationSuccess();
    void onRegistrationFailed(int status);

private:
    Ui::MainWindow *ui;
    std::shared_ptr<Client> client;     //client che fa tutte le operazioni di comunicazione

    void closeEvent(QCloseEvent *event);
};
#endif // MAINWINDOW_H
