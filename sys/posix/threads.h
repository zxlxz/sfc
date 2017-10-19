#ifndef _THREADS_H
#define _THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _THREAD_TYPES
struct _thrd_st;
typedef struct _thrd_st* thrd_t;

#if   defined(_WIN32)
typedef union { char data[48];  long long align; } mtx_t;
typedef struct _cnd_st*  cnd_t;
#elif defined(__linux)
typedef union { char data[40];  long long align; } mtx_t;
typedef union { char data[48];  long long align; } cnd_t;
#elif defined(__APPLE__)
typedef union { char data[64];  long long align; } mtx_t;
typedef union { char data[1];   long long align; } cnd_t;
#endif

#endif

// thread
enum {
    thrd_success,
    thrd_timedout,
    thrd_busy,
    thrd_nomem,
    thrd_error
};

typedef int(*thrd_start_t)(void*);

int     thrd_create(thrd_t *thr, thrd_start_t func, void*arg);
int     thrd_equal(thrd_t lhs, thrd_t rhs);
thrd_t  thrd_current(void);
int     thrd_sleep(const struct timespec* time_point, struct timespec* remaining);
void    thrd_yield(void);
void    thrd_exit(int res);
int     thrd_detach(thrd_t thr);
int     thrd_join(thrd_t thr, int* rhs);
int     thrd_id(thrd_t thr);

// mutex
typedef enum {
    mtx_palin,
    mtx_recursive,
    mtx_timed,
} mtx_type;

int  mtx_init(mtx_t* mutex, mtx_type type);
int  mtx_lock(mtx_t* mutex);
int  mtx_timedlock(mtx_t*  mutex, const struct timespec* time_point);
int  mtx_trylock(mtx_t* mutex);
int  mtx_unlock(mtx_t* mutex);
void mtx_destroy(mtx_t* mutex);

int  cnd_init(cnd_t *cond);
void cnd_destroy(cnd_t *cond);
int  cnd_signal(cnd_t *cond);
int  cnd_broadcast(cnd_t *cond);
int  cnd_wait(cnd_t *cond, mtx_t *mtx);
int  cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts);

#ifdef __cplusplus
    }
#endif

#endif
