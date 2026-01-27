#include "eventmanager.h"

Q_GLOBAL_STATIC(EventManager,eventManager);
EventManager::EventManager(QObject *parent)
    : QObject{parent}
{

}

EventManager *EventManager::GetInstance()
{
    return eventManager();
}
