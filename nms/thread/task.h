#pragma once

#include <nms/core.h>
#include <nms/thread/semaphore.h>

namespace  nms::thread
{

class ITask;
class Scheduler;

/* task */
class ITask
{
    friend class Scheduler;

public:
    enum State
    {
        None,       // none
        Waiting,    // wait depends to be complete.
        Running,    // this task is running.
        Success,    // task run success.
        Failed,     // task run failed.
    };

    NMS_API virtual ~ITask();

    /* make self depend on task */
    NMS_API bool addDepend(ITask& task);

    /* query task status */
    NMS_API State status() const;

    NMS_API virtual StrView name() const {
        return "nms::thread::ITask";
    }

    /* a << b: set a depend on b */
    friend ITask& operator<<(ITask& a, ITask& b) {
        a.addDepend(b);
        return b;
    }

    /* a >> b: set b depend on a */
    friend ITask& operator>>(ITask& a, ITask& b) {
        b.addDepend(a);
        return b;
    }

protected:
    State               status_;
    List<ITask*>        depends_;

    NMS_API ITask();

    /* task run method */
    virtual void run() = 0;

    /* task exec method */
    virtual bool exec() {
        try {
            run();
            return true;
        }
        catch (const IException& e) {
            e.dump();
            return false;
        }
        catch (...) {
            return false;
        }
    }

private:
    mutable volatile u32    query_cnt_;
    mutable Semaphore       semaphore_;

    /* invoke this task to run */
    NMS_API void invoke();
};

/* task scheduler */
class Scheduler
    : public INocopyable
{
public:
    NMS_API Scheduler();
    NMS_API ~Scheduler();
    NMS_API void run();

    NMS_API Scheduler& operator+=(ITask& task);

private:
    List<ITask*> tasks_;
};

}
