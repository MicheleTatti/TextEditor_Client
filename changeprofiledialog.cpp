#include "changeprofiledialog.h"
#include "ui_changeprofiledialog.h"

changeProfileDialog::changeProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::changeProfileDialog)
{
    ui->setupUi(this);
}

changeProfileDialog::~changeProfileDialog()
{
    delete ui;
}

QPixmap changeProfileDialog::getImagePixmap(){
    return imagePixmap;
}

QString changeProfileDialog::getNickname() {
    return nickname;
}

void changeProfileDialog::setNickname(QString nick) {
    this->nickname = nick;
    ui->Nickname->setText(nick);
}

void changeProfileDialog::setPixmap(QPixmap pixmap) {
    this->imagePixmap = pixmap;
    ui->label_3->setPixmap(imagePixmap);
    havePixmap = true;
}

bool changeProfileDialog::getHavePixmap() {
    return havePixmap;
}

void changeProfileDialog::on_selectImageButton_clicked() {
    QString imagePath = QFileDialog::getOpenFileName(this, tr("Apri file"), "",tr("JPEG (*.jpg *.jpeg);;PNG (*.png)" ));
    QImage image;
    if(imagePath != nullptr) {
        image.load(imagePath);
        int imgsize = fmin(image.width(), image.height());
        QRect rect = QRect((image.width() - imgsize) / 2, (image.height() - imgsize) / 2, imgsize, imgsize);
        image = image.copy(rect);
        QPixmap pixmap;
        pixmap.convertFromImage(image.scaled(ui->label_3->size()));
        ui->label_3->setPixmap(pixmap);
        havePixmap = true;
        imagePixmap=pixmap;
    }
}

void changeProfileDialog::on_buttonBox_accepted()
{
    nickname = ui->Nickname->text();
    this->close();
}

void changeProfileDialog::on_buttonBox_rejected()
{
    this->close();
}
