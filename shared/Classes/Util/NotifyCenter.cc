//
//  NotifyCenter.cc
//  xtal
//
//  Created by Thrud The Barbarian on 11/6/22.
//

#include "NotifyCenter.h"

#define LOCK(x)     std::lock_guard<std::recursive_mutex> lock(x);

/******************************************************************************\
|* The global notification center
\******************************************************************************/
std::shared_ptr<NotifyCenter> NotifyCenter::_defaultCenter = NULL;



/******************************************************************************\
|* Add an observer
\******************************************************************************/
NotifyCenter::observe_citr_t
NotifyCenter::addObserver(std::function<void(NotifyData& arg)> method,
                                const std::string& name)
    {
    LOCK(_mutex);
    NotifyObserver n;
    n.callback = method;
    _observers[name].push_back(n);
    return --_observers[name].end();
    }

/******************************************************************************\
|* Add an observer
\******************************************************************************/
NotifyCenter::observe_citr_t
NotifyCenter::addObserver(std::function<void (NotifyData& arg)> method,
						   notify_itr_t &notification)
    {
    LOCK(_mutex);
    auto retVal = notification->second.end();
    if (notification != _observers.end())
        {
        NotifyObserver n;
        n.callback = method;
        notification->second.push_back(n);
        retVal = --notification->second.end();
        }
    return retVal;
    }


/******************************************************************************\
|* Remove an observer
\******************************************************************************/
void NotifyCenter::removeObserver(const std::string& name)
    {
    LOCK(_mutex);
    notify_citr_t i = _observers.find(name);
    if (i != _observers.end())
        _observers.erase(i);
    }

/******************************************************************************\
|* Remove an observer
\******************************************************************************/
void NotifyCenter::removeObserver(notify_itr_t& notification,
                    std::list<NotifyObserver>::const_iterator& observer)
    {
    LOCK(_mutex);
    if (notification != _observers.end())
        notification->second.erase(observer);
    }

/******************************************************************************\
|* Remove all observers
\******************************************************************************/
void NotifyCenter::removeAllObservers(const std::string& name)
    {
    LOCK(_mutex);
    _observers.erase(name);
    }

/******************************************************************************\
|* Remove all observers
\******************************************************************************/
void NotifyCenter::removeAllObservers(notify_itr_t& notification)
    {
    LOCK(_mutex);
    if (notification != _observers.end())
        _observers.erase(notification);
    }

/******************************************************************************\
|* Notify all the observers immediately
\******************************************************************************/
bool NotifyCenter::notify(const std::string& notification) const
    {
    return notify(notification, NotifyData(nullptr));
    }

bool NotifyCenter::notify(const std::string& notification, NotifyData arg) const
    {
    LOCK(_mutex);
    auto i = _observers.find(notification);
    if (i != _observers.end())
        {
        const std::list<NotifyObserver>& notiList = i->second;
        for (auto ia = notiList.begin();
             ia != notiList.end();
             ia++)
            ia->callback(arg);
        return true;
        }
    else
        {
        printf("WARNING: Notification \"%s\" does not exist.\n",
                notification.data());
        return false;
        }
    }

/******************************************************************************\
|* Notify all the observers immediately
\******************************************************************************/
bool NotifyCenter::notify(notify_citr_t& notification) const
    {
    return notify(notification, NotifyData(nullptr));
    }

bool NotifyCenter::notify(notify_citr_t& notification, NotifyData arg) const
    {
    LOCK(_mutex);
    if (notification != _observers.end())
        {
        const std::list<NotifyObserver>& notiList = notification->second;
        for (observe_citr_t i = notiList.begin();
             i != notiList.end();
             i++)
            i->callback(arg);
        return true;
        }
    else
        {
        printf("WARNING: Notification \"%s\" does not exist.\n",
                notification->first.data());
        return false;
        }
    }

/******************************************************************************\
|* Get an iterator
\******************************************************************************/
NotifyCenter::notify_itr_t
NotifyCenter::getNotificationIterator(const std::string& notification)
    {
    notify_itr_t retVal;
    if (_observers.find(notification) != _observers.end())
        retVal = _observers.find(notification);

    return retVal;
    }

/******************************************************************************\
|* Return the default notification center
\******************************************************************************/
std::shared_ptr<NotifyCenter> NotifyCenter::defaultNotifyCenter()
    {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (!_defaultCenter)
        _defaultCenter = std::shared_ptr<NotifyCenter>(new NotifyCenter);


    return _defaultCenter;
    }

/******************************************************************************\
|* Post a notification to the queue
\******************************************************************************/
bool NotifyCenter::postNotification(const std::string& name, long msecs)
    {
    return postNotification(name, NotifyData(nullptr), msecs);
    }

bool NotifyCenter::postNotification(const std::string& name,
                                          NotifyData arg,
                                          long msecs)
    {
    LOCK(_mutex);

    arg.name = name;
    arg.cron = std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch()).count()
                + msecs;
    _pool.push_back(arg);
    return true;
    }


/******************************************************************************\
|* Drain any enqueued notifications. Must be called frequently if notifications
|* are to be enqueud
\******************************************************************************/
void NotifyCenter::drainNotifications(void)
    {
    LOCK(_mutex);

    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();

    std::vector<NotifyData> remaining;

    for (auto& i : _pool)
        if (i.cron < now)
            notify(i.name, i);
        else
            remaining.push_back(i);

    _pool.clear();
    _pool = remaining;
    }
