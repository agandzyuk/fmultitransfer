#ifndef __guard_h__
#define __guard_h__

template<class T>
class Guard
{
public:
    /*Constructor. Acquires the given resource.
     * @param lock Resource monitor implementation
     */
    Guard(T& lock) : lock_(lock), isActive_(true)
    { 
        lock_.lock(); 
    }

    /*
     * Destructor. Releases the given resource.
     */
    ~Guard()
    { 
        release();
    }

    /*
     * Releases the resource.
     */  
    void release()
    {
        if( isActive_ )
        {
            lock_.unlock();
            isActive_ = false;
        }
    }

protected:

    /* Copy constructor */
    Guard( const Guard& );

    /* Assignment operator */
    Guard& operator=( const Guard& );

private:
    T& lock_;
    bool isActive_;
};

/*
 * Used to temporary unlock mutex
 * Unlocker<Mutex> unlocker (lock_); ( guard constructor releases m_lock. )
 */
template<class T>
class Unlocker
{
public:
    /* Constructor. Acquires the given resource.
     * @param lock Resource monitor implementation
     */
    Unlocker(T& lock) : lock_(lock) 
    { lock_.unlock(); }

    /*
     * Destructor. Releases the given resource.
     */
    ~Unlocker()
    { 
        lock_.lock();
    }

protected:

    /* Copy constructor */
    Unlocker( const Unlocker& );

    /* Assignment operator */
    Unlocker& operator=( const Unlocker& );

private:
    T& lock_;
};

#endif /* __guard_h__ */
