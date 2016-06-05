#ifndef __notify_base_h__
#define __notify_base_h__

#include <string>

/*  Abstract class class for managing notifications.    */
class NotifyBase
{
public:
    /* Destructor */
    virtual ~NotifyBase()
    {}

    /*  Notifies about the event 
        @param  aNotification - notification message
     */
    virtual void notify(const std::string& aNotification) = 0;

    /*  Notifies about the warning
        @param  aNotification - warning message
     */
    virtual void warning(const std::string& aNotification) = 0;

    /*  Notifies about the error
        @param  aNotification - error message
     */
    virtual void error(const std::string& aNotification) = 0;

    /*  Notifies about the debug message
        @param  aNotification - debug message
     */
    virtual void debug(const std::string& aNotification) = 0;
};

#endif /* __notify_base_h__  */
