#include "task.h"
#include "task_impl.h"

Task::Task()
    : impl_(new Task::Impl() )
{}

Task::Task( const std::string& name )
    : impl_( new Task::Impl() )
{
    impl_->name_ = name;
}

Task::~Task()
{
    delete impl_;
}

std::string Task::get_name() const
{
    return impl_->name_;
}
