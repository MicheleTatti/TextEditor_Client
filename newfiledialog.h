#ifndef NEWFILEDIALOG_H
#define NEWFILEDIALOG_H

#include <QDialog>

namespace Ui {
class NewFileDialog;
}

class NewFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewFileDialog(QWidget *parent = nullptr);
    ~NewFileDialog();
    QString getFilename();

private slots:
    void on_Ok_clicked();
    void on_Cancel_clicked();

private:
    Ui::NewFileDialog *ui;
    QString filename;
};

#endif // NEWFILEDIALOG_H
