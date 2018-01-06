#include <thread_pool.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

ThreadPool::ThreadPool(int n_threads)
    :m_done(false)
{
    GHULBUS_PRECONDITION(n_threads > 0);
    GHULBUS_LOG(Debug, "Spawning thread pool with " << n_threads << " threads.");
    m_threads.reserve(n_threads);
    for(int i = 0; i < n_threads; ++i) {
        m_threads.emplace_back([this]() {
            for(;;) {
                std::unique_lock<std::mutex> lk(m_mtx);
                m_condvar.wait(lk, [this]() {
                    return m_done || !m_work.empty();
                });
                if(!m_work.empty()) {
                    auto work = m_work.front();
                    m_work.pop_front();
                    lk.unlock();
                    work();
                } else {
                    GHULBUS_ASSERT(m_done);
                    break;
                }
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    GHULBUS_LOG(Debug, "Shutting down thread pool.");
    m_done = true;
    m_condvar.notify_all();
    for(auto& t : m_threads) { t.join(); }
    GHULBUS_ASSERT(m_work.empty());
}

void ThreadPool::post(Work&& work)
{
    std::lock_guard<std::mutex> lk(m_mtx);
    m_work.emplace_back(std::move(work));
    m_condvar.notify_one();
}
