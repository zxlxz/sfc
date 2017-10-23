#include <time.h>
#include <errno.h>
#include <pthread.h>

typedef pthread_t       thrd_t;
typedef pthread_mutex_t mtx_t;
typedef pthread_cond_t  cnd_t;

#define _THREAD_TYPES
#include "sys/posix/threads.h"

#pragma region thread

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg) {
    if (pthread_create(thr, NULL, (void*(*)(void*))func, arg) != 0) {
        *thr = 0;
    }
    return *thr ? thrd_success : thrd_error;

}

int thrd_id(thrd_t thr) {
#ifdef __APPLE__
    unsigned long long thread_id = 0;
    if (pthread_threadid_np(thr, &thread_id) == 0) {
        return (int)thread_id;
    }
    return -1;
#else
    return (int)thr;
#endif
}

thrd_t thrd_current(void) {
    return pthread_self();
}

int thrd_detach(thrd_t thr) {
    return pthread_detach(thr) == 0 ? thrd_success : thrd_error;
}

int thrd_equal(thrd_t thr0, thrd_t thr1) {
    return pthread_equal(thr0, thr1);
}

void thrd_exit(int res) {
    pthread_exit((void*)(size_t)res);
}

int thrd_join(thrd_t thr, int* res) {
    void* tmp = NULL;
    int eid = pthread_join(thr, &tmp);
    if (eid == 0 || eid == ESRCH) {
        *res = (int)(long long)tmp;
        return thrd_success;
    }
    return thrd_error;
}

int thrd_sleep(const struct timespec *duration, struct timespec *remaining) {
    int res = nanosleep(duration, remaining);
    if (res == 0) {
        return 0;
    }
    else if (errno == EINTR) {
        return -1;
    }
    else {
        return -2;
    }
}

void thrd_yield(void) {
    sched_yield();
}
#pragma endregion

#pragma region mutex

int mtx_init(mtx_t *mtx, mtx_type type) {
    int ret;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (type & mtx_recursive) {
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    }
    ret = pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return ret == 0 ? thrd_success : thrd_error;
}

void mtx_destroy(mtx_t *mtx) {
    pthread_mutex_destroy(mtx);
}

int mtx_lock(mtx_t *mtx) {
    return pthread_mutex_lock(mtx) == 0 ? thrd_success : thrd_error;
}

int mtx_timedlock(mtx_t *mtx, const struct timespec *ts) {
    int rc;
    struct timespec cur, dur;

    /* Try to acquire the lock and, if we fail, sleep for 5ms. */
    while ((rc = pthread_mutex_trylock (mtx)) == EBUSY) {
        clock_gettime(CLOCK_MONOTONIC, &cur);

        if ((cur.tv_sec > ts->tv_sec) || ((cur.tv_sec == ts->tv_sec) && (cur.tv_nsec >= ts->tv_nsec))) {
            break;
        }

        dur.tv_sec = ts->tv_sec - cur.tv_sec;
        dur.tv_nsec = ts->tv_nsec - cur.tv_nsec;
        if (dur.tv_nsec < 0) {
            dur.tv_sec--;
            dur.tv_nsec += 1000000000;
        }

        if ((dur.tv_sec != 0) || (dur.tv_nsec > 5000000)) {
            dur.tv_sec = 0;
            dur.tv_nsec = 5000000;
        }

        nanosleep(&dur, NULL);
    }

    switch (rc) {
    case 0:
        return thrd_success;
    case ETIMEDOUT:
    case EBUSY:
        return thrd_timedout;
    default:
        return thrd_error;
    }
}

int mtx_trylock(mtx_t *mtx) {
    return (pthread_mutex_trylock(mtx) == 0) ? thrd_success : thrd_busy;
}

int mtx_unlock(mtx_t *mtx) {
    return pthread_mutex_unlock(mtx) == 0 ? thrd_success : thrd_error;;
}

#pragma endregion

#pragma region condition variable
int cnd_init(cnd_t *cond) {
    return pthread_cond_init(cond, NULL) == 0 ? thrd_success : thrd_error;
}

void cnd_destroy(cnd_t *cond) {
    pthread_cond_destroy(cond);
}

int cnd_signal(cnd_t *cond) {
    return pthread_cond_signal(cond) == 0 ? thrd_success : thrd_error;
}

int cnd_broadcast(cnd_t *cond) {
    return pthread_cond_broadcast(cond) == 0 ? thrd_success : thrd_error;
}

int cnd_wait(cnd_t *cond, mtx_t *mtx) {
    return pthread_cond_wait(cond, mtx) == 0 ? thrd_success : thrd_error;
}

int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts) {
    int ret;
    ret = pthread_cond_timedwait(cond, mtx, ts);
    if (ret == ETIMEDOUT) {
        return thrd_timedout;
    }
    return ret == 0 ? thrd_success : thrd_error;
}
#pragma endregion
