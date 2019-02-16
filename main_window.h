#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDirIterator>
#include <QStandardItemModel>
#include <QThread>
#include <QVector>
#include <QPair>
#include <QTreeWidget>
#include <QFileSystemModel>
#include <QtConcurrent/QtConcurrent>
#include <QFileSystemWatcher>

#include "preparer.h"

namespace Ui {
class main_window;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = nullptr);
    ~main_window();

private slots:
    void select_directory();
    void run_preparing();
    void full_cleaning();
    void run_scanning();
    void interrupt_scanning();
    void interrupt_preparing();
    void dir_changed(const QString& str);

public slots:
    void set_progress_bar(qint64 value);
    void add_file_list(QString path);
    void scaning_finished(bool status);
    void preparing_finished(bool status);

private:
    Ui::main_window *ui;
    const QString NO_DIR_MSG = QString("No dirrectory chosen");
    const QString HEADER_DIR_MSG = QString("Chosen dirrectory: ");
    const QString NOTHING_DOING_MSG = QString("Nothing");
    const QString STARTED_SCANNING_MSG = QString("Scanning");
    const QString FINISHED_SCANNING_MSG = QString("Finished");
    const QString DIRECTORY_IS_PREPARED_MSG = QString("Dir. is prepared");
    const QString DIRECTORY_IS_UNPREPARED_MSG = QString("Dir. is unprepared");
    const QString PREPARING_MSG = QString("Preparing");
    const QString NTHG_OR_ERROR_MSG = QString("Nthg. found/interrupted");
    QStandardItemModel *model;
    QString start_dir;
    QFileSystemWatcher watcher;
    bool is_scanning;
    bool is_preparing;
    bool is_directory_prepared;
    preparer thread;

    void prepare_directory(QString const& dir);
    void scan_directory(QString const& dir);
    void add_row(QString const& dir);
    void clear_table();
};

#endif // MAIN_WINDOW_H
