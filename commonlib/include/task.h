#ifndef __task_h__
#define __task_h__

#include <string>
#include "system_defines.h"

class TaskAccessor;

/*  Timer's task 
    It can be scheduled for one-time or repeated execution by a Timer
    @see Timer
*/
class Task
{
public:  
	struct Impl;

	/*  Returns task's name */
	virtual std::string get_name() const;

protected:
	Task();
	Task(const std::string& name);
	virtual ~Task();

	/*  The action to be performed by this timer task   */
	virtual void run() = 0;

private:  
	/*  Implementation detail   */
	Impl* impl_;
	friend class TaskAccessor;
};

#endif /* __task_h__ */
