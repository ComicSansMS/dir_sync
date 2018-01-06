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
    layout->addStretch();
    layout->addWidget(select2.widget);
    layout->addStretch();

    checkparallel->setText("Scan simultaneously");
    checkparallel->setChecked(true);
    layout->addWidget(checkparallel);

    buttongo->setText("Go!");
    layout->addWidget(buttongo);
}

MainWindow::MainWindow(Config const& cfg)
    :QMainWindow(), m_config(cfg), m_central(new QStackedWidget(this)), m_dirPicker(this)
{
    m_central->addWidget(m_dirPicker.widget);
    setCentralWidget(m_central);

    m_dirPicker.buttongo->setEnabled(false);
    connect(m_dirPicker.buttongo, &QPushButton::clicked, this, &MainWindow::initiateScan);

    auto const selectionChangedHandler = [this](QString const&) {
        m_dirPicker.buttongo->setEnabled(!m_dirPicker.select1.text->text().isEmpty() &&
                                         !m_dirPicker.select2.text->text().isEmpty());
    };
    connect(m_dirPicker.select1.text, &QLineEdit::textChanged, selectionChangedHandler);
    connect(m_dirPicker.select2.text, &QLineEdit::textChanged, selectionChangedHandler);
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


}

