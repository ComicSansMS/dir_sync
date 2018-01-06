#ifndef DIR_SYNC_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP
#define DIR_SYNC_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP

#include <config.hpp>

#include <QMainWindow>

#include <QBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Config m_config;
    QStackedWidget* m_central;

    struct DirPicker {
        QWidget* widget;
        QVBoxLayout* layout;
        struct Selector {
            QWidget* widget;
            QHBoxLayout* layout;
            QLineEdit* text;
            QPushButton* button;

            Selector(QWidget* parent);
        };
        Selector select1;
        Selector select2;
        QCheckBox* checkparallel;
        QPushButton* buttongo;

        DirPicker(MainWindow* parent);
    } m_dirPicker;

public:
    MainWindow(Config const& cfg);

    void closeEvent(QCloseEvent* evnt) override;

public slots:
    void serializeConfig() const;

    void initiateScan();
};

#endif
