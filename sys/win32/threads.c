#include <windows.h>
#include <time.h>

typedef HANDLE                  thrd_t;
typedef RTL_CRITICAL_SECTION    mtx_t;
typedef RTL_CONDITION_VARIABLE  cnd_t;

#define _THREAD_TYPES
#include "posix/threads.h"

#pragma region threads
int thrd_create(thrd_t *thr, thrd_start_t func, void *arg) {
    *thr = (thrd_t)(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL));

    return thr != NULL ? thrd_success : thrd_error;
}

thrd_t thrd_current(void) {
    return (thrd_t)(GetCurrentThread());
}

int thrd_detach(thrd_t thr) {
    /* https://stackoverflow.com/questions/12744324/how-to-detach-a-thread-on-windows-c#answer-12746081 */
    const BOOL ret =  CloseHandle(thr);
    return ret != 0 ? thrd_success : thrd_error;
}

int thrd_equal(thrd_t thr0, thrd_t thr1) {
    const DWORD tid0 = GetThreadId(thr0);
    const DWORD tid1 = GetThreadId(thr1);
    return tid0 == tid1 ? (int)tid0 : 0;
}

void thrd_exit(int res) {
    DWORD dw_res = (DWORD)(res);
    ExitThread(dw_res);
}

int thrd_join(thrd_t thr, int *res) {
    if (WaitForSingleObject(thr, INFINITE) == WAIT_FAILED) {
        return thrd_error;
    }
    if (res != NULL) {
        DWORD dw_res;
        if (GetExitCodeThread(thr, &dw_res) != 0) {
            *res = (int)(dw_res);
        }
        else {
            return thrd_error;
        }
    }
    CloseHandle(thr);
    return thrd_success;
}

int thrd_sleep(const struct timespec *duration, struct timespec *remaining) {
    struct timespec start;
    timespec_get(&start, TIME_UTC);

    // round-up
    const time_t ms =  duration->tv_sec * 1000 + (duration->tv_nsec + 500000) / 1000000;
    const DWORD  t  = SleepEx((DWORD)(ms), TRUE);

    if (t == 0) {
        return 0;
    }
    else {
        if (remaining != NULL) {
            timespec_get(remaining, TIME_UTC);
            remaining->tv_sec   -= start.tv_sec;
            remaining->tv_nsec  -= start.tv_nsec;

            if (remaining->tv_nsec < 0) {
                remaining->tv_nsec += 1000000000;
                remaining->tv_sec -= 1;
            }
        }

        return (t == WAIT_IO_COMPLETION) ? -1 : -2;
    }
}

void thrd_yield(void) {
    Sleep(0);
}

int thrd_id(thrd_t thr) {
    DWORD id = GetThreadId(thr);
    return (int)id;
}

#pragma endregion

#pragma region mutex
int mtx_init(mtx_t* mutex, mtx_type type) {
    (void)type;
    InitializeCriticalSection(mutex);
    return thrd_success;
}

int mtx_lock(mtx_t* mutex) {
    EnterCriticalSection(mutex);
    return thrd_success;
}

int mtx_trylock(mtx_t* mutex) {
    BOOL ret = TryEnterCriticalSection(mutex) ? thrd_success : thrd_busy;
    return ret;
}

int mtx_timedlock(mtx_t*  mutex, const struct timespec* time_point) {
    int rc;
    struct timespec cur, dur;

    /* Try to acquire the lock and, if we fail, sleep for 5ms. */
    while ((rc = mtx_trylock(mutex)) == thrd_busy) {
        timespec_get(&cur, TIME_UTC);

        if ((cur.tv_sec > time_point->tv_sec) || ((cur.tv_sec == time_point->tv_sec) && (cur.tv_nsec >= time_point->tv_nsec))) {
            break;
        }

        dur.tv_sec = time_point->tv_sec - cur.tv_sec;
        dur.tv_nsec = time_point->tv_nsec - cur.tv_nsec;
        if (dur.tv_nsec < 0) {
            dur.tv_sec--;
            dur.tv_nsec += 1000000000;
        }

        if ((dur.tv_sec != 0) || (dur.tv_nsec > 5000000)) {
            dur.tv_sec = 0;
            dur.tv_nsec = 5000000;
        }
        const time_t ms =  dur.tv_sec * 1000 + (dur.tv_nsec + 500000) / 1000000;
        SleepEx((DWORD)ms, FALSE);
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


int mtx_unlock(mtx_t* mutex) {
    LeaveCriticalSection(mutex);
    return thrd_success;
}

void mtx_destroy(mtx_t* mutex) {
    DeleteCriticalSection(mutex);
}
#pragma endregion

#pragma region condition variable
int cnd_init(cnd_t *cond) {
    InitializeConditionVariable(cond);
    return thrd_success;
}

void cnd_destroy(cnd_t *cond) {
    (void)cond;
}

int cnd_signal(cnd_t *cond) {
    WakeConditionVariable(cond);
    return thrd_success;
}

int cnd_broadcast(cnd_t *cond) {
    WakeAllConditionVariable(cond);
    return thrd_success;
}

int cnd_wait(cnd_t *cond, mtx_t *mtx) {
    BOOL ret = SleepConditionVariableCS(cond, mtx, INFINITE);
    return ret == TRUE ? thrd_success : thrd_error;
}

int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts) {
    DWORD ms    = (DWORD)(ts->tv_sec + (ts->tv_nsec+500000)*1000000);
    BOOL  ret   = SleepConditionVariableCS(cond, mtx, ms);

    if (ret == TRUE) {
        return thrd_success;
    }
    else {
        DWORD err = GetLastError();
        if (err == ERROR_TIMEOUT) {
            return thrd_timedout;
        }
        return thrd_error;
    }
}
#pragma endregion
