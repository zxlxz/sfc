# Cyber

Cyber 通信模型

```mermaid
graph LR

subgraph thread_pool
    direction LR
    Thread.1
    Thread.2
    Thread.3
end

subgraph scheduler
    direction LR
    TaskQueue.RealTime
    TaskQueue.High
    TaskQueue.Normal
    TaskQueue.Low
end

subgraph channel
Chan.A -- transmite --> Dispatcher.A
Chan.B -- transmite --> Dispatcher.B
end

scheduler  ==> thread_pool

Dispatcher.A
Send.A[Send.A] --> Chan.A
Dispatcher.A --> Listener.A1 -. bind .-> Recv.A1 -- Task --> scheduler
Dispatcher.A --> Listener.A2 -. bind .-> Recv.A2 -- Task --> scheduler

Dispatcher.B
Send.B1 --> Chan.B
Send.B2 --> Chan.B
Dispatcher.B --> Listener.B -. bind .-> Recv.B[Recv] -- Task --> scheduler
```

Cyber 代码示例

```c++
struct Message{};

void on_message(Message msg) {
    //...
}

void test() {
    // recv
    cyber::Recv<Message> recv{"ni.adas.test"};
    recv.bind(on_message);

    // send
    cyber::Send<Message> send{"ni.adas.test"};
    send.send(msg...);
}
```
