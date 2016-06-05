#ifndef __ref_counted_h__
#define __ref_counted_h__

#include "common_types.h"

#include <cassert>

struct RefCounted
{
public:
    RefCounted();

protected:
    virtual ~RefCounted();

public:
    /*  safe increment */
    virtual s32 add_ref() const;

    /*  safe decrement */
    virtual s32 release() const;

    /*  number of references */
    virtual s32 references() const;

protected:
    /*  atomic decrement */
    s32 decrement() const;

private:
    /*  the counter (used as atom) */
#ifdef WIN32
    volatile mutable long m_nRef;
#else
    volatile mutable s32 m_nRef;
#endif
};

/*  Smart Pointer for reference counting.   */
template<class T> class RefCountedPtr
{
private:
    /** pointee */
    T* m_pI;

public:
    typedef RefCountedPtr<T> MyT;

    T& operator*() const 
    {
        assert( nullptr != this->m_pI ); 
        return *this->m_pI; 
    }
        
    T* operator->() const 
    {
        assert(nullptr != this->m_pI); 
        return this->m_pI; 
    }

    MyT& operator=( MyT const& rv )
    {
        reset( rv.get() );
        return *this;
    }

    void reset(T* apI = nullptr, bool addRef = true )
    {
        if( nullptr != apI && addRef )
        {
            apI->add_ref();
        }

        if( nullptr != m_pI )
        {
            m_pI->release();
        }

            this->m_pI = apI;
        }

        bool operator!() const 
        {
            return nullptr == m_pI;
        }

        operator bool() const 
        {
            return nullptr != m_pI; 
        }

        /*  Sets the ownership indicator to false, then returns the stored pointer. */
        T* abandon() 
        {
            T* result = this->m_pI; 
            this->m_pI = nullptr; 
            return result;
        }

        T* get() const
        {
            return m_pI;
        }

public:
    RefCountedPtr() 
        : m_pI( nullptr )
    {}

    explicit RefCountedPtr(T* apV, bool increaseRefCounter = false) 
        : m_pI( apV )
    {
        if( nullptr != apV && increaseRefCounter )
        {
            apV->add_ref();
        }
    }

    RefCountedPtr( MyT const& aV ) 
        : m_pI( aV.m_pI ) 
    {
        if( nullptr != aV.m_pI )
        {
            aV.m_pI->add_ref();
        }
    }

    template< typename Other >
    RefCountedPtr( RefCountedPtr< Other > const& src ) 
        : m_pI( src.get() ) 
    {
        if( nullptr != this->m_pI )
        {
            this->m_pI->add_ref();
        }
    }

    ~RefCountedPtr()
    {
        if(nullptr != this->m_pI)
        {
            this->m_pI->release();
            this->m_pI = nullptr;
        }
    }
};

#endif /* __ref_counted_h__ */
