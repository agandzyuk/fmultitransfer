#ifndef __timer_h__
#define __timer_h__

#include "thread.h"

class Task;   

class Timer : public Thread
{
public:
    Timer( void );

   /*   Destructor. 
        @note calls cancel() if it has not been called
        @see cancel()
    */
    virtual ~Timer( void );

   /*   Schedules the specified task for repeated fixed-delay execution,
        beginning after the specified delay.
        @param task The task to be scheduled.
        @param delay Delay in milliseconds before task is to be executed, can be negative.
        @param period Time in milliseconds between successive task executions, 0 implies
        a non-repeating task.
        @note This call transfers the ownership of the task to the timer
        (in other words, the task will be deleted by the timer).
        @warning TimerTask must not be a stack object.       
        @throw Exception if a task with the same non-empty name has been already scheduled.
        @throw Exception if the task has been scheduled already.
    */
    void schedule( Task* task, i64 delay, i64 period );

   /*   Schedules the specified task for execution at the specified time
        @param task The task to be scheduled.
        @param time The current local time in milliseconds when task should be executed 
        @note Supposed that period of task execution is 24 hours
        @param period Time in milliseconds between successive task executions, 0 implies
        a non-repeating task.
        @note This call transfers the ownership of the task to the timer
        (in other words, the task will be deleted by the timer).
        @warning Task must not be a stack object.       
        @throw Exception if a task with the same non-empty name has been already scheduled.
        @throw Exception if the task has been scheduled already.
    */
    void scheduleAtTime( Task* task, i64 time, i64 period );

   /*   Reshedules the specified task for repeated fixed-delay execution,
        beginning after the specified delay.
        @param task The task to be scheduled.
        @param delay Delay in milliseconds before task is to be executed, can be negative.
        @param period Time in milliseconds between successive task executions, 0 implies
        a non-repeating task.
        @throw Exception if the task was not found.
    */
    void reschedule( Task* task, i64 delay, i64 period );

   /*   Terminates this timer, discarding and deleting any currently scheduled tasks   */
    virtual void cancel( void );

   /*   Cancels the execution of the given task. 
        @param takeOwnership If 'true' then the timer will not be in charge of the task anymore
        (in other words, the task will not be deleted by the timer).
        @return 'true' if the task was successfully cancelled, otherwise - 'false'.
        (e.g the task was not found or had been executed and deleted). 
        @note Method does not remove task from queue. Instead it sets its
        state to the CANCELLED, which is checked by Timer::run.
        @note Even if takeOwnership set to true this does not mean that
        Task if fully operable. For example if you delete this task 
        right after cancel call you may get GPF when Timer::run try to
        check the 'task->impl_->state_'.
    */
    bool cancel( Task* task, bool takeOwnership = false );

   /*   Cancels the execution of the given task. 
        @param taskName The name that is returned by Task::getName().
        @param takeOwnership If 'true' then the timer will not be in charge of the task anymore        
        (in other words, the task will not be deleted by the timer).
        @note taskName cannot be empty.
        @return 'true' if the task was successfully cancelled, otherwise - 'false'.
        (e.g the task was not found or had been executed and deleted). 
        @note Method does not remove task from queue. Instead it sets its
        state to the CANCELLED, which is checked by Timer::run.
        @note Even if takeOwnership set to true this does not mean that
        Task if fully operable. For example if you delete this task 
        right after cancel call you may get GPF when Timer::run try to
        check the 'task->impl_->state_'.
    */ 
    bool cancel( const std::string& taskName, bool takeOwnership = false );       

    struct TimerImpl;

protected:
   /* Reimplemented from Thread */
   virtual void run( void );

private:     
    /* Implementation details */
    TimerImpl* impl_;
};

#endif /* __timer_h__ */
