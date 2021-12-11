#pragma once

#include <qvector.h>
#include <QtGui>

class Symbol
{
public:
    Symbol(QVector<int>& position, int counter, int siteId, QChar value, bool bold, bool italic, bool underlined, int alignment,
           int textSize, QColor color, QString font);
    Symbol();
    //Symbol(const Symbol &s);
    virtual ~Symbol();
	QVector<int>& getPosition();
	void setPosition(QVector<int> position);
	int getCounter();
	int getSiteId();
    bool equals(std::shared_ptr<Symbol> s);
    QChar getValue();
    bool isBold();
    bool isItalic();
    bool isUnderlined();
    int getAlignment();
    int getTextSize();
    QColor& getColor();
    QString& getFont();
protected:
    QChar value;            //Carattere del simbolo
    QVector<int> position;  //Vettore che identifica la posizione nel testo
    int counter;            //Contatore che dice che questo è l'ennesimo carattere inserito dall'utente
    int siteId;             //SiteId dell'utente che ha inserito il simbolo
    bool bold;              //Booleano, true se carattere è grassetto
    bool italic;            //Booleano, true se carattere è corsivo
    bool underlined;        //Booleano, true se carattere è sottolineato
    int alignment;          //Identifica allineamento testo (destra, sinistra, giustificato..)
    int textSize;           //Dimensione del carattere
    QColor color;           //Colore del carattere
    QString font;           //Famiglia del font del carattere
};

