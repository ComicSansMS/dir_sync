#include <ui/main_window.hpp>

#include <gbBase/Log.hpp>

#include <boost/filesystem/fstream.hpp>

MainWindow::MainWindow(Config const& cfg)
    :QMainWindow(), m_config(cfg)
{
}

void MainWindow::closeEvent(QCloseEvent* evnt)
{
    serializeConfig();
    QMainWindow::closeEvent(evnt);
}

void MainWindow::serializeConfig() const
{
    GHULBUS_LOG(Trace, "Saving config to " << m_config.getConfigFilePath() << ".");
    boost::filesystem::ofstream fout(m_config.getConfigFilePath(), std::ios_base::binary);
    m_config.serialize(fout);
    if(!fout) {
        GHULBUS_LOG(Error, "Error writing to config file.");
    }
}

