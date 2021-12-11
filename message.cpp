#include "message.h"

Message::Message(const char action, std::shared_ptr<Symbol> s)
{
    this->action = action;
    this->s = s;
}

Message::Message()
{
}

Message::~Message()
{
}

char Message::getAction()
{
    return this->action;
}

std::shared_ptr<Symbol> Message::getSymbol()
{
    return this->s;
}

void Message::setAction(const char action)
{
    this->action=action;
}

void Message::setSymbol(std::shared_ptr<Symbol> s)
{
    this->s = s;
}
