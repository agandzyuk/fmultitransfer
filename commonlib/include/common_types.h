#ifndef __common_types_h__
#define __common_types_h__

#ifdef WIN32
#pragma warning(disable:4214) /* disable depricated warnings */
#pragma warning(disable:4482) /* disable warnings about nonstandard enumerations using */
#pragma warning(disable:4996) /* disable 'This function or variable may be unsafe.' */
#endif

/****************************************************/
/* Constants used in ALL modules                    */
/****************************************************/
#define s8              char
#define u8              unsigned char
#define s16             short
#define u16             unsigned short
#define s32             int
#define u32             unsigned int
#define i32             int
#define ui32            unsigned int
#define i64             long long
#define u64             unsigned long long

/****************************************************/
/*  nullptr and other                               */
/****************************************************/
class NullPtr
{
public: 
    template<typename T>
    operator T*() const 
    { 
        return 0; 
    }
};

template<typename T>
inline bool operator==( T* obj, NullPtr const& )
{
    return 0 == obj;
}

template< typename T >
inline bool operator==( NullPtr const&, T* obj )
{
    return 0 == obj;
}

template< typename T >
inline bool operator!=( T* obj, NullPtr const& )
{
    return 0 != obj;
}

template< typename T >
inline bool operator!=( NullPtr const&, T* obj )
{
    return 0 != obj;
}


#if defined(__linux) || (defined(_MSC_VER) && _MSC_VER < 1600 && !defined(_MANAGED))
/*
#ifdef _DEBUG
    static NullPtr nullptr;
#else
    static int const nullptr = 0;
#endif
*/
    static int const nullptr = 0;
#endif 


template<typename INT>
struct IntTraints;

template<>
struct IntTraints<i32>
{
    typedef i32 Signed;
    typedef u32 Unsigned;
};

template<>
struct IntTraints<u32>
{
    typedef i32 Signed;
    typedef u32 Unsigned;
};

template<>
struct IntTraints<i64>
{
    typedef i64 Signed;
    typedef u64 Unsigned;
};

template<>
struct IntTraints <u64>
{
    typedef i64 Signed;
    typedef u64 Unsigned;
};

struct mem_block
{
    s8* ptr_;
    s8* end_;

    s8* end() 
    { 
        return end_; 
    }
    s8* last() 
    { 
        return end_ - 1; 
    }
};

#endif /* __common_types_h__ */
