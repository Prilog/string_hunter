#ifndef PREPARER_H
#define PREPARER_H

#include <QCommonStyle>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDirIterator>
#include <QMap>
#include <QVector>
#include <QSet>
#include <QString>
#include <QThread>
#include <QTextStream>
#include <QCryptographicHash>

class preparer: public QObject {
    Q_OBJECT

private:
    void collect_files();
    void add_file(QFileInfo& file);
    bool check_interruption();
    void change_progress();
    uint32_t get_trigram(char a, char b, char c);
    QVector <QFileInfo> files;
    QVector<QPair<QFileInfo, QSet<uint32_t>>> trigram_file_list;
    QString root_directory;
    qint64 total_size, cur_size, cur_percent, total_text_size;
    bool is_interrupted;
    const int MAX_TRIGRAMS = 100000;

public slots:
    void do_preparing(const QString& dir);
    void find_string(const QString& str);
    void stop();

signals:
    void add_file_list(QString path);
    void set_progress_bar(qint64 value);
    void preparing_finished(bool status);
    void scaning_finished(bool status);
};

#endif // PREPARER_H
