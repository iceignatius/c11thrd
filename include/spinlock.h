/*
 * Spin lock.
 *
 * @note This module is not defined in C standard!
 */
#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <threads.h>

// Identify which platforms that does not have "pthread".
#ifndef _WIN32
    #define MTX_SPIN_HAVE_PTHREAD
#endif

// Check if atomic operation supported if not use "pthread".
#ifndef MTX_SPIN_HAVE_PTHREAD
    #if __STDC_VERSION__ < 201112L || defined(__STDC_NO_ATOMICS__)
        #error Spin lock modul needs pthread or atomic operation support!
    #endif
#endif

#ifdef MTX_SPIN_HAVE_PTHREAD
    #include <pthread.h>
#else
    #include <stdatomic.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @class mtx_spin_t
 * @brief Spin lock.
 */
#ifdef MTX_SPIN_HAVE_PTHREAD
    typedef pthread_spinlock_t mtx_spin_t;
#else
    typedef atomic_flag mtx_spin_t;
#endif

int  THRDS_CALL mtx_spin_init   ( mtx_spin_t *lock );
void THRDS_CALL mtx_spin_destroy( mtx_spin_t *lock );

int  THRDS_CALL mtx_spin_lock   ( mtx_spin_t *lock );
int  THRDS_CALL mtx_spin_trylock( mtx_spin_t *lock );
int  THRDS_CALL mtx_spin_unlock ( mtx_spin_t *lock );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
