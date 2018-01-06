#ifndef DIR_SYNC_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP
#define DIR_SYNC_INCLUDE_GUARD_UI_MAIN_WINDOW_HPP

#include <config.hpp>
#include <dir_tree.hpp>
#include <thread_pool.hpp>

#include <QMainWindow>

#include <QBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
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

    struct Scanner {
        QWidget* widget;
        QVBoxLayout* layout;
        QLabel* labelHeader;
        QProgressBar* progress;
        QLabel* label1;
        QLabel* label2;
        QPushButton* cancelButton;

        CancellationTokenPtr cancelScan[2];
        std::future<Directory> scanResult[2];
        int scanFinishedCount;

        Scanner(MainWindow* parent);
    } m_scanner;

    struct Diff {
        Directory dir1;
        Directory dir2;

        QWidget* widget;
        QHBoxLayout* outerLayout;
        QListWidget* list;

        QVBoxLayout* buttonsLayout;
        QPushButton* okButton;

        Diff(MainWindow* parent);
    } m_diff;

    ThreadPool m_threadPool;
public:
    MainWindow(Config const& cfg);

    void closeEvent(QCloseEvent* evnt) override;

public slots:
    void serializeConfig() const;

    void initiateScan();
    void cancelScanning();
    void finishedScanning();

signals:
    void scanProgress1(QString path);
    void scanProgress2(QString path);

    void scanFinished(int idx);
    void allScansFinished();
};

#endif
