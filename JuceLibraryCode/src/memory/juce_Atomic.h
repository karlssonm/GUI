/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_ATOMIC_JUCEHEADER__
#define __JUCE_ATOMIC_JUCEHEADER__


//==============================================================================
/**
    Simple class to hold a primitive value and perform atomic operations on it.

    The type used must be a 32 or 64 bit primitive, like an int, pointer, etc.
    There are methods to perform most of the basic atomic operations.
*/
template <typename Type>
class Atomic
{
public:
    /** Creates a new value, initialised to zero. */
    inline Atomic() throw()
        : value (0)
    {
    }

    /** Creates a new value, with a given initial value. */
    inline Atomic (const Type initialValue) throw()
        : value (initialValue)
    {
    }

    /** Copies another value (atomically). */
    inline Atomic (const Atomic& other) throw()
        : value (other.get())
    {
    }

    /** Destructor. */
    inline ~Atomic() throw()
    {
        // This class can only be used for types which are 32 or 64 bits in size.
        static_jassert (sizeof (Type) == 4 || sizeof (Type) == 8);
    }

    /** Atomically reads and returns the current value. */
    Type get() const throw();

    /** Copies another value onto this one (atomically). */
    inline Atomic& operator= (const Atomic& other) throw()          { exchange (other.get()); return *this; }

    /** Copies another value onto this one (atomically). */
    inline Atomic& operator= (const Type newValue) throw()          { exchange (newValue); return *this; }

    /** Atomically sets the current value. */
    void set (Type newValue) throw()                                { exchange (newValue); }

    /** Atomically sets the current value, returning the value that was replaced. */
    Type exchange (Type value) throw();

    /** Atomically adds a number to this value, returning the new value. */
    Type operator+= (Type amountToAdd) throw();

    /** Atomically subtracts a number from this value, returning the new value. */
    Type operator-= (Type amountToSubtract) throw();

    /** Atomically increments this value, returning the new value. */
    Type operator++() throw();

    /** Atomically decrements this value, returning the new value. */
    Type operator--() throw();

    /** Atomically compares this value with a target value, and if it is equal, sets
        this to be equal to a new value.

        This operation is the atomic equivalent of doing this:
        @code
        bool compareAndSetBool (Type newValue, Type valueToCompare)
        {
            if (get() == valueToCompare)
            {
                set (newValue);
                return true;
            }

            return false;
        }
        @endcode

        @returns true if the comparison was true and the value was replaced; false if
                 the comparison failed and the value was left unchanged.
        @see compareAndSetValue
    */
    bool compareAndSetBool (Type newValue, Type valueToCompare) throw();

    /** Atomically compares this value with a target value, and if it is equal, sets
        this to be equal to a new value.

        This operation is the atomic equivalent of doing this:
        @code
        Type compareAndSetValue (Type newValue, Type valueToCompare)
        {
            Type oldValue = get();
            if (oldValue == valueToCompare)
                set (newValue);

            return oldValue;
        }
        @endcode

        @returns the old value before it was changed.
        @see compareAndSetBool
    */
    Type compareAndSetValue (Type newValue, Type valueToCompare) throw();

    /** Implements a memory read/write barrier. */
    static void memoryBarrier() throw();

    //==============================================================================
    JUCE_ALIGN(8)

    /** The raw value that this class operates on.
        This is exposed publically in case you need to manipulate it directly
        for performance reasons.
    */
    volatile Type value;

private:
    static inline Type castFrom32Bit (int32 value) throw()    { return *(Type*) &value; }
    static inline Type castFrom64Bit (int64 value) throw()    { return *(Type*) &value; }
    static inline int32 castTo32Bit (Type value) throw()      { return *(int32*) &value; }
    static inline int64 castTo64Bit (Type value) throw()      { return *(int64*) &value; }

    Type operator++ (int); // better to just use pre-increment with atomics..
    Type operator-- (int);
};


//==============================================================================
/*
    The following code is in the header so that the atomics can be inlined where possible...
*/
#if (JUCE_IOS && (__IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_3_2 || ! defined (__IPHONE_3_2))) \
      || (JUCE_MAC && (JUCE_PPC || __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 2)))
  #define JUCE_ATOMICS_MAC 1        // Older OSX builds using gcc4.1 or earlier

  #if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
    #define JUCE_MAC_ATOMICS_VOLATILE
  #else
    #define JUCE_MAC_ATOMICS_VOLATILE volatile
  #endif

  #if JUCE_PPC || JUCE_IOS
    // None of these atomics are available for PPC or for iPhoneOS 3.1 or earlier!!
    template <typename Type> static Type OSAtomicAdd64Barrier (Type b, JUCE_MAC_ATOMICS_VOLATILE Type* a) throw()   { jassertfalse; return *a += b; }
    template <typename Type> static Type OSAtomicIncrement64Barrier (JUCE_MAC_ATOMICS_VOLATILE Type* a) throw()     { jassertfalse; return ++*a; }
    template <typename Type> static Type OSAtomicDecrement64Barrier (JUCE_MAC_ATOMICS_VOLATILE Type* a) throw()     { jassertfalse; return --*a; }
    template <typename Type> static bool OSAtomicCompareAndSwap64Barrier (Type old, Type newValue, JUCE_MAC_ATOMICS_VOLATILE Type* value) throw()
        { jassertfalse; if (old == *value) { *value = newValue; return true; } return false; }
    #define JUCE_64BIT_ATOMICS_UNAVAILABLE 1
  #endif

//==============================================================================
#elif JUCE_GCC
  #define JUCE_ATOMICS_GCC 1        // GCC with intrinsics

  #if JUCE_IOS
    #define JUCE_64BIT_ATOMICS_UNAVAILABLE 1  // (on the iphone, the 64-bit ops will compile but not link)
  #endif

//==============================================================================
#else
  #define JUCE_ATOMICS_WINDOWS 1    // Windows with intrinsics

  #if JUCE_USE_INTRINSICS || JUCE_64BIT
    #pragma intrinsic (_InterlockedExchange, _InterlockedIncrement, _InterlockedDecrement, _InterlockedCompareExchange, \
                       _InterlockedCompareExchange64, _InterlockedExchangeAdd, _ReadWriteBarrier)
    #define juce_InterlockedExchange(a, b)              _InterlockedExchange(a, b)
    #define juce_InterlockedIncrement(a)                _InterlockedIncrement(a)
    #define juce_InterlockedDecrement(a)                _InterlockedDecrement(a)
    #define juce_InterlockedExchangeAdd(a, b)           _InterlockedExchangeAdd(a, b)
    #define juce_InterlockedCompareExchange(a, b, c)    _InterlockedCompareExchange(a, b, c)
    #define juce_InterlockedCompareExchange64(a, b, c)  _InterlockedCompareExchange64(a, b, c)
    #define juce_MemoryBarrier _ReadWriteBarrier
  #else
    // (these are defined in juce_win32_Threads.cpp)
    long juce_InterlockedExchange (volatile long* a, long b) throw();
    long juce_InterlockedIncrement (volatile long* a) throw();
    long juce_InterlockedDecrement (volatile long* a) throw();
    long juce_InterlockedExchangeAdd (volatile long* a, long b) throw();
    long juce_InterlockedCompareExchange (volatile long* a, long b, long c) throw();
    __int64 juce_InterlockedCompareExchange64 (volatile __int64* a, __int64 b, __int64 c) throw();
    inline void juce_MemoryBarrier() throw()   { long x = 0; juce_InterlockedIncrement (&x); }
  #endif

  #if JUCE_64BIT
    #pragma intrinsic (_InterlockedExchangeAdd64, _InterlockedExchange64, _InterlockedIncrement64, _InterlockedDecrement64)
    #define juce_InterlockedExchangeAdd64(a, b)     _InterlockedExchangeAdd64(a, b)
    #define juce_InterlockedExchange64(a, b)        _InterlockedExchange64(a, b)
    #define juce_InterlockedIncrement64(a)          _InterlockedIncrement64(a)
    #define juce_InterlockedDecrement64(a)          _InterlockedDecrement64(a)
  #else
    // None of these atomics are available in a 32-bit Windows build!!
    template <typename Type> static Type juce_InterlockedExchangeAdd64 (volatile Type* a, Type b) throw()   { jassertfalse; Type old = *a; *a += b; return old; }
    template <typename Type> static Type juce_InterlockedExchange64 (volatile Type* a, Type b) throw()      { jassertfalse; Type old = *a; *a = b; return old; }
    template <typename Type> static Type juce_InterlockedIncrement64 (volatile Type* a) throw()             { jassertfalse; return ++*a; }
    template <typename Type> static Type juce_InterlockedDecrement64 (volatile Type* a) throw()             { jassertfalse; return --*a; }
    #define JUCE_64BIT_ATOMICS_UNAVAILABLE 1
  #endif
#endif

#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4311)  // (truncation warning)
#endif

//==============================================================================
template <typename Type>
inline Type Atomic<Type>::get() const throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) OSAtomicAdd32Barrier ((int32_t) 0, (JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value))
                              : castFrom64Bit ((int64) OSAtomicAdd64Barrier ((int64_t) 0, (JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value));
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) juce_InterlockedExchangeAdd ((volatile long*) &value, (long) 0))
                              : castFrom64Bit ((int64) juce_InterlockedExchangeAdd64 ((volatile __int64*) &value, (__int64) 0));
  #elif JUCE_ATOMICS_GCC
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) __sync_add_and_fetch ((volatile int32*) &value, 0))
                              : castFrom64Bit ((int64) __sync_add_and_fetch ((volatile int64*) &value, 0));
  #endif
}

template <typename Type>
inline Type Atomic<Type>::exchange (const Type newValue) throw()
{
  #if JUCE_ATOMICS_MAC || JUCE_ATOMICS_GCC
    Type currentVal = value;
    while (! compareAndSetBool (newValue, currentVal)) { currentVal = value; }
    return currentVal;
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) juce_InterlockedExchange ((volatile long*) &value, (long) castTo32Bit (newValue)))
                              : castFrom64Bit ((int64) juce_InterlockedExchange64 ((volatile __int64*) &value, (__int64) castTo64Bit (newValue)));
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator+= (const Type amountToAdd) throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? (Type) OSAtomicAdd32Barrier ((int32_t) castTo32Bit (amountToAdd), (JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value)
                              : (Type) OSAtomicAdd64Barrier ((int64_t) amountToAdd, (JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) (juce_InterlockedExchangeAdd ((volatile long*) &value, (long) amountToAdd) + (long) amountToAdd)
                              : (Type) (juce_InterlockedExchangeAdd64 ((volatile __int64*) &value, (__int64) amountToAdd) + (__int64) amountToAdd);
  #elif JUCE_ATOMICS_GCC
    return (Type) __sync_add_and_fetch (&value, amountToAdd);
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator-= (const Type amountToSubtract) throw()
{
    return operator+= (juce_negate (amountToSubtract));
}

template <typename Type>
inline Type Atomic<Type>::operator++() throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? (Type) OSAtomicIncrement32Barrier ((JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value)
                              : (Type) OSAtomicIncrement64Barrier ((JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) juce_InterlockedIncrement ((volatile long*) &value)
                              : (Type) juce_InterlockedIncrement64 ((volatile __int64*) &value);
  #elif JUCE_ATOMICS_GCC
    return (Type) __sync_add_and_fetch (&value, 1);
  #endif
}

template <typename Type>
inline Type Atomic<Type>::operator--() throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? (Type) OSAtomicDecrement32Barrier ((JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value)
                              : (Type) OSAtomicDecrement64Barrier ((JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? (Type) juce_InterlockedDecrement ((volatile long*) &value)
                              : (Type) juce_InterlockedDecrement64 ((volatile __int64*) &value);
  #elif JUCE_ATOMICS_GCC
    return (Type) __sync_add_and_fetch (&value, -1);
  #endif
}

template <typename Type>
inline bool Atomic<Type>::compareAndSetBool (const Type newValue, const Type valueToCompare) throw()
{
  #if JUCE_ATOMICS_MAC
    return sizeof (Type) == 4 ? OSAtomicCompareAndSwap32Barrier ((int32_t) castTo32Bit (valueToCompare), (int32_t) castTo32Bit (newValue), (JUCE_MAC_ATOMICS_VOLATILE int32_t*) &value)
                              : OSAtomicCompareAndSwap64Barrier ((int64_t) castTo64Bit (valueToCompare), (int64_t) castTo64Bit (newValue), (JUCE_MAC_ATOMICS_VOLATILE int64_t*) &value);
  #elif JUCE_ATOMICS_WINDOWS
    return compareAndSetValue (newValue, valueToCompare) == valueToCompare;
  #elif JUCE_ATOMICS_GCC
    return sizeof (Type) == 4 ? __sync_bool_compare_and_swap ((volatile int32*) &value, castTo32Bit (valueToCompare), castTo32Bit (newValue))
                              : __sync_bool_compare_and_swap ((volatile int64*) &value, castTo64Bit (valueToCompare), castTo64Bit (newValue));
  #endif
}

template <typename Type>
inline Type Atomic<Type>::compareAndSetValue (const Type newValue, const Type valueToCompare) throw()
{
  #if JUCE_ATOMICS_MAC
    for (;;) // Annoying workaround for OSX only having a bool CAS operation..
    {
        if (compareAndSetBool (newValue, valueToCompare))
            return valueToCompare;

        const Type result = value;
        if (result != valueToCompare)
            return result;
    }

  #elif JUCE_ATOMICS_WINDOWS
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) juce_InterlockedCompareExchange ((volatile long*) &value, (long) castTo32Bit (newValue), (long) castTo32Bit (valueToCompare)))
                              : castFrom64Bit ((int64) juce_InterlockedCompareExchange64 ((volatile __int64*) &value, (__int64) castTo64Bit (newValue), (__int64) castTo64Bit (valueToCompare)));
  #elif JUCE_ATOMICS_GCC
    return sizeof (Type) == 4 ? castFrom32Bit ((int32) __sync_val_compare_and_swap ((volatile int32*) &value, castTo32Bit (valueToCompare), castTo32Bit (newValue)))
                              : castFrom64Bit ((int64) __sync_val_compare_and_swap ((volatile int64*) &value, castTo64Bit (valueToCompare), castTo64Bit (newValue)));
  #endif
}

template <typename Type>
inline void Atomic<Type>::memoryBarrier() throw()
{
  #if JUCE_ATOMICS_MAC
    OSMemoryBarrier();
  #elif JUCE_ATOMICS_GCC
    __sync_synchronize();
  #elif JUCE_ATOMICS_WINDOWS
    juce_MemoryBarrier();
  #endif
}

#if JUCE_MSVC
  #pragma warning (pop)
#endif

#endif   // __JUCE_ATOMIC_JUCEHEADER__
