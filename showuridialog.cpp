#include "showuridialog.h"
#include "ui_showuridialog.h"

ShowUriDialog::ShowUriDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowUriDialog)
{
    ui->setupUi(this);
}
/**
 * @brief Setta la stringa desiderata nella lineEdit della dialog
 * @param uri: stringa identificativa dell'uri
 */
void ShowUriDialog::setUri(QString uri) {
    ui->lineEdit->setText(uri);
}

ShowUriDialog::~ShowUriDialog()
{
    delete ui;
}
