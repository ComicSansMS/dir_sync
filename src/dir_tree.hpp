#ifndef DIR_SYNC_INCLUDE_GUARD_DIR_TREE_HPP
#define DIR_SYNC_INCLUDE_GUARD_DIR_TREE_HPP

#include <thread_pool.hpp>

#include <boost/filesystem/path.hpp>

#include <atomic>
#include <functional>
#include <future>
#include <utility>
#include <vector>

struct File {
    boost::filesystem::path path;
    std::uintmax_t size;
    std::time_t modified;
};

struct Directory {
    boost::filesystem::path path;
    std::time_t modified;

    std::vector<File> files;
    std::vector<Directory> subdirs;
};

using ProgressCallback = std::function<void(boost::filesystem::path const& p)>;
using FinishedCallback = std::function<void()>;

class CancellationToken {
private:
    std::atomic<bool> m_isCanceled;
public:
    CancellationToken() : m_isCanceled(false) {}

    CancellationToken(CancellationToken const&) = delete;
    CancellationToken& operator=(CancellationToken const&) = delete;

    CancellationToken(CancellationToken&&) = delete;
    CancellationToken& operator=(CancellationToken&&) = delete;

    void cancel() { m_isCanceled.store(true); }

    bool wasCanceled() const { return m_isCanceled; }
};

using CancellationTokenPtr = std::shared_ptr<CancellationToken>;

std::pair<std::future<Directory>, CancellationTokenPtr> scanDirectory(ThreadPool& thread_pool,
    boost::filesystem::path const& p, ProgressCallback const& progress_cb, FinishedCallback finished_cb);

#endif
