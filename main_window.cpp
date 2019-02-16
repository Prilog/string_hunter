#include "main_window.h"
#include "ui_main_window.h"

main_window::main_window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::main_window)
{
    ui->setupUi(this);
    is_directory_prepared = false;
    is_scanning = false;
    is_preparing = false;

    //Flags Setting
    is_scanning = false;

    //Text Settings
    ui->folder_label->setText(NO_DIR_MSG);
    ui->status_label->setText(DIRECTORY_IS_UNPREPARED_MSG);

    //Button Settings
    ui->progress_bar->setValue(0);
    connect(ui->select_button, SIGNAL (released()), this, SLOT (select_directory()));
    connect(ui->prepare_button, SIGNAL (released()), this, SLOT (run_preparing()));
    connect(ui->clear_button, SIGNAL (released()), this, SLOT (full_cleaning()));
    connect(ui->run_button, SIGNAL (released()), this, SLOT (run_scanning()));
    connect(ui->interrupt_scan_button, SIGNAL (released()), this, SLOT (interrupt_scanning()));
    connect(ui->interrupt_prep_button, SIGNAL (released()), this, SLOT (interrupt_preparing()));

    //Preparer Setting
    connect(&thread, &preparer::set_progress_bar, this, &main_window::set_progress_bar);
    connect(&thread, &preparer::add_file_list, this, &main_window::add_file_list);
    connect(&thread, &preparer::preparing_finished, this, &main_window::preparing_finished);
    connect(&thread, &preparer::scaning_finished, this, &main_window::scaning_finished);

    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &main_window::dir_changed);

    //Table Settings
    ui->table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    model = new QStandardItemModel(0,1,this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Path")));
    model->setRowCount(0);
    ui->table_view->setModel(model);
    ui->table_view->setColumnWidth(0, 600);

    //Progress bar Settings
    ui->progress_bar->setMinimum(0);
    ui->progress_bar->setMaximum(100);
    ui->progress_bar->setValue(0);
}

void main_window::select_directory()
{
    if (!is_scanning && !is_preparing) {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (dir.size() != 0) {
            full_cleaning();
            is_directory_prepared = false;
            ui->status_label->setText(DIRECTORY_IS_UNPREPARED_MSG);
            start_dir = dir;
            ui->folder_label->setText(HEADER_DIR_MSG + start_dir);
            set_progress_bar(0);
        }
    }
}

void main_window::run_preparing()
{
    if (start_dir.size() != 0 && !is_directory_prepared && !is_preparing && !is_scanning) {
        is_preparing = true;
        is_directory_prepared = false;
        ui->status_label->setText(PREPARING_MSG);
        watcher.addPath(start_dir);
        prepare_directory(start_dir);
    }
}

void main_window::clear_table()
{
    delete model;
    model = new QStandardItemModel(0,1,this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Path")));
    model->setRowCount(0);
    ui->table_view->setModel(model);
}

void main_window::full_cleaning() {
    if (!is_preparing && !is_scanning) {
        start_dir.clear();
        ui->progress_bar->setValue(0);
        ui->line_edit->setText("");
        clear_table();
        ui->status_label->setText(is_directory_prepared ? DIRECTORY_IS_PREPARED_MSG : DIRECTORY_IS_UNPREPARED_MSG);
        ui->line_edit->setEnabled(true);
        QPalette pal = ui->line_edit->palette();
        pal.setColor(QPalette::Base, QColor(255,255,255));
        ui->line_edit->setPalette(pal);
    }
}

void main_window::interrupt_scanning() {
    if (is_scanning) {
        is_scanning = false;
        thread.stop();
    }
}

void main_window::interrupt_preparing() {
    if (is_preparing) {
        thread.stop();
        is_preparing = false;
        is_directory_prepared = false;
    }
}

void main_window::run_scanning()
{
    QString word = ui->line_edit->text();
    if (word.size() != 0 && is_directory_prepared && !is_preparing && !is_scanning) {
        ui->line_edit->setEnabled(false);
        QPalette pal = ui->line_edit->palette();
        pal.setColor(QPalette::Base, QColor(255,255,0));
        ui->line_edit->setPalette(pal);
        clear_table();
        is_scanning = true;
        ui->status_label->setText(STARTED_SCANNING_MSG);
        scan_directory(word);
    }
}

void main_window::add_file_list(QString path) {
    add_row(path);
}

void main_window::add_row(QString const& dir)
{
    QStandardItem* item = new QStandardItem(dir);
    model->setRowCount(model->rowCount() + 1);
    model->setItem(model->rowCount() - 1, 0, item);
    ui->table_view->setModel(model);
}

void main_window::set_progress_bar(qint64 value) {
    ui->progress_bar->setValue(int(value));
}

void main_window::scaning_finished(bool status) {
    if (status) {
        ui->status_label->setText(FINISHED_SCANNING_MSG);
        set_progress_bar(100);
    } else {
        ui->status_label->setText(NTHG_OR_ERROR_MSG);
    }
    ui->line_edit->setEnabled(true);
    QPalette pal = ui->line_edit->palette();
    pal.setColor(QPalette::Base, QColor(255,255,255));
    ui->line_edit->setPalette(pal);
    is_scanning = false;
}

void main_window::preparing_finished(bool status) {
    if (status) {
        ui->status_label->setText(DIRECTORY_IS_PREPARED_MSG);
        is_directory_prepared = true;
        set_progress_bar(100);
    } else {
        ui->status_label->setText(DIRECTORY_IS_UNPREPARED_MSG);
    }
    is_preparing = false;
}

void main_window::prepare_directory(QString const& dir)
{
    QtConcurrent::run(&thread, &preparer::do_preparing, dir);
}

void main_window::scan_directory(QString const& dir)
{
    QtConcurrent::run(&thread, &preparer::find_string, dir);
}

void main_window::dir_changed(const QString& str) {
    if (is_directory_prepared) {
        ui->status_label->setText(DIRECTORY_IS_UNPREPARED_MSG);
        is_directory_prepared = false;
        set_progress_bar(0);
        if (is_preparing) {
            interrupt_preparing();
        }
        if (is_scanning) {
            interrupt_scanning();
        }
    }
}

main_window::~main_window()
{
    delete ui;
}
