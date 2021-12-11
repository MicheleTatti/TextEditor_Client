#ifndef FILESSELECTION_H
#define FILESSELECTION_H

#include <QMainWindow>
#include <client.h>
#include <newfiledialog.h>
#include <newfilefromuridialog.h>
#include "texteditor/textedit.h"
#include <QListWidgetItem>
#include <QTableWidgetItem>

namespace Ui {
class FilesSelection;
}

class FilesSelection : public QMainWindow
{
    Q_OBJECT

public:
    explicit FilesSelection(QWidget *parent = nullptr, std::shared_ptr<Client> client=nullptr);
    ~FilesSelection();

signals:
    void closing();

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void on_newDocumentButton_clicked();
    void on_newFileFromLink_clicked();
    void on_changeProfileButton_clicked();
    void showWindow();
    void on_fileListWidget_itemDoubleClicked(QListWidgetItem *item);
    void onFilesListRefreshed(QVector<std::shared_ptr<FileInfo>> files);
    void showContextMenu(const QPoint&);
    void onShareURIButtonPressed();
    void onEraseFileButtonPressed();
    void onURIReady(QString uri);
    void onUriError(int operation);
    void onFileErased(int index);
    void onEraseFileError();
    void onNicknameError(QString oldNick);

private:
    Ui::FilesSelection *ui;
    std::shared_ptr<Client> client;     //client che fa tutte le operazioni di comunicazione, passato dalla classe MainWindow
    bool uriRequest = false;            //flag usato per identificare se sto facendo richiesta di una uri

    void setUriRequest(bool status);
};

#endif // FILESSELECTION_H
