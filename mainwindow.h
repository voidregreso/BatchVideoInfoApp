#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QTableView>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QHeaderView>
#include "video_info.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void browsePath();
    void processVideos();
    void populateTable(const std::vector<VideoInfo>& videoList);

private:
    void setupUI();
    void runScannerThread(const std::string& directory);
    void on_progressChanged(double progress);

    QLineEdit* pathLineEdit;
    QPushButton* browseButton;
    QPushButton* processButton;
    QTableView* tableView;
    QProgressBar* progressBar;
    VideoScanner scanner;
    QStandardItemModel* model;
    QStringList headers;
};

class NumericStandardItem : public QStandardItem
{
public:
    NumericStandardItem(const QString& text)
        : QStandardItem(text),
        numericValue(text.toDouble())
    {
    }

    bool operator <(const QStandardItem& other) const override
    {
        const NumericStandardItem* otherItem =
            dynamic_cast<const NumericStandardItem*>(&other);
        if (otherItem) {
            return this->numericValue < otherItem->numericValue;
        }
        else {
            // For non-NumericStandardItem objects, fall back to the base class comparison method
            return QStandardItem::operator<(other);
        }
    }

private:
    double numericValue;
};

#endif // MAINWINDOW_H
