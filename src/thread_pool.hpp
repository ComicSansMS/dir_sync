#ifndef DIR_SYNC_INCLUDE_GUARD_THREAD_POOL_HPP
#define DIR_SYNC_INCLUDE_GUARD_THREAD_POOL_HPP

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <thread>
#include <vector>

class ThreadPool {
public:
    using Work = std::function<void()>;
private:
    std::vector<std::thread> m_threads;

    bool m_done;
    std::deque<Work> m_work;
    std::condition_variable m_condvar;
    std::mutex m_mtx;
public:
    ThreadPool(int n_threads);
    ~ThreadPool();

    ThreadPool(ThreadPool const&) = delete;
    ThreadPool& operator=(ThreadPool const&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void post(Work&& work);
};

#endif
