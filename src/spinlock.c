#include "../include/spinlock.h"

#ifdef MTX_SPIN_HAVE_PTHREAD
    #include <errno.h>
#endif

//------------------------------------------------------------------------------
int THRDS_CALL mtx_spin_init( mtx_spin_t *lock )
{
    /**
     * @memberof mtx_spin_t
     * @brief Constructor.
     *
     * @param lock Object instance.
     * @return ::thrd_success if successful; and
     *         ::thrd_error otherwise.
     */
#ifdef MTX_SPIN_HAVE_PTHREAD
    return pthread_spin_init(lock, PTHREAD_PROCESS_SHARED) ?
           thrd_error : thrd_success;
#else
    atomic_flag flag = ATOMIC_FLAG_INIT;
    *lock = flag;
    return thrd_success;
#endif
}
//------------------------------------------------------------------------------
void THRDS_CALL mtx_spin_destroy( mtx_spin_t *lock )
{
    /**
     * @memberof mtx_spin_t
     * @brief Destructor.
     *
     * @param lock Object instance.
     * @return ::thrd_success if successful; and
     *         ::thrd_error otherwise.
     */
#ifdef MTX_SPIN_HAVE_PTHREAD
    pthread_spin_destroy(lock);
#else
    // Nothing to do.
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL mtx_spin_lock( mtx_spin_t *lock )
{
    /**
     * @memberof mtx_spin_t
     * @brief Lock the lock.
     *
     * @param lock Object instance.
     * @return ::thrd_success if successful; and
     *         ::thrd_error otherwise.
     */
#ifdef MTX_SPIN_HAVE_PTHREAD
    return pthread_spin_lock(lock) ? thrd_error : thrd_success;
#else
    while( atomic_flag_test_and_set_explicit(lock, memory_order_relaxed) )
    {
        (void)0;  // Busy wait.
    }

    return thrd_success;
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL mtx_spin_trylock( mtx_spin_t *lock )
{
    /**
     * @memberof mtx_spin_t
     * @brief Try to lock the lock.
     *
     * @param lock Object instance.
     * @return ::thrd_success if successful; and
     *         ::thrd_busy if the lock has already been locked; and
     *         ::thrd_error otherwise.
     */
#ifdef MTX_SPIN_HAVE_PTHREAD
    int errcode = pthread_spin_trylock(lock);
    return ( errcode == EBUSY )?( thrd_busy ):( errcode ? thrd_error : thrd_success );
#else
    return atomic_flag_test_and_set_explicit(lock, memory_order_relaxed) ?
           thrd_busy : thrd_success;
#endif
}
//------------------------------------------------------------------------------
int THRDS_CALL mtx_spin_unlock( mtx_spin_t *lock )
{
    /**
     * @memberof mtx_spin_t
     * @brief Unlock the lock.
     *
     * @param lock Object instance.
     * @return ::thrd_success if successful; and
     *         ::thrd_error otherwise.
     */
#ifdef MTX_SPIN_HAVE_PTHREAD
    return pthread_spin_unlock(lock) ? thrd_error : thrd_success;
#else
    atomic_flag_clear_explicit(lock, memory_order_relaxed);
    return thrd_success;
#endif
}
//------------------------------------------------------------------------------
