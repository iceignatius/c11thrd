/*
 * C11 Thread Library
 */
#ifndef _THREADS_H_
#define _THREADS_H_

#ifdef __linux__
    #include <pthread.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

#if   defined(__linux__)
    #define THRDS_CALL
#elif defined(_WIN32)
    #define THRDS_CALL __cdecl
#else
    #define THRDS_CALL
#endif

#if   defined(__linux__)
    typedef int(THRDS_CALL *thrd_start_t)(void*);
    typedef pthread_t thrd_t;
#elif defined(_WIN32)
    typedef int(THRDS_CALL *thrd_start_t)(void*);
    typedef void* thrd_t;
#else
    #error No implementation on this platform!
#endif

enum
{
    thrd_success,
    thrd_timedout,
    thrd_busy,
    thrd_nomem,
    thrd_error
};

int    THRDS_CALL thrd_create( thrd_t *thr, thrd_start_t func, void *arg );
int    THRDS_CALL thrd_equal( thrd_t lhs, thrd_t rhs );
thrd_t THRDS_CALL thrd_current( void );
void   THRDS_CALL thrd_exit( int res );
int    THRDS_CALL thrd_detach( thrd_t thr );
int    THRDS_CALL thrd_join( thrd_t thr, int *res );

typedef struct mtx_t
{
    char bindata[40];
} mtx_t;

enum
{
    mtx_plain,
    mtx_recursive,
    mtx_timed
};

int  THRDS_CALL mtx_init   ( mtx_t* mutex, int type );  // 注意：
                                                        //       1. mtx_plain 和 mtx_timed 屬性在這裡沒有作用。
                                                        //       2. Windows 下只支援 ( mtx_plain | mtx_recursive ) 類型的 Mutex，因此參數 type 將會被忽略。
int  THRDS_CALL mtx_lock   ( mtx_t* mutex );
int  THRDS_CALL mtx_trylock( mtx_t* mutex );
int  THRDS_CALL mtx_unlock ( mtx_t* mutex );
void THRDS_CALL mtx_destroy( mtx_t* mutex );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
