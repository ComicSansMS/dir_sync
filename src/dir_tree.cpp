#include <dir_tree.hpp>

#include <exception>

#include <gbBase/Log.hpp>

#include <boost/filesystem.hpp>

#include <memory>

namespace {
Directory scanDirectory_rec(boost::filesystem::path const& p, ProgressCallback const& progress_cb, CancellationToken const& ct)
{
    if(progress_cb) { progress_cb(p); }
    Directory ret;
    ret.path = p;
    ret.modified = boost::filesystem::last_write_time(p);

    auto const it_end = boost::filesystem::directory_iterator();
    for(auto it = boost::filesystem::directory_iterator(p); it != it_end; ++it) {
        if(ct.wasCanceled()) { return Directory{}; }
        auto const it_path = it->path();
        if(boost::filesystem::is_regular_file(it_path)) {
            File f;
            f.path = *it;
            f.modified = boost::filesystem::last_write_time(it_path);
            f.size = boost::filesystem::file_size(it_path);
            ret.files.push_back(f);
        } else if(boost::filesystem::is_directory(it_path)) {
            ret.subdirs.emplace_back(scanDirectory_rec(it_path, progress_cb, ct));
        } else {
            GHULBUS_LOG(Warning, "Ignoring unsupported file of unsupported type " << it_path);
        }
    }
    return ret;
}
}


std::pair<std::future<Directory>, CancellationTokenPtr> scanDirectory(ThreadPool& thread_pool,
    boost::filesystem::path const& p, ProgressCallback const& progress_cb, FinishedCallback finished_cb)
{
    GHULBUS_LOG(Info, "Scanning " << p << "...");

    auto ct = std::make_shared<CancellationToken>();
    auto t = std::make_shared<std::packaged_task<Directory()>>([p, progress_cb, finished_cb, ct]() {
        auto ret = scanDirectory_rec(p, progress_cb, *ct);
        finished_cb();
        return ret;
    });
    auto ret = t->get_future();
    thread_pool.post([tt = std::move(t)]() { (*tt)(); });
    return std::make_pair(std::move(ret), ct);
}

