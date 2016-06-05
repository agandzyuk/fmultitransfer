#ifndef __task_impl_h__
#define __task_impl_h__

#include "common_types.h"

class TaskAccessor
{
public:
	static Task::Impl* impl( Task* src ) 
    { 
        return src->impl_; 
    }

	static Task::Impl const* impl( Task const* src ) 
    { 
        return src->impl_; 
    }

	static void destroy( Task const* task ) 
    { 
        delete task; 
    }

	static void run( Task* task ) 
    { 
        task->run(); 
    }
};

struct Task::Impl
{
    typedef enum 
    {
        NA, 
        SCHEDULED, 
        CANCELLED, 
        EXECUTED
    } STATE;

    Impl( void ) 
        : state_(NA), 
        period_(0), 
        taskScheduleTime_(0), 
        nextExecutionTime_(0), 
        externalOwnership_(false)
    {}

    STATE state_;
    
    /*  time in milliseconds between successive task executions */
    i64 period_; 

    /*  time of the task scheduling (in milliseconds) */
    i64 taskScheduleTime_;

    /*  next execution time (in milliseconds)   */
    i64 nextExecutionTime_;

    /* Task Name    */
    std::string name_;    

    /* 'false' if the task's ownership belongs to the timer, otherwise - false */
    bool externalOwnership_;
};

#endif /* __task_impl_h__ */
