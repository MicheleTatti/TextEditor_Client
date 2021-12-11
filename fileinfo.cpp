#include "fileinfo.h"

FileInfo::FileInfo(QString filename, QString usernameOwner, QString nicknameOwner)
    :filename(filename),usernameOwner(usernameOwner),nicknameOwner(nicknameOwner)
{

}

QString FileInfo::getFileName(){
    return filename;
}
QString FileInfo::getUsername(){
    return usernameOwner;
}
QString FileInfo::getNickname(){
    return nicknameOwner;
}
QString FileInfo::getFilePath(){
    return usernameOwner + "/" + filename;
}

void FileInfo::setNickname(QString nickname) {
    this->nicknameOwner = nickname;
}
