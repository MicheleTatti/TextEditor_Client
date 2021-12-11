#ifndef NEWFILEFROMURIDIALOG_H
#define NEWFILEFROMURIDIALOG_H

#include <QDialog>

namespace Ui {
class NewFileFromURIdialog;
}

class NewFileFromURIdialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewFileFromURIdialog(QWidget *parent = nullptr);
    ~NewFileFromURIdialog();
    QString getUri();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();


private:
    Ui::NewFileFromURIdialog *ui;
    QString hashedfilename;         //hash corrispondente al filename
};

#endif // NEWFILEFROMURIDIALOG_H
