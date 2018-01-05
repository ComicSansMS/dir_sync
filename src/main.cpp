#include <ui/main_window.hpp>

#include <QApplication>

#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>

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

    MainWindow main_window;
    main_window.show();

    return theApp.exec();
}

