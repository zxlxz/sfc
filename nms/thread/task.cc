#include <nms/test.h>
#include <nms/thread/task.h>
#include <nms/thread/thread.h>
#include <nms/util/stackinfo.h>

namespace nms::thread
{

#pragma region task

NMS_API bool ITask::exec() {
    try {
        io::log::info(">> task[{}] running...", name_);
        run();
        io::log::info("<< task[{}] success.", name_);
        return true;
    }
    catch (const IException& e) {
        auto& type_id = typeid(e);
        io::log::error("throw {}: `{}`", type_id, e);
        auto& stack_info = IException::get_stackinfo();
        io::console::writeln("{}", stack_info);

        io::log::error("<< task[{}] failed.", name_);
        return false;
    }
    catch (...) {
        io::log::error("<< task[{}] failed.", name_);
        return false;
    }
}

NMS_API void ITask::invoke() {

    // 1. set status = Waiting
    status_ = Waiting;

    // 2. query depend tasks status
    auto depends_failed_cnt = 0;
    for (auto ptask : depends_) {
        const auto stat = ptask->status();

        if (stat == Failed) {
            ++depends_failed_cnt;
        }
    }

    // 3. run this task
    if (depends_failed_cnt == 0) {
        try {
            status_ = Running;
            auto ret = exec();
            status_ = ret ? Success : Failed;
        }
        catch (...) {
        }
    }
    else {
        status_ = Failed;
    }

    // 4. notify
    semaphore_ += query_cnt_;
}

#pragma endregion

#pragma region unittest

class TestTask
    : public ITask
{
public:
    explicit TestTask(char id)
        : ITask(format("{:c}", id))
    {}

    TestTask(const TestTask&)               = delete;
    TestTask& operator=(const TestTask&)    = delete;

private:
    void run() override {
        Thread::sleep(0.1);
        io::log::info("nms.thread.TestTask[\033[33m{}\033[0m]: run...", name_);
        Thread::sleep(0.1);
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

    Scheduler<16> scheduler;
    scheduler += a;
    scheduler += b;
    scheduler += c;
    scheduler += d;
    scheduler += e;

    scheduler.run();
}
#pragma endregion

}

