#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __linux__
    #include <errno.h>
#endif
#ifdef _WIN32
    #include <windows.h>
#if !defined(WINCE)
    #include <process.h>
#endif
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "static_assert.h"
#include "../include/threads.h"

#if   defined(__linux__)
    STATIC_ASSERT( sizeof(mtx_t) >= sizeof(pthread_mutex_t) );
#elif defined(_WIN32)
    STATIC_ASSERT( sizeof(mtx_t) >= sizeof(CRITICAL_SECTION) );
#else
    #error No implementation on this platform!
#endif

//------------------------------------------------------------------------------
#ifdef _WIN32
#if defined(_MSC_VER) && ( 1500 <= _MSC_VER )

    // Function "GetThreadId" already declared in "winbase.h"
    // when use Microsoft Visual C++ version 2008 or higher.

#elif WINVER >= 0x0600

    WINBASEAPI DWORD WINAPI GetThreadId( IN HANDLE Thread );

#else

    typedef LONG NTSTATUS;
    typedef INT  THREADINFOCLASS;

    typedef
    NTSTATUS (WINAPI *func_NtQueryInformationThread)(
        IN           HANDLE          ThreadHandle,
        IN           THREADINFOCLASS ThreadInformationClass,
        IN OUT       PVOID           ThreadInformation,
        IN           ULONG           ThreadInformationLength,
        OUT OPTIONAL PULONG          ReturnLength
    );

    #pragma pack(push,8)
    typedef struct _THREAD_BASIC_INFORMATION
    {
        ULONG ExitStatus;
        PVOID TebBaseAddress;
        ULONG UniqueProcessId;
        ULONG UniqueThreadId;
        ULONG AffinityMask;
        ULONG BasePriority;
        ULONG DiffProcessPriority;
    } THREAD_BASIC_INFORMATION;
    #pragma pack(pop)

    static DWORD WINAPI GetThreadId( IN HANDLE Thread )
    {
        DWORD   ThreadID = 0;
        HMODULE hDll;

        hDll = LoadLibrary(_T("ntdll.dll"));
        if( hDll )
        {
            func_NtQueryInformationThread NtQueryInformationThread = (func_NtQueryInformationThread) GetProcAddress(hDll, _T("NtQueryInformationThread"));
            if( NtQueryInformationThread )
            {
                THREAD_BASIC_INFORMATION ThreadInfo;
                ULONG                    InfoSize;
                NTSTATUS                 Result;

                Result = NtQueryInformationThread(Thread, 0, &ThreadInfo, sizeof(ThreadInfo), &InfoSize);
                if( SUCCEEDED(Result) )
                {
                    assert( InfoSize == sizeof(THREAD_BASIC_INFORMATION) );
                    ThreadID = ThreadInfo.UniqueThreadId;
                }
            }

            FreeLibrary(hDll);
        }

        return ThreadID;
    }

#endif  // Compiler and OS version
#endif  // _WIN32
//------------------------------------------------------------------------------
//---- Threads -----------------------------------------------------------------
//------------------------------------------------------------------------------
int THRDS_CALL thrd_create( thrd_t *thr, thrd_start_t func, void *arg )
{
    /**
     * 建立一個執行緒
     * @thr    : 新執行緒的識別
     * @func   : 執行緒函式
     * @arg    : 傳遞給執行緒函式的參數
     * @return : thrd_success if successful,
     *           thrd_nomem if memory allocation failed,
     *           thrd_error otherwise.
     * note : thrd_create 函式結束時，func 函式開始運作。
     * note : 執行緒識別在執行緒 finished、joined、或 detached 後仍能被其他執行緒所使用。
     */
#if   defined(__linux__)
    int error_code = pthread_create(thr, NULL, (void*(*)(void*))func, arg);
    return error_code ? thrd_error : thrd_success;
#elif defined(_WIN32)
    #ifdef WINCE
    HANDLE thread = CreateThread(NULL, 0, (DWORD(WINAPI*)(LPVOID))func, arg, 0, NULL);
    #else
    HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, (unsigned(__stdcall*)(void*))func, arg, 0, NULL);
    #endif
    if( !thread ) return thrd_error;
    if( thr ) *thr = thread;
    return thrd_success;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL thrd_equal( thrd_t lhs, thrd_t rhs )
{
    /**
     * 判斷兩執行緒識別是否指向同一執行緒
     * @lhs, rhs : 兩個執行緒識別
     * @return   : 若兩執行緒相同則傳回非零
     */
#if   defined(__linux__)
    return pthread_equal(lhs, rhs);
#elif defined(_WIN32)
    return ( GetThreadId(lhs) == GetThreadId(rhs) );
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
thrd_t THRDS_CALL thrd_current( void )
{
    /**
     * 取得當前執行緒識別
     */
#if   defined(__linux__)
    return pthread_self();
#elif defined(_WIN32)
    return GetCurrentThread();
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
void THRDS_CALL thrd_exit( int res )
{
    /**
     * 結束當前的執行緒
     * @res 執行緒返回值
     * note : The termination of the program after the last thread. The standard seems to be contradictory
     */
#if   defined(__linux__)
    union
    {
        void     *posixval;
        intptr_t  intval;
    } thrd_result;

    thrd_result.intval = res;
    pthread_exit(thrd_result.posixval);
#elif defined(_WIN32)
    #ifdef WINCE
    ExitThread(res);
    #else
    _endthreadex(res);
    #endif
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL thrd_detach( thrd_t thr )
{
    /**
     * 分離執行緒，該執行緒會在結束時自行釋放所屬資源，同時本執行緒在乎叫此函式後便會失去對目標執行緒的控制。
     * thrd_detach 與 thrd_join 為使目標執行緒完成資源釋放的兩個方法。
     * @thr    : 指定的執行緒識別
     * @return : thrd_success if successful,
                 thrd_error otherwise.
     */
#if   defined(__linux__)
    return pthread_detach(thr) ? thrd_error : thrd_success;
#elif defined(_WIN32)
    return CloseHandle(thr) ? thrd_success : thrd_error;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL thrd_join( thrd_t thr, int *res )
{
    /**
     * 等待指定執行緒結束並釋放所屬資源
     * @thr    : 執行緒識別
     * @res    : 該執行緒的返回值
     * @return : thrd_success if successful,
     *           thrd_error otherwise.
     */
#if   defined(__linux__)
    union
    {
        void     *posixval;
        intptr_t  intval;
    } thrd_result;

    if( pthread_join(thr, &thrd_result.posixval) ) return thrd_error;
    if( res ) *res = thrd_result.intval;

    return thrd_success;
#elif defined(_WIN32)
    int ret_code = thrd_success;

    // Wait until the thread terminated
    if( WAIT_OBJECT_0 != WaitForSingleObject(thr,INFINITE) ) ret_code = thrd_error;
    // Get return value of the thread
    if( res )
    {
        DWORD exit_code;
        if( !GetExitCodeThread(thr, &exit_code) ) ret_code = thrd_error;
        else                                      *res     = exit_code;
    }
    // Close the thread
    if( !CloseHandle(thr) ) ret_code = thrd_error;

    return ret_code;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
//---- Mutex -------------------------------------------------------------------
//------------------------------------------------------------------------------
int THRDS_CALL mtx_init( mtx_t* mutex, int type )
{
    /**
     * 建立一個 Mutex 物件
     * @mutex  : 接收函式所產生的 mtx_t 物件
     * @type   : 欲建立的 Mutex 類型，可以為下列四種輸入：
     *           1 mtx_plain                 - 一個單純的、不可遞迴的 Mutex。
     *           2 mtx_timed                 - 一個不可遞迴的 Mutex，支援 Timeout。
     *           3 mtx_plain | mtx_recursive - 一個可遞迴的 Mutex。
     *           4 mtx_timed | mtx_recursive - 一個可遞迴的 Mutex，支援 Timeout。
     *           注意：
     *                 1. mtx_plain 和 mtx_timed 屬性在這裡沒有作用。
     *                 2. 較舊版本的 POSIX 系統可能不支援 mtx_recursive 屬性。
     *                 3. Windows 下只支援 ( mtx_plain | mtx_recursive ) 類型的 Mutex，因此參數 type 將會被忽略。
     * @return : thrd_success if successful,
     *           thrd_error otherwise.
     */
    if( !mutex ) return thrd_error;

#if   defined(__linux__)
    int                 error_code;
    pthread_mutexattr_t attr;
    int                 result = thrd_error;

    error_code = pthread_mutexattr_init(&attr);
    if( error_code ) return thrd_error;

    do
    {
#ifdef __USE_UNIX98
        int mutextype = ( type & mtx_recursive )?( PTHREAD_MUTEX_RECURSIVE ):( PTHREAD_MUTEX_ERRORCHECK );  // Note : The falg can set to PTHREAD_MUTEX_NORMAL for fast mutex.
        if(( error_code = pthread_mutexattr_settype(&attr, mutextype) )) break;
#else
        if( type & mtx_recursive ) break;  // Recursive mutex does not supported on this platform.
#endif
        if(( error_code = pthread_mutex_init((pthread_mutex_t*)mutex, &attr) )) break;

        result = thrd_success;
    } while(false);

    pthread_mutexattr_destroy(&attr);  // Ignore error code

    return result;
#elif defined(_WIN32)
    InitializeCriticalSection((CRITICAL_SECTION*)mutex);
    return thrd_success;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL mtx_lock( mtx_t* mutex )
{
    /**
     * 鎖定 Mutex 物件
     * @mutex  : 指定的 Mutex 物件
     * @return : thrd_success if successful,
     *           thrd_error otherwise.
     */
    if( !mutex ) return thrd_error;

#if   defined(__linux__)
    return pthread_mutex_lock((pthread_mutex_t*)mutex) ? thrd_error : thrd_success;
#elif defined(_WIN32)
    EnterCriticalSection((CRITICAL_SECTION*)mutex);
    return thrd_success;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL mtx_trylock( mtx_t* mutex )
{
    /**
     * 嘗試鎖定 Mutex 物件
     * @mutex  : 指定的 Mutex 物件
     * @return : thrd_success if successful,
     *           thrd_busy if the mutex has already been locked,
     *           thrd_error if an error occurrs.
     */
    if( !mutex ) return thrd_error;

#if   defined(__linux__)
    int error_code = pthread_mutex_trylock((pthread_mutex_t*)mutex);
    return ( error_code == 0 )?( thrd_success ):(  ( error_code == EBUSY )?( thrd_busy ):( thrd_error )  );
#elif defined(_WIN32)
    return TryEnterCriticalSection((CRITICAL_SECTION*)mutex) ?
           thrd_success : thrd_busy;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL mtx_unlock( mtx_t* mutex )
{
    /**
     * 解鎖 Mutex 物件
     * @mutex  : 指定的 Mutex 物件
     * @return : thrd_success if successful,
     *           thrd_error otherwise.
     */
    if( !mutex ) return thrd_error;

#if   defined(__linux__)
    return pthread_mutex_unlock((pthread_mutex_t*)mutex) ? thrd_error : thrd_success;
#elif defined(_WIN32)
    LeaveCriticalSection((CRITICAL_SECTION*)mutex);
    return thrd_success;
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
void THRDS_CALL mtx_destroy( mtx_t* mutex )
{
    /**
     * 銷毀 Mutex 物件
     * @mutex : 指定的 Mutex 物件
     */
#if   defined(__linux__)
    pthread_mutex_destroy((pthread_mutex_t*)mutex);  // Ignore error code
#elif defined(_WIN32)
    DeleteCriticalSection((CRITICAL_SECTION*)mutex);
#else
    #error No implementation on this platform!
#endif
}
//------------------------------------------------------------------------------
