#include <ui/main_window.hpp>

#include <QFileDialog>
#include <QMessageBox>

#include <gbBase/Log.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <algorithm>

namespace {
bool contains(boost::filesystem::path p1, boost::filesystem::path p2)
{
    auto const p1_len = std::distance(p1.begin(), p1.end());
    auto const p2_len = std::distance(p2.begin(), p2.end());
    if(p1_len < p2_len) {
        std::swap(p1, p2);
    }

    auto it1 = p1.begin();
    for(auto it = p2.begin(), it_end = p2.end(); it != it_end; ++it, ++it1) {
        if(*it != *it1) {
            return false;
        }
    }

    return true;
}
}

MainWindow::DirPicker::Selector::Selector(QWidget* parent)
    :widget(new QWidget(parent)), layout(new QHBoxLayout(widget)), text(new QLineEdit(widget)),
     button(new QPushButton(widget))
{
    text->setMinimumWidth(400);
    layout->addWidget(text);

    button->setText("...");
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, [this]() {
        auto const dir = QFileDialog::getExistingDirectory(widget, "Select directory to sync");
        text->setText(dir);
    });
}

MainWindow::DirPicker::DirPicker(MainWindow* parent)
    :widget(new QWidget(parent)), layout(new QVBoxLayout(widget)),
     select1(widget), select2(widget), checkparallel(new QCheckBox(widget)), buttongo(new QPushButton(widget))
{
    layout->addWidget(select1.widget);
    layout->addWidget(select2.widget);
    layout->addStretch();

    checkparallel->setText("Scan simultaneously");
    checkparallel->setChecked(true);
    layout->addWidget(checkparallel);

    buttongo->setText("Go!");
    layout->addWidget(buttongo);
}

MainWindow::Scanner::Scanner(MainWindow* parent)
    :widget(new QWidget(parent)), layout(new QVBoxLayout(widget)), labelHeader(new QLabel(widget)),
     progress(new QProgressBar(widget)), label1(new QLabel(widget)), label2(new QLabel(widget)),
     cancelButton(new QPushButton(widget)), scanFinishedCount(-1)
{
    layout->addWidget(labelHeader);
    layout->addWidget(progress);
    layout->addWidget(label1);
    layout->addWidget(label2);
    cancelButton->setText("Cancel");
    layout->addWidget(cancelButton);
}

MainWindow::Diff::Diff(MainWindow* parent)
    :widget(new QWidget(parent)), outerLayout(new QHBoxLayout(widget)), list(new QListWidget(widget)),
     buttonsLayout(new QVBoxLayout(widget)), okButton(new QPushButton(widget))
{
    outerLayout->addWidget(list);

    okButton->setText("Sync!");
    buttonsLayout->addWidget(okButton);
    outerLayout->addLayout(buttonsLayout);
}


MainWindow::MainWindow(Config const& cfg)
    :QMainWindow(), m_config(cfg), m_central(new QStackedWidget(this)), m_dirPicker(this), m_scanner(this),
     m_diff(this), m_threadPool(2)
{
    m_central->addWidget(m_dirPicker.widget);
    m_central->addWidget(m_scanner.widget);
    m_central->addWidget(m_diff.widget);
    m_central->setCurrentWidget(m_dirPicker.widget);
    setCentralWidget(m_central);

    m_dirPicker.buttongo->setEnabled(false);
    connect(m_dirPicker.buttongo, &QPushButton::clicked, this, &MainWindow::initiateScan);

    auto const selectionChangedHandler = [this](QString const&) {
        m_dirPicker.buttongo->setEnabled(!m_dirPicker.select1.text->text().isEmpty() &&
                                         !m_dirPicker.select2.text->text().isEmpty());
    };
    connect(m_dirPicker.select1.text, &QLineEdit::textChanged, selectionChangedHandler);
    connect(m_dirPicker.select2.text, &QLineEdit::textChanged, selectionChangedHandler);

    connect(this, &MainWindow::scanProgress1,
            this, [this](QString p) { m_scanner.label1->setText("Scanning " + p + "..."); }, Qt::QueuedConnection);
    connect(this, &MainWindow::scanProgress2,
            this, [this](QString p) { m_scanner.label2->setText("Scanning " + p + "..."); }, Qt::QueuedConnection);
    connect(this, &MainWindow::scanFinished,
            this, [this](int i) {
                        ((i == 0) ? m_scanner.label1 : m_scanner.label2)->setText("");
                        if(++m_scanner.scanFinishedCount == 2) { emit allScansFinished();
                    } },
            Qt::QueuedConnection);
    connect(this, &MainWindow::allScansFinished, this, &MainWindow::finishedScanning);
    connect(m_scanner.cancelButton, &QPushButton::clicked, this, &MainWindow::cancelScanning);
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

void MainWindow::initiateScan()
{
    auto const dir1 = boost::filesystem::path(m_dirPicker.select1.text->text().toStdString());
    auto const dir2 = boost::filesystem::path(m_dirPicker.select2.text->text().toStdString());

    if(!boost::filesystem::is_directory(dir1)) {
        QMessageBox::warning(this, "Error opening directory #1",
                             "Not a directory:\n\n" + QString::fromStdString(dir1.generic_string()));
        return;
    }
    if(!boost::filesystem::is_directory(dir2)) {
        QMessageBox::warning(this, "Error opening directory #2",
                             "Not a directory:\n\n" + QString::fromStdString(dir2.generic_string()));
        return;
    }
    if(dir1 == dir2) {
        QMessageBox::warning(this, "Error syncing directories",
                             "Cannot sync directory with itself.");
        return;
    }

    if(contains(dir1, dir2)) {
        QMessageBox::warning(this, "Error syncing directories",
                             "Cannot sync a directory with its subdirectory.");
        return;
    }

    m_scanner.labelHeader->setText("Scanning...");
    m_scanner.progress->setMinimum(0);
    m_scanner.progress->setMaximum(0);
    m_scanner.label1->setText("Scanning " + QString::fromStdString(dir1.generic_string()));
    m_scanner.label2->setText("Scanning " + QString::fromStdString(dir2.generic_string()));
    m_central->setCurrentWidget(m_scanner.widget);

    m_scanner.scanFinishedCount = 0;
    auto const progress1 = [this](boost::filesystem::path const& p) {
        GHULBUS_LOG(Trace, "Scanning " << p);
        emit scanProgress1(QString::fromStdString(p.generic_string()));
    };
    auto const progress2 = [this](boost::filesystem::path const& p) {
        GHULBUS_LOG(Trace, "Scanning " << p);
        emit scanProgress2(QString::fromStdString(p.generic_string()));
    };
    auto const finished1 = [this]() {
        m_scanner.cancelScan[0].reset();
        emit scanFinished(0);
    };
    auto const finished2 = [this]() {
        m_scanner.cancelScan[1].reset();
        emit scanFinished(1);
    };
    if(m_dirPicker.checkparallel->isChecked()) {
        auto [dir_res1, ct1] = scanDirectory(m_threadPool, dir1, progress1, finished1);
        m_scanner.cancelScan[0] = ct1;
        m_scanner.scanResult[0] = std::move(dir_res1);
        auto [dir_res2, ct2] = scanDirectory(m_threadPool, dir2, progress2, finished2);
        m_scanner.cancelScan[1] = ct2;
        m_scanner.scanResult[1] = std::move(dir_res2);
    } else {
        auto [dir_res1, ct1] = scanDirectory(m_threadPool, dir1, progress1,
            [this, dir2, finished1, finished2, progress2]() {
                finished1();
                auto [dir_res2, ct2] = scanDirectory(m_threadPool, dir2, progress2, finished2);
                m_scanner.cancelScan[1] = ct2;
                m_scanner.scanResult[1] = std::move(dir_res2);
            });
        m_scanner.cancelScan[0] = ct1;
        m_scanner.scanResult[0] = std::move(dir_res1);
    }
}

void MainWindow::cancelScanning()
{
    if(m_scanner.cancelScan[0]) { m_scanner.cancelScan[0]->cancel(); }
    m_scanner.cancelScan[0].reset();
    if(m_scanner.cancelScan[1]) { m_scanner.cancelScan[1]->cancel(); }
    m_scanner.cancelScan[1].reset();
}

void compareDirectories_rec(Directory const& d1, Directory const& d2,
                            boost::filesystem::path const& base_d1, boost::filesystem::path const& base_d2,
                            QListWidget* list)
{
    if(boost::filesystem::relative(d1.path, base_d1) != boost::filesystem::relative(d2.path, base_d2)) {
        list->addItem(QString::fromStdString(d1.path.generic_string()));
    }
    for(auto const& f : d1.files) {
        auto relf1 = boost::filesystem::relative(f.path, base_d1);
        auto it_file2 = std::find_if(d2.files.begin(), d2.files.end(), [base_d2, relf1](auto f2) {
            auto relf2 = boost::filesystem::relative(f2.path, base_d2);
            return relf2 == relf1;
        });
        if(it_file2 == d2.files.end()) {
            // file not present in d2; add to list
            list->addItem(QString::fromStdString(f.path.generic_string()));
        } else {
            if((it_file2->size != f.size) || (it_file2->modified != f.modified)) {

            }
        }
    }
    for(auto const& d : d1.subdirs) {
        auto reld1 = boost::filesystem::relative(d.path, base_d1);
        auto it_dir2 = std::find_if(d2.subdirs.begin(), d2.subdirs.end(), [base_d2, reld1](auto subd2) {
            auto reld2 = boost::filesystem::relative(subd2.path, base_d2);
            return reld2 == reld1;
        });
        if(it_dir2 == d2.subdirs.end()) {
            // dir not present in d2; add to list
            list->addItem(QString::fromStdString(d.path.generic_string()));
        } else {
            compareDirectories_rec(d, *it_dir2, base_d1, base_d2, list);
        }
    }
}

void MainWindow::finishedScanning()
{
    GHULBUS_LOG(Info, "Finished scan.");
    m_central->setCurrentWidget(m_diff.widget);
    m_diff.dir1 = std::move(m_scanner.scanResult[0].get());
    m_diff.dir2 = std::move(m_scanner.scanResult[1].get());

    auto const& dir1 = m_diff.dir1;
    auto const& dir2 = m_diff.dir2;

    auto const dir1_prefix = dir1.path;
    auto const dir2_prefix = dir2.path;

    compareDirectories_rec(dir1, dir2, dir1_prefix, dir2_prefix, m_diff.list);
}

