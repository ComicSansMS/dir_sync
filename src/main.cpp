#include <ui/main_window.hpp>

#include <config.hpp>

#include <QApplication>

#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>

#include <boost/filesystem.hpp>

#include <fstream>

int main(int argc, char* argv[])
{
    QApplication theApp(argc, argv);

    Ghulbus::Log::initializeLogging();
    auto log_guard = Ghulbus::finally(Ghulbus::Log::shutdownLogging);
    Ghulbus::Log::Handlers::LogToFile file_handler("dir_sync.log");
#if defined WIN32 && defined _DEBUG
    Ghulbus::Log::Handlers::LogMultiSink middle_handler(file_handler, Ghulbus::Log::Handlers::logToWindowsDebugger);
    Ghulbus::Log::setLogLevel(Ghulbus::LogLevel::Trace);
#else
    auto& middle_handler = file_handler;
    Ghulbus::Log::setLogLevel(Ghulbus::LogLevel::Warning);
#endif
    Ghulbus::Log::Handlers::LogAsync top_handler(middle_handler);
    top_handler.start();
    auto top_handler_guard = Ghulbus::finally([&top_handler]() { top_handler.stop(); });
    Ghulbus::Log::setLogHandler(top_handler);

    GHULBUS_LOG(Info, "Dir Sync up and running.");

    auto const exec_path = boost::filesystem::system_complete( boost::filesystem::path(argv[0]) ).parent_path();
    auto const cfg_path = exec_path / "dir_sync.conf";
    Config cfg = [cfg_path]() {
        if(boost::filesystem::ifstream fin(cfg_path, std::ios_base::binary); fin) {
            GHULBUS_LOG(Info, "Loading config from " << cfg_path << ".");
            Config ret(fin);
            return ret;
        } else {
            return Config();
        }
    }();
    cfg.setConfigFilePath(cfg_path);

    MainWindow main_window(cfg);
    main_window.show();

    return theApp.exec();
}

