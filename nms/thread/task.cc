#include <nms/thread/task.h>
#include <nms/thread/thread.h>
#include <nms/test.h>

namespace nms::thread
{

#pragma region task

NMS_API ITask::ITask()
    : status_(State::None)
    , depends_{}
    , query_cnt_{ 0 }
    , semaphore_{ 0 } {}

NMS_API ITask::~ITask() {}

bool ITask::addDepend(ITask& task) {
    for (auto pdepend : depends_) {
        if (pdepend == &task) {
            return false;
        }
    }
    depends_ += &task;
    return true;
}

ITask::State ITask::status() const {
    switch (status_) {
    case Success:
    case Failed:
        break;
    default:
        ++query_cnt_;

        // waiting task completes...
        --semaphore_;
    }
    return status_;
}

NMS_API void ITask::invoke() {
    // 1. set status = Waiting
    status_ = Waiting;

    // 2. query depend tasks status
    auto failed_cnt = 0;
    for (auto ptask : depends_) {
        const auto stat = ptask->status();

        if (stat == Failed) {
            ++failed_cnt;
        }
    }
    status_ = failed_cnt == 0 ? Running : Failed;

    // 3. run this task
    if (status_ == Running) {
        try {
            onRun();
            run();
            status_ = Success;
        }
        catch (...) {
            status_ = Failed;
        }
    }

    // 4. notify 
    semaphore_ += query_cnt_;

    // 5. invoke events
    if (status_ == Success) {
        this->onSuccess();
    }
    if (status_ == Failed) {
        this->onFailed();
    }
}
#pragma endregion

#pragma region scheduler

NMS_API Scheduler::Scheduler()
{}

NMS_API Scheduler::~Scheduler()
{}

NMS_API Scheduler& Scheduler::operator+=(ITask& task) {
    tasks_.append(&task);
    return *this;
}

NMS_API void Scheduler::run() {
    List<Thread> threads_;
    threads_.reserve(tasks_.count());

    for (auto ptask : tasks_) {
        Thread thread([=] {
            ptask->invoke();
        });

        thread.setName(ptask->name());
        threads_.append(move(thread));
    }

    for (auto& thread : threads_) {
        thread.join();
    }
}
#pragma endregion

#pragma region unittest

class TestTask: public ITask
{
public:
    TestTask(char id)
        : id_(id)
        , name_(format("{:c}", id))
    {}

    StrView name() const override {
        return name_;
    }
private:
    char id_;
    String name_;

    void run() override {
        Thread::sleep(0.1);
        io::log::info("nms.thread.TestTask[{:c}]: run...", id_);
        Thread::sleep(0.1);
    }

    void onRun() override {
        io::log::warn("nms.thread.TestTask[{:c}]: start...", id_);
    }

    void onSuccess() override {
        io::log::debug("nms.thread.TestTask[{:c}]: success...", id_);
    }

    void onFailed() override {
        io::log::error("nms.thread.TestTask[{:c}]: failed", id_);
    }
};

nms_test(Task) {
    TestTask a('a');
    TestTask b('b');
    TestTask c('c');
    TestTask d('d');
    TestTask e('e');

    a >> c;
    b >> c;
    c >> e;
    d >> e;
    
    Scheduler scheduler;
    scheduler += a;
    scheduler += b;
    scheduler += c;
    scheduler += d;
    scheduler += e;

    scheduler.run();
}
#pragma endregion

}

