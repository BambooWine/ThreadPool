# ThreadPool
Realization of lightweight thread-pool based on C++17/20
## 1. Introduction
### 1.1 Project motivation
Multithreading technology plays an important role in improving program concurrency and in modern high-performance computing. After the C++11 standard, a concurrency support library is provided, including threads, atomic operations, mutexes, conditional variables and the built-in support of the future. However, there is a certain overhead in the creation and destruction of threads, especially the frequent creation and destruction may cause the response speed of the program to decrease, and it is not convenient to manage.

Thread pool is a kind of pooled resource technology, which is responsible for managing and scheduling threads to execute tasks; The main feature is that threads can be reused to handle tasks submitted by users throughout the life cycle, avoiding the overhead caused by frequent creation and destruction of threads, improving the response speed and increasing the manageability of threads.

However, there is no corresponding thread pool framework in C++ standard, so the purpose of this project is **to realize lightweight thread pool based on C++ standard**, and at the same time meet the following requirements:
+ **High performance:** reasonably design the project structure and code logic to make the program performance and response speed as high as possible;
+ **Portability:** only rely on C++ standard library, not applicable to any third-party library;
+ **Ease of use:** provide convenient user interface and documentation, and provide necessary program operation prompt information and anomaly detection mechanism;

### 1.2 Project introduction
This project is based on **C++17 standard**, and part of it involves C++20 (don't worry about this part, just use format libraries such as std::format). The overall logic of the project is as follows:
+ Use **std::function** and **bind** to receive tasks submitted by users and generate corresponding task queues; Use **packaged_task** to delay start and **future** to get the return value of the task asynchronously;
+ Adding a new task will activate the worker thread to get the task from the head of the queue; Using mutex and unique_lock and conditional variables to realize **thread synchronization**; For some shared variables between threads, std::atomic atomic atomic type is used to complete **lock-free concurrent programming**;
+ Add optional **guardian threads**, which are responsible for: adding worker threads and destroying a certain number of idle threads when there are too many tasks;
+ Support **parallel processing of circulating tasks**, and improve CPU utilization and operation efficiency as much as possible;
+ The thread pool is lightweight, easy to use and portable. Reusing threads can avoid the overhead of creating and destroying threads by a single task, improve the response speed and increase the manageability of threads.

## 2. Method of application
### 2.1 Compilation environment
The development environment of this project is:
+ Windows 10 x64； Inter(R) Core(TM) i5-7200U CPU @2.50GHZ 2.70GHZ； 8.00GB RAM；
+ Visual Studio Code editor;
+ MinGW gcc 13.1.0, compilation parameter `-std = c++23`;
In addition, the library is compiled correctly in the following environments (minimum, C++20 is required because of using std::format):
+ MinGW gcc 13.1.0, compilation parameter `-std = c++20`;
+ MinGW clang 16.0.2, compilation parameter `-std = C++20`;
+ x86-64 gcc 13.1.0, compilation parameter `-std = C++20`;
+ x64 msvc v19.32, compilation parameter `/std: C++20`;

### 2.2 Project import
Download the source code file from this warehouse, and include `thread_pool.hpp` and `until.hpp` in the local program, where:
+ `Thread_pool.hpp`: contains the whole code of thread pool;
+ `Until.hpp`: Provide necessary tools, including synchronous output stream and time class;
```cpp
#include <iostream>
#include <string>
#include "thread_pool.hpp"

int main () {
    using namespace tp;
    
    ThreadPool pool;
    std::future<int> ans = pool.push([](int n){
        int sum = 0;
        for (int i = 0; i < n; ++i) sum += i;
        return sum;
    }, 100);
    pool.wait_until_done();
    cout << "ans: " << ans.get() << endl;
    return 0;
}
```
output:
```cpp
ans: 4950
```

### 2.3 User interface
#### 2.3.1 Constructor
```cpp
ThreadPool(size_t count = std::thread::hardware_concurrency(), bool destroy_idle = false);
```
+ `count`: the initial number of threads in the thread pool;
+ `destroy_idle`: whether the daemon thread needs to destroy the idle thread, the default is false;
For the number of threads, there are `THREADS_MAX` and `THREADS_MIN` restrictions, which can be customized according to actual tasks.

#### 2.3.2 Add tasks
```cpp
template<typename F, typename... Args>
    decltype(auto) push(F&& f, Args&&... args);
```
This function is a template method:
+ `f`: function caller, including function pointer, imitation function, lambda, function, etc.
+ `args`: function parameter, which is a parameter package type;
The `push` method will receive the task submitted by the user and add it to the task queue, waiting for the worker thread to respond; Use `std::invoke_result_t` to get the return value type of the task. This `push` method returns a variable of type `future<return_type>`, indicating the return value of the user task, and then get the actual value through the `get` interface.
```cpp
template<typename F, typename T>
    decltype(auto) push_loop(F&& f, const T start, const T end, const size_t num_blocks = 0);
```
Similar to `push`, the function of this function is: to submit tasks that are executed in cycles (loops are not related);

+ `f`: function caller;
+ `start`: cycle start interval, integer type;
+ `end`: cycle end interval, integer type; It should be emphasized that it is consistent with the standard library, and the cycle interval is **left closed and right open**;
+ `num_blocks`: divide the loop task into several sub-blocks, and the default is STD:: thread:: hardware _ concurrency ();
#### 2.3.3 Thread Pool Status Operation
+ `pause`: pause the thread pool;
+ `resume`: the thread pool continues to execute;
+ `clear`: clear the remaining tasks;
+ `wait_until_done`: Wait for all tasks to be completed;
+ `wait_for`: Wait for all tasks to be completed (for a period of time);
```cpp
template<typename _Rep, typename _Period>
    bool wait_for(const chrono::duration<_Rep, _Period>& __rtime);
```

#### 2.3.4 Thread pool attribute acquisition
+ `get_threads_count`: get the current number of surviving threads;
+ `get_threads_running`: get the number of threads currently running;
+ `get_tasks_count`: get the number of tasks in the task queue, that is, the number of remaining tasks;
+ `get_tasks_total`: get the current number of all unfinished tasks;
+ `is_running`: judge whether the thread pool is running;
+ `is_closed`: determines whether the thread pool is closed;
+ `is_waiting`: judge whether the thread pool is waiting for the task to be completed;
+ `is_paused`: judge whether the thread pool is paused;
+ `is_empty`: judge whether the task queue is empty;

#### 2.3.5 Tool class
**SyncStream:** synchronous output stream:
Function prototype:
```cpp
template<typename... Args>
    void println(Args&&... args);
```
Use: 
```cpp
SyncStream().println(std::format("Bob is {} years old.", 25));
```
Output:
```cpp
Bob is 25 years old.
```
**TimeGuard:** Time management class, which can be used to print running time;
Use:
```cpp
TimeGuard tg;
// ...
tg.print_duration();			          // Printing time consuming
tg.update_start()			              // update start
// ...
auto interval = tg.get_duration();	// Get time consuming
```
## 3. Testing and use cases
### 3.1 Submit tasks
In this test, a user task `ff` is constructed; Comparing the efficiency of this thread pool with `std::thread` and `std::async`, the test code is as follows:
```cpp
struct AA {
    string s;
};

long long ff(int n, int id) {
    long long ans = 0;
    AA arr[1000];
    for (int i = 0; i < n; ++i) {
        ans += i;
        arr[min(i, 999)].s = to_string(i);
    }
    return ans;
}

void test_task() {
    SyncStream().println("\n========== Test Tasks ==========");
    TimeGuard tg;
    const int mx = 100;
    {
        using namespace tp;
        ThreadPool pool(8);
        vector<std::future<long long>> v(mx);
        for (int i = 0; i < mx; ++i) {
            auto r = pool.push(ff, (i + 1) * 10000, i + 1);
            v[i] = std::move(r);
        }
        for (int i = 0; i < mx; ++i) {
            v[i].get();
        }
    }
    SyncStream().println(std::format("Thread-pool cost {:.3f}s", tg.get_duration()));
    tg.update_start();
    {
        thread t[mx];
        for (int i = 0; i < mx; ++i) {
            t[i] = thread{ff, (i + 1) * 10000, i + 1};
        }
        for (int i = 0; i < mx; ++i) t[i].join();
    }
    SyncStream().println(std::format("std::thread cost {:.3f}s", tg.get_duration()));
    tg.update_start();
    {
        vector<std::future<long long>> v(mx);
        for (int i = 0; i < mx; ++i) {
            auto r = std::async(ff, (i + 1) * 10000, i + 1);
            v[i] = std::move(r);
        }
        for (int i = 0; i < mx; ++i) {
            v[i].get();
        }
    }
    SyncStream().println(std::format("std::async cost {:.3f}s", tg.get_duration()));
}
```
### 3.2 Parallel execution cycle task
For some unrelated tasks between loops, **parallelization** can be used; Now compare the efficiency of thread pool parallelization and serial execution; The test code is as follows:
```cpp
void test_loop() {
    constexpr int len = 1000;
    vector<int> arr(len);
    vector<AA> ss(len);
    auto loop = [&arr, &ss](const int start, const int end) {
        for (int i = start; i < end; ++i) {
            size_t sum = 0;
            for (size_t j = 0; j < i * 1000; ++j) sum += j;
            arr[i] = sum;
            for (size_t j = 0; j < i * 50; ++j) {
                ss[i].s += to_string(j * i * 12344321);
            }
        }
    };
    SyncStream().println("\n========== Test Loop ========== ");
    TimeGuard tg;
    {
        using namespace tp;
        ThreadPool pool(8);
        pool.push_loop(loop, 0, len, 8);
        pool.wait_until_done();
    }
    SyncStream().println(std::format("parallel loop costs {:.3f}s", tg.get_duration()));
    ss = vector<AA>(len);
    tg.update_start();
    {
        loop(0, len);
    }
    SyncStream().println(std::format("Serial loop costs {:.3f}s", tg.get_duration()));
}
```
### 3.3 Test effect diagram
The comparison test effect diagram is as follows:

![image-20230801113830197](https://bamboowine-img-1259155549.cos.ap-beijing.myqcloud.com/img/image-20230801113830197.png)

+ Test Tasks: There is little difference among them. In the current experiment, the efficiency of thread pool is slightly lower than `std::async` (there may be thread scheduling overhead).
+ Test Loop: the efficiency of thread pool is more than twice that of serial;

## 4. About the project
+ If you are interested in this project, you are welcome to pull the warehouse or download the code for local use;
+ If you encounter any mistakes in use or want to add new functions, you can publish an issue； at any time;
+ If you think this project is useful, you can consider giving it a star;
