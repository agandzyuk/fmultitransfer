
#include "refcounted.h"
#include "system_defines.h"


#ifdef WIN32

static inline long increment( long volatile* value )
{
    return InterlockedIncrement( value );
}
static inline long decrement( long volatile* value )
{
    return InterlockedDecrement( value );
}

#else

static inline s32 decrement( s32 volatile* value )
{
    return --(*value);
}
static inline s32 increment( s32 volatile* value )
{
    return ++(*value);
}

#endif


RefCounted::RefCounted()
    : m_nRef(1)
{}

RefCounted::~RefCounted()
{
    assert(0 == m_nRef || 1 == m_nRef);
}

s32 RefCounted::add_ref() const
{
    return (s32)::increment( &m_nRef );
}

s32 RefCounted::references() const
{
    return m_nRef;
}

s32 RefCounted::release() const 
{
    s32 ref_cnt = (s32)::decrement( &m_nRef );

    if( 0 == ref_cnt )
    {
        delete this;
        return 0;
    }
    return ref_cnt;
}

s32 RefCounted::decrement() const
{
    return (s32)::decrement( &m_nRef );
}
