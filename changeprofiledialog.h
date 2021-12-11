#ifndef CHANGEPROFILEDIALOG_H
#define CHANGEPROFILEDIALOG_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class changeProfileDialog;
}

class changeProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit changeProfileDialog(QWidget *parent = nullptr);
    ~changeProfileDialog();
    QPixmap getImagePixmap();
    QString getNickname();
    void setNickname(QString nick);
    void setPixmap(QPixmap pixmap);
    bool getHavePixmap();

private slots:
    void on_selectImageButton_clicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();



private:
    Ui::changeProfileDialog *ui;
    QString nickname;               //nickname dell'utente
    QPixmap imagePixmap;            //immagine profilo dell'utente
    bool havePixmap = false;        //flag che indica se l'utente ha un immagine profilo
};

#endif // CHANGEPROFILEDIALOG_H
