#ifndef DIR_SYNC_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP
#define DIR_SYNC_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP

#include <config.hpp>

#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Config m_config;
public:
    MainWindow(Config const& cfg);

    void closeEvent(QCloseEvent* evnt) override;

public slots:
    void serializeConfig() const;
};

#endif
