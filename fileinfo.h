#ifndef FILEINFO_H
#define FILEINFO_H
#include <QtNetwork>
#include <symbol.h>
#include <iostream>
#include <message.h>



class FileInfo: public QObject
{
    Q_OBJECT

public:
    FileInfo(QString filename, QString usernameOwner, QString nicknameOwner);
    QString getFileName();
    QString getUsername();
    QString getNickname();
    QString getFilePath();
    void setNickname(QString nickname);

private:
    QString filename;       //nome del file
    QString usernameOwner;  //username del creatore del file
    QString nicknameOwner;  //nickname del creatore del file

};

#endif // FILEINFO_H
