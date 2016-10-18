#ifdef __linux__
    #include <pthread.h>
#endif
#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "../include/mutex"

using namespace std;

//------------------------------------------------------------------------------
THRD_CALL std::mutex_base::mutex_base(bool bRecursive)
{
    /**
     * Constructs the mutex. The mutex is in unlocked state after the call.
     * Throw std::system_error if the construction is unsuccessful.
     */
#if   defined(__linux__)
    int                 ErrorCode;
    pthread_mutexattr_t attr;
    int                 type = bRecursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK;  // Note : The falg can set to PTHREAD_MUTEX_NORMAL for fast mutex.

    ErrorCode = pthread_mutexattr_init(&attr);
    if( ErrorCode ) throw system_error(ErrorCode, "Mutex attribute initialize failed");

    try
    {
        ErrorCode = pthread_mutexattr_settype(&attr, type);
        if( ErrorCode ) throw system_error(ErrorCode, "Mutex attribute set value failed");
        ErrorCode = pthread_mutex_init(&obj, &attr);
        if( ErrorCode ) throw system_error(ErrorCode, "Mutex initialize failed");
    }
    catch(system_error &E)
    {
        pthread_mutexattr_destroy(&attr);  // Ignore error code
        throw;
    }

    pthread_mutexattr_destroy(&attr);  // Ignore error code
#elif defined(_WIN32)
    InitializeCriticalSection(&obj);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
THRD_CALL std::mutex_base::~mutex_base()
{
#if   defined(__linux__)
    pthread_mutex_destroy(&obj);  // Ignore error code
#elif defined(_WIN32)
    DeleteCriticalSection(&obj);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
void THRD_CALL std::mutex_base::lock()
{
    /**
     * Throws std::system_error when errors occur, including errors from the underlying operating system that would prevent lock from meeting its specifications.
     * The mutex is not locked in the case of any exception being thrown.
     */
#if   defined(__linux__)
    int ErrorCode = pthread_mutex_lock(&obj);
    if( ErrorCode ) throw system_error(ErrorCode, "Mutex lock error");
#elif defined(_WIN32)
    EnterCriticalSection(&obj);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
bool THRD_CALL std::mutex_base::try_lock()
{
    /**
     * @return : true if the lock was acquired successfully, otherwise false.
     */
#if   defined(__linux__)
    return !pthread_mutex_trylock(&obj);
#elif defined(_WIN32)
    return !!TryEnterCriticalSection(&obj);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
void THRD_CALL std::mutex_base::unlock()
{
#if   defined(__linux__)
    pthread_mutex_unlock(&obj);  // Ignore error code
#elif defined(_WIN32)
    LeaveCriticalSection(&obj);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
