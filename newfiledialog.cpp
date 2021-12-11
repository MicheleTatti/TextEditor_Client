#include "newfiledialog.h"
#include "ui_newfiledialog.h"

NewFileDialog::NewFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewFileDialog)
{
    ui->setupUi(this);

}

NewFileDialog::~NewFileDialog()
{
    delete ui;
}

QString NewFileDialog::getFilename(){
    return filename;
}

void NewFileDialog::on_Ok_clicked()
{
    filename = ui->filenameEdit->text();
    this->close();
}

void NewFileDialog::on_Cancel_clicked()
{
    this->close();
}

