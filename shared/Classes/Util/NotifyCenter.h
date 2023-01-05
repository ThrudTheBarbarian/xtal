//
//  NotifiyCenter.h
//  xtal
//
//  Created by Thrud The Barbarian on 11/6/22.
//

/*****************************************************************************\
|* Notification center that sort of mimics the Cocoa one. Typical use might
|* be, using the NTFY_QUIT notification defined below:
|*
|* auto nc = NotificationCenter::defaultNotificationCenter();
|* nc->addObserver(std::bind(&myClass::_cb1, this), NTFY_QUIT);
|* nc->addObserver([=](NotifyData &z){_cb2(z);}, NTFY_DEBUG_ACCESS);
|*
|* void myClass::_cb1(void)
|* 	{...}
|*
|* void myClass::_cb2(NotifyData &d)
|*	{
|*	String name = d.stringValue();
|* 	...
|*	}
|*
|*
|* and in some other class, possibly on some other thread:
|*
|*	auto nc = NotificationCenter::defaultNotificationCenter();
|*	nc->notify(NTFY_QUIT);
|*
|* 	String s = "hi there";
|* 	nc->notify(NTFY_DEBUG_ACCESS, s);
|*
\*****************************************************************************/

	


#ifndef NotifiyCenter_h
#define NotifiyCenter_h

#include <cstdio>
#include <cstdint>

#include <iostream>
#include <map>
#include <functional>
#include <string>
#include <list>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

#include "properties.h"
#include "macros.h"

/*****************************************************************************\
|* Notification strings as defines to reduce typos
\*****************************************************************************/
#define NTFY_DEBUG_ACCESS		"debug-access"
#define NTFY_QUIT               "quit"

/*****************************************************************************\
|* Create the NotifyData object. Probably do this better with std::variant...
\*****************************************************************************/
typedef struct NotifyData
    {
    int 		iVal;
    String 		sVal;
    float 		fVal;
    void * 		vVal;
    uint64_t 	cron;
		
    String 		name;

    NotifyData(int x)           { setIntegerValue(x); }
    NotifyData(String x) 	  	{ setStringValue(x); }
    NotifyData(float x)         { setFloatValue(x); }
    NotifyData(void *x)         { setVoidValue(x); }
    NotifyData(uint64_t x)      { setCronValue(x); }

    inline int          integerValue()          { return iVal; }
    inline uint64_t     cronValue()             { return cron; }
    inline std::string  stringValue()           { return sVal; }
    inline float        floatValue()            { return fVal; }
    inline void *       voidValue()             { return vVal; }

    inline void setCronValue(uint64_t x)        { cron = x; }
    inline void setIntegerValue(int x)          { iVal = x; }
    inline void setStringValue(std::string x)   { sVal = x; }
    inline void setFloatValue(float x)          { fVal = x; }
    inline void setVoidValue(void *x)           { vVal = x; }
    } NotifyData;

/*****************************************************************************\
|* Callback management
\*****************************************************************************/
struct NotifyObserver
    {
    std::function<void(NotifyData& arg)> callback;
    };

class NotifyCenter
	{
	/*************************************************************************\
    |* Properties
    \*************************************************************************/
    
    private:
        static std::shared_ptr<NotifyCenter>	    		_defaultCenter;
        std::map<std::string, std::list<NotifyObserver> >	_observers;
        mutable std::recursive_mutex						_mutex;
        std::vector<NotifyData>                         	_pool;

        enum
            {
            NO_ARGS = 0,
            ONE_ARG = 1
            };
        
    public:
        /*********************************************************************\
        |* Reduce the use of auto...
        \*********************************************************************/
        typedef std::map<std::string, std::list<NotifyObserver> >::const_iterator
                notify_citr_t;
                
        typedef std::map<std::string, std::list<NotifyObserver> >::iterator
                notify_itr_t;
                
        typedef std::list<NotifyObserver>::const_iterator
                observe_citr_t;
                
        typedef std::list<NotifyObserver>::iterator observe_itr_t;


        /*********************************************************************\
        |* Add a function callback as an observer to a named notification.
        |*
        |* method : the function callback. Accepts void(NotifyData&) methods or
        |*          lambdas
        |* name	  : the name of the notification you wish to observe.
        \*********************************************************************/
        observe_citr_t addObserver(std::function<void(NotifyData& n)> method,
								   const std::string& name);

        /**********************************************************************\
        |* Add a function callback as an observer to a given notification
        |*
        |* method		: the function callback as above
		|* notification	: the notification you wish to observe.
        \**********************************************************************/
        observe_citr_t addObserver(std::function<void(NotifyData& arg)> method,
								   notify_itr_t& notification);

        /**********************************************************************\
        |* Removes an observer by name
        |*
        |* name	: the name of the notification you wish to no longer observe.
        \**********************************************************************/
        void removeObserver(const std::string& name);

        /**********************************************************************\
        |* Removes an observer by iterator
        |*
        |* notification	: the notification you wish to observe.
        |* observer		: the iterator to the observer you wish to remove
        \**********************************************************************/
        void removeObserver(notify_itr_t& notification,
							observe_citr_t& observer);

        /**********************************************************************\
        |* removes all observers from a given notification, removing the
        |* notification from being tracked outright
        |*
        |* name		: the name of the notification you wish to observe.
        \**********************************************************************/
        void removeAllObservers(const std::string& name);

        /**********************************************************************\
        |* removes all observers from a given notification, removing the
        |* notification from being tracked outright
        |*
        |* notification	: the notification you wish to observe.
        \**********************************************************************/
        void removeAllObservers(notify_itr_t& notification);

        /**********************************************************************\
        |* Posts a notification to a set of observers.
        |* If successful, this function sends a request for a delayed post of
        |* a given notification. This depends on the notification queue being
        |* polled regularly via drainNotifications(). If the delay is 0, the
        |* posting will happen the next time the notification center is drained
        |* otherwise it will happen as soon as a minimum of 'msecs' millisecs
        |* have passed
        |*
        |* name		: the name of the notification you wish to observe.
        \**********************************************************************/
        bool postNotification(const std::string& name, long msecs=0);
        bool postNotification(const std::string& name,
                              NotifyData arg,
                              long msecs=0);

        /**********************************************************************\
        |* If posted notifications are being used, this method should be called
        |* regularly (eg: once every 1/60th of a second in a game loop). It
        |* should be called on the thread that the notifications will be sent
        |* from,
        \**********************************************************************/
        void drainNotifications(void);

        /**********************************************************************\
        |* Notifies a set of observers.
        |* If successful, this function calls all callbacks associated with
        |* that notification and return true.  If no such notification exists,
        |* this function will print a warning to the console and return false
        |*
        |* name		: the name of the notification you wish to observe.
        \**********************************************************************/
        bool notify(const std::string& name) const;
        bool notify(const std::string& name, NotifyData arg) const;

        /**********************************************************************\
        |* Notifies a set of observers.
        |* If successful, this function calls all callbacks associated with
        |* that notification and return true.  If no such notification exists,
        |* this function will print a warning to the console and return false
        |*
        |* notification	: the notification you wish to observe.
        \**********************************************************************/
        bool notify(notify_citr_t& notification) const;
        bool notify(notify_citr_t& notification, NotifyData arg) const;

        /**********************************************************************\
        |* Retrieves a notification iterator for a named notification.
        |* The returned iterator may be used with the overloaded variants of
        |* postNotification, removeAllObservers, removeObserver, and addObserver
        |* to avoid string lookups
        |*
        |* notification	: the notification you wish to observe.
        \**********************************************************************/
        notify_itr_t getNotificationIterator(const std::string& name);

        /**********************************************************************\
        |* This method returns the default global notification center.  You may
        |* alternatively create your own notification center without using the
        |* default notification center
        |*
        |* notification	: the notification you wish to observe.
        \**********************************************************************/
        static std::shared_ptr<NotifyCenter> defaultNotifyCenter();
	};

#endif /* NotifiyCenter_h */
