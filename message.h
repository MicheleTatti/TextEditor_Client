#pragma once
#include "symbol.h"

class Message       //serve per comunicare al server non solo il simbolo ma anche se deve inserirlo o cancellarlo
{
private:

    char action; //"i" per l'inserimento, "d" per la cancellazione
    std::shared_ptr<Symbol> s;

public:

    Message(const char action, std::shared_ptr<Symbol> s);

    Message();

    ~Message();

    char getAction();

    void setAction(const char action);

    std::shared_ptr<Symbol> getSymbol();

    void setSymbol(std::shared_ptr<Symbol> s);
};
