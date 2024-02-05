#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTime>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();
    connect(&scanner, &VideoScanner::progressUpdated, this, &MainWindow::on_progressChanged);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Bulk recursive retrieval of video info");
    setWindowIcon(QIcon(":/vi.ico"));

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* topLayout = new QHBoxLayout();
    QLabel* pathLabel = new QLabel("Path:", this);
    pathLineEdit = new QLineEdit(this);
    browseButton = new QPushButton("Browse", this);
    topLayout->addWidget(pathLabel);
    topLayout->addWidget(pathLineEdit);
    topLayout->addWidget(browseButton);

    tableView = new QTableView(this);
    model = new QStandardItemModel(this);
    headers << "Name" << "Size (MB)" << "Bitrate (Kbps)" << "Frame Rate (fps)" << "Codec" << "Ratio" << "Duration";
    model->setHorizontalHeaderLabels(headers);
    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(tableView->horizontalHeader(), &QHeaderView::sectionResized, this, [this]() {
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        });
    tableView->setSortingEnabled(true);

    processButton = new QPushButton("Process", this);
    recursiveCheckBox = new QCheckBox("Recursive", this);
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    
    bottomLayout->addWidget(processButton);
    bottomLayout->addWidget(recursiveCheckBox);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tableView);
    mainLayout->addLayout(bottomLayout);
    mainLayout->addWidget(progressBar);

    connect(browseButton, &QPushButton::clicked, this, &MainWindow::browsePath);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::processVideos);
}

void MainWindow::browsePath()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"));
    pathLineEdit->setText(directory);
}

void MainWindow::processVideos()
{
    QString directory = pathLineEdit->text();
    bool isRecursive = recursiveCheckBox->isChecked();
    if (directory.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Please select a directory.");
        return;
    }

    browseButton->setEnabled(false);
    processButton->setEnabled(false);

    runScannerThread(directory.toStdString(), isRecursive);
}

void MainWindow::populateTable(const std::vector<VideoInfo>& videoList)
{
    model->clear();
    model->setHorizontalHeaderLabels(headers);

    for (const VideoInfo& video : videoList)
    {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::fromStdString(video.path));
        row << new NumericStandardItem(QString::number(video.size));
        row << new NumericStandardItem(QString::number(video.bitrate));
        row << new NumericStandardItem(QString::number(video.frameRate));
        row << new QStandardItem(QString::fromStdString(video.codec));
        row << new NumericStandardItem(QString::number(video.ratio));
        row << new QStandardItem(QTime::fromMSecsSinceStartOfDay(video.duration * 1000).toString());
        model->appendRow(row);
    }

    tableView->setModel(model);
    tableView->sortByColumn(1, Qt::DescendingOrder);
}

void MainWindow::on_progressChanged(double progress)
{
    progressBar->setValue(progress * 100);
}

void MainWindow::runScannerThread(const std::string& directory, bool isRecursive) {
    QThread* scannerThread = QThread::create([this, directory, isRecursive]() {
        std::vector<VideoInfo> videoList = scanner.TraverseVideos(directory, isRecursive);
        emit this->populateTable(videoList);
        browseButton->setEnabled(true);
        processButton->setEnabled(true);
        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        });

    connect(scannerThread, &QThread::finished, scannerThread, &QObject::deleteLater);
    scannerThread->start();
}
