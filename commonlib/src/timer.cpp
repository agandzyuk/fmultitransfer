#include "timer.h"

#include "thread.h"
#include "condition.h"
#include "semaphorp.h"
#include "task.h"
#include "task_impl.h"
#include "useful.h"

#include <time.h>
#include <list>
#include <algorithm>

using namespace std;

const int DL = -1;

/*  Functional object. Compares two Task's pointer.
    The task with smaller nextExecutionTime_ has higher priority.
  
    It is used in TaskQueue:
    After each insertion or removal of the top element (at position zero), 
    for the iterators P0 and Pi designating elements at positions 0 and i, 
    CompareTaskPtr(*P0, *Pi) is false. 
*/
class CompareTaskPtr
{
public:
    bool operator()( const Task* left, const Task* right ) const
    {
		return (TaskAccessor::impl( left )->nextExecutionTime_ < 
                TaskAccessor::impl( right )->nextExecutionTime_);
    }
};

class IsEqualTaskName
{
public:
    IsEqualTaskName( const std::string& name ) 
        : name_(name)
    {}

    bool operator()( const Task* task ) const
    {	
        return (task->get_name() == name_);
    }

private:
    std::string name_;
};

struct Timer::TimerImpl
{
    TimerImpl( void ) 
        : isCancelled_(false)
    {}	 

    /*  Lock */
    Mutex lock_;

    /*  Condition */
    Condition cond_;

    typedef std::list<Task*> TasksQueue;

    /*  Queue */
    TasksQueue queue_;

    /*  True if the timer is cancelled, otherwise false */
    bool isCancelled_;

    /*  Semaphore that is used by cancel() to wait until the timer is terminated */
    Semaphore sem_;

    void run( void );

    void schedule( Task* task, i64 delay, i64 period );

    void scheduleAtTime( Task* task, i64 time, i64 period );

    void reschedule( Task* task, i64 delay, i64 period );

    void clean( void );

    /* @note Non-synchronized */
    void push( Task* task );

    bool cancel( void );

    bool cancel( Task *task, bool takeOwnership );

    bool cancel( const string& taskName, bool takeOwnership );
};

void Timer::TimerImpl::push( Task* task )
{
    TasksQueue::iterator it = lower_bound( queue_.begin(), queue_.end(), task, CompareTaskPtr() );
    queue_.insert( it, task );
}

Timer::Timer() 
    : Thread("Timer"), 
    impl_( new TimerImpl() )
{
    Thread::start();
}

void Timer::TimerImpl::run()
{
    Task* executedTask = NULL;
    for(;;)
    {
        if( NULL != executedTask )
        {
            TaskAccessor::destroy( executedTask );
            executedTask = NULL;
        }

        bool fireTask = false;
        Task *task = NULL;
        {
            MGuard g(lock_);
            while (queue_.empty() && (! isCancelled_))
            {
                cond_.wait(&lock_);
            }
            if( isCancelled_ )
            {
                sem_.post();
                return;
            }
            else if( queue_.empty() )
            {
                continue;
            }

            task = queue_.front();	 
            u64 executionTime, currentTime = 0;
            { /* block where the task will be locked */
                if( Task::Impl::CANCELLED == TaskAccessor::impl( task )->state_ ) 
                {                    
                    queue_.pop_front();
                    if( !TaskAccessor::impl( task )->externalOwnership_ )
                    {
			            TaskAccessor::destroy( task );
                    }                    
                    continue;  /* No action required */
                }

                executionTime = TaskAccessor::impl( task )->nextExecutionTime_;
                currentTime   = current_time();
                fireTask = executionTime <= currentTime;

                if( fireTask )
                {
                    queue_.pop_front();
		            if( TaskAccessor::impl( task )->period_ == 0 ) 
                    { /* Non-repeating   */
                        TaskAccessor::impl( task )->state_ = Task::Impl::EXECUTED;
                        executedTask = task;
                    } 
                    else 
                    { /* Repeating task, reschedule */
                        TaskAccessor::impl( task )->nextExecutionTime_ += TaskAccessor::impl( task )->period_;                      
                        push(task);
                    }
                }
            }          
            if( !fireTask )
            { /* Task hasn't yet fired; wait */
                cond_.timed_wait( &lock_, executionTime - currentTime );
            }			
        }
        /* all locks are released */
        if( fireTask )
        {
            TaskAccessor::run( task );
        }       
    }
}

void Timer::schedule( Task* task, i64 delay, i64 period )
{
    impl_->schedule( task, delay, period );
}

void Timer::scheduleAtTime( Task* task, i64 time, i64 period )
{
    impl_->scheduleAtTime( task, time, period );
}

void Timer::reschedule( Task* task, i64 delay, i64 period )
{
    impl_->reschedule( task, delay, period );
}

bool Timer::TimerImpl::cancel( Task *task, bool takeOwnership )
{
    MGuard g(lock_);

    typedef TasksQueue::iterator It;
    It it = find( queue_.begin(), queue_.end(), task );
    if( it != queue_.end() )
    {
        TaskAccessor::impl( task )->state_ = Task::Impl::CANCELLED;
        TaskAccessor::impl( task )->externalOwnership_ = takeOwnership;
        return true;
    }    
    return false;
}

bool Timer::TimerImpl::cancel( const string& taskName, bool takeOwnership )
{
    MGuard g(lock_);

    typedef TasksQueue::iterator It;
    IsEqualTaskName fo(taskName);
    It it = find_if(queue_.begin(), queue_.end(), fo);

    if( it != queue_.end() )
    {
        Task* task = *it;
        TaskAccessor::impl( task )->state_ = Task::Impl::CANCELLED;
        TaskAccessor::impl( task )->externalOwnership_ = takeOwnership;
        return true;
    }    
    return false;
}

void Timer::TimerImpl::schedule( Task* task, i64 delay, i64 period )
{
    MGuard g(lock_);

    if( isCancelled_ )
    {
	    throw Exception("Timer::schedule() unable to schedule task - timer was stopped!");
    }

    typedef TasksQueue::iterator It;
    if( !task->get_name().empty() )
    {
        IsEqualTaskName fo(task->get_name());
        It it = find_if(queue_.begin(), queue_.end(), fo);
        if(it != queue_.end() && (Task::Impl::CANCELLED != TaskAccessor::impl( (*it) )->state_))
        {
            throw Exception("Timer::Impl::schedule(task (name="+ task->get_name() + "), "
                            + tostring((u32)delay) + ", " + tostring((u32)period) 
                            + ") failed: a task with this name has been already scheduled.");
        }
    }

    It it = find(queue_.begin(), queue_.end(), task);
    if(it != queue_.end())
    {
        throw Exception("Timer::schedule(task (taskName=" + task->get_name() + ") , " + 
                        tostring((u32)delay) + ", " + tostring((u32)period) + 
                        ") failed: the task was already sheduled.");
    }

    assert( period >= 0 );

    TaskAccessor::impl( task )->nextExecutionTime_ = current_time() + delay; 
    TaskAccessor::impl( task )->period_ = period;
    TaskAccessor::impl( task )->state_ = Task::Impl::SCHEDULED;         

    push(task);
    if(queue_.front() == task)
    {
        cond_.signal();
    }
}

void Timer::TimerImpl::scheduleAtTime( Task* task, i64 time, i64 period )
{
    MGuard g(lock_);

    if( isCancelled_ )
	    throw Exception("Timer::scheduleAtTime() unable to schedule task - timer was stopped!");            

    typedef TasksQueue::iterator It;

    if( !task->get_name().empty() )
    {
        IsEqualTaskName fo( task->get_name() );
        It it = find_if( queue_.begin(), queue_.end(), fo );
        if( it != queue_.end() && (Task::Impl::CANCELLED != TaskAccessor::impl( (*it) )->state_) )
        {            
            throw Exception("Timer::Impl::schedule(task (name="+ task->get_name() + ") failed:"
                            " a task with this name has been already scheduled.");
        }
    }

    It it = find(queue_.begin(), queue_.end(), task);
    if(it != queue_.end())
    {
        throw Exception("Timer::schedule(task (taskName=" + task->get_name() + ") failed:"
                        " the task was already sheduled.");            
    }

    TaskAccessor::impl( task )->nextExecutionTime_ = time; 

    assert( period >= 0 );
    TaskAccessor::impl( task )->period_ = period;
    TaskAccessor::impl( task )->state_  = Task::Impl::SCHEDULED;         

    push(task);

    if(queue_.front() == task)
    {
        cond_.signal();
    }
}

void Timer::TimerImpl::reschedule( Task* task, i64 delay, i64 period )
{
    MGuard g(lock_);

    if( isCancelled_ )
	    throw Exception("Timer::reschedule() unable to schedule task - timer was stopped!");            

    TasksQueue::iterator it = find(queue_.begin(), queue_.end(), task);
    if( it == queue_.end() )
    {
        throw Exception("Timer::reschedule(task, " + tostring((u32)delay) + ", " + tostring((u32)period)
        + ") failed: the task cannot be found");
    }

    TaskAccessor::impl( task )->nextExecutionTime_ = current_time() + delay; 
    TaskAccessor::impl( task )->period_ = period;
    TaskAccessor::impl( task )->state_  = Task::Impl::SCHEDULED; 

    queue_.erase(it);
    push(task);

    if( queue_.front() == task )
    {
        cond_.signal();
    }
}

Timer::~Timer()
{
    cancel();	
    delete impl_;
}

void Timer::TimerImpl::clean()
{
    MGuard g(lock_);

    for(TasksQueue::iterator it = queue_.begin(); it != queue_.end(); ++ it)
    {  
        Task* task = *it;
        if(! TaskAccessor::impl( task )->externalOwnership_)
        {
            TaskAccessor::destroy( task );
        }        
    }
    queue_.clear();
}

bool Timer::TimerImpl::cancel()
{
    {
        MGuard g(lock_);
        if( isCancelled_ )
        {
            return false;
        }

        isCancelled_ = true;  
        cond_.signal();
    }
    sem_.wait(); 
    return true;
}

void Timer::cancel()
{
    if( impl_->cancel() )
    {
        this->join();
        impl_->clean();
    }
}

void Timer::run()
{
    impl_->run();
}

bool Timer::cancel( Task *task, bool takeOwnership )
{
    return impl_->cancel(task, takeOwnership);
}

bool Timer::cancel(const std::string& taskName, bool takeOwnership)
{
    if( taskName.empty() )
    {
        throw Exception("Timer::cancel("+taskName +") failed: taskName cannot be empty");
    }
    return impl_->cancel(taskName, takeOwnership);
}
