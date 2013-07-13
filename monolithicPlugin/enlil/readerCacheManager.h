#ifndef READERCACHEMANAGER_H
#define READERCACHEMANAGER_H

#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
#include <libkern/OSAtomic.h>
#endif

#define LOCKED 1

class readerCacheManager
{
public:
    readerCacheManager();

private:

    bool TestAndSet(volatile int* lock)
    {
        int old_value = 0;
#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
        return OSAtomicCompareAndSwapInt (old_value, 1, lock);
#elif defined(_MSC_VER)
        return InterlockedCompareExchange(lock, 1, old_value);
#elif (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
        return __sync_val_compare_and_swap(lock, old_value, 1);
#else
#  error No implementation
#endif
    }


};

#endif // READERCACHEMANAGER_H
