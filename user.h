#ifndef USER_H
#define USER_H

#include <QtGui>

constexpr auto N_COLOR = 18;
class User
{
public:
    User(int siteId, QString nickname, int colorId);

    int getColorId();
    void setNickname(QString nickname);
    QString getNickname();
    QColor getColor();
    int getSiteId();
protected:
    QString nickname;   //Nickname dell'utente
    int siteId;         //SiteId dell'utente
    QColor color;       //Colore associato all'utente
    int colorId;        //Identificativo del colore associato all'utente
};

#endif // USER_H
