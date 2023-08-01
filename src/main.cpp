#include <iostream>
#include <string>
#include "thread_pool.hpp"

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
    // SyncStream().println(std::format("Thread {}: {}", id, ans));
    return ans;
}

void test_task() {
    SyncStream().println("\n========== Test Taaks ==========");
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
    SyncStream().println(std::format("Thread-pool cost total {:.3f}s", tg.get_duration()));
    tg.update_start();
    {
        thread t[mx];
        for (int i = 0; i < mx; ++i) {
            t[i] = thread{ff, (i + 1) * 10000, i + 1};
        }
        for (int i = 0; i < mx; ++i) t[i].join();
    }
    SyncStream().println(std::format("std::thread cost total {:.3f}s", tg.get_duration()));
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
    SyncStream().println(std::format("std::async cost total {:.3f}s", tg.get_duration()));
}

void test_loop() {
    constexpr int len = 1'000;
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

int main() {
    test_task();
    test_loop();
    return 0;
}