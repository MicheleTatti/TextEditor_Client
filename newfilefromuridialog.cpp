#include "newfilefromuridialog.h"
#include "ui_newfilefromuridialog.h"

NewFileFromURIdialog::NewFileFromURIdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewFileFromURIdialog)
{
    ui->setupUi(this);
}

NewFileFromURIdialog::~NewFileFromURIdialog()
{
    delete ui;
}

QString NewFileFromURIdialog::getUri(){
    return hashedfilename;
}

void NewFileFromURIdialog::on_buttonBox_accepted()
{
    hashedfilename = ui->lineEdit->text();
    this->close();
}

void NewFileFromURIdialog::on_buttonBox_rejected()
{
    this->close();
}
