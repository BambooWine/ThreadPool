#include <iostream>
#include <algorithm>
#include <chrono>
#include <mutex>

using namespace std;

struct SyncStream {
    SyncStream(std::ostream& _out = std::cout) : out(_out) {}

    template<typename... Args>
    void print(Args&&... args) {
        std::lock_guard<std::mutex> lk(stream_mtx);
        (out << ... << std::forward<Args>(args));
    }

    template<typename... Args>
    void println(Args&&... args) {
        print(std::forward<Args>(args)..., '\n');
    }
private:
    std::ostream& out;
    static inline std::mutex stream_mtx = {};
};

struct TimeGuard {
    using time_point = chrono::time_point<chrono::steady_clock>;
    using ms = std::chrono::microseconds;

    TimeGuard () : start(chrono::steady_clock::now()) {

    }
    
    void update_start() {
        start = chrono::steady_clock::now();
    }

    double get_duration() {
        time_point end = chrono::steady_clock::now();
        auto duration = duration_cast<ms>(end - start).count();
        return (double) duration / ms::period::den;
    }

    void print_duration() {
        SyncStream().println(get_duration(), "s");
    }
private:
    time_point start;
};