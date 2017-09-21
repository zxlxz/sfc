#pragma once

#include <nms/core.h>
#include <nms/thread/thread.h>
#include <nms/thread/semaphore.h>

namespace  nms::thread
{

class ITask;
class IScheduler;

/*! task */
class ITask
{
    friend class IScheduler;
public:
    using Tdepends  = List<ITask*, 16>;
    using Tname     = U8String<64>;

    enum State
    {
        None,       // none
        Waiting,    // wait depends to be complete.
        Running,    // this task is running.
        Success,    // task run success.
        Failed,     // task run failed.
    };

    virtual ~ITask()
    { }

    /* copy constructor is disabled */
    ITask(const ITask&)             = delete;

    /* assign operator is disabled */
    ITask& operator=(const ITask&)  = delete;

    /* make self depend on task */
    bool addDepend(ITask& task) {
        for (auto pdepend : depends_) {
            if (pdepend == &task) {
                return false;
            }
        }
        depends_ += &task;
        return true;
    }

    /* query task status */
    State status() const {
        switch (status_) {
        case Success:
        case Failed:
            break;

        case None:
            break;

        case Waiting:
        case Running:
        default:
            ++query_cnt_;

            // waiting task completes...
            --semaphore_;
        }
        return status_;
    }

    /* get task name */
    auto& name() const {
        return name_;
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
    Tdepends                depends_    = {};
    Tname                   name_       = {};
    mutable Semaphore       semaphore_  = {};
    mutable volatile u32    query_cnt_  = 0;
    State                   status_     = State::None;

    explicit ITask(const StrView& name)
        : name_(name)
    { }

    /*! task run method */
    virtual void run() = 0;

    /*! task exec method */
    NMS_API virtual bool exec();

    /*! invoke this task to run */
    NMS_API void invoke();
};

/*! task scheduler */
class IScheduler
{
protected:
    IScheduler() = default;

public:
    IScheduler(const IScheduler&)            = delete;
    IScheduler& operator=(const IScheduler&) = delete;

    virtual void run()  = 0;

protected:
    static void _run(IList<Thread>& threads, IList<ITask*>& tasks) {
        for (auto ptask : tasks) {
            threads.append([=] { ptask->invoke(); });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }
};

/*! task scheduler */
template<u32 Icapicity = 64>
class Scheduler final
    : public IScheduler
{
public:
    constexpr static auto $capacity = Icapicity;

    using base   = IScheduler;
    using Ttasks = List<ITask*, $capacity>;

    Scheduler() = default;
    ~Scheduler()= default;

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    Scheduler& operator+=(ITask& task) {
        tasks_.append(&task);
        return *this;
    }

    void run() override {
        List<Thread, $capacity> threads;
        base::_run(threads, tasks_);
    }

private:
    Ttasks  tasks_ = {};
};

}
