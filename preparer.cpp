#include "preparer.h"

#include <string>

bool preparer::check_interruption() {
    return is_interrupted;
}

void preparer::do_preparing(const QString& dir) {
    is_interrupted = false;
    root_directory = dir;
    total_size = 0;
    cur_size = 0;
    cur_percent = 0;
    total_text_size = 0;
    trigram_file_list.clear();
    files.clear();
    collect_files();
    QMap <QFileInfo, QSet <QString> > m;
    for (auto file_info : files) {
        if (check_interruption()) {
            emit preparing_finished(false);
            return;
        }
        QFile file(file_info.filePath());
        bool is_text_file = false;
        char last[2];
        char buffer[1024];
        QSet<uint32_t> cur_trigrams;
        if (file.open(QIODevice::ReadOnly)) {
            is_text_file = true;
            while(!file.atEnd()) {
                qint64 buffer_size = file.read(buffer, sizeof(buffer));
                if (!cur_trigrams.empty()) {
                    cur_trigrams.insert(get_trigram(last[0], last[1], buffer[0]));
                    if (buffer_size > 1) {
                        cur_trigrams.insert(get_trigram(last[1], buffer[0], buffer[1]));
                    }
                }
                for (int i = 2; i < buffer_size; i++) {
                    cur_trigrams.insert(get_trigram(buffer[i - 2], buffer[i - 1], buffer[i]));
                }
                if (buffer_size > 1) {
                    last[0] = buffer[buffer_size - 2];
                    last[1] = buffer[buffer_size - 1];
                }
                if (cur_trigrams.size() > MAX_TRIGRAMS) {
                    is_text_file = false;
                    break;
                }
            }

        }
        if (is_text_file) {
            trigram_file_list.push_back(qMakePair(file_info, std::move(cur_trigrams)));
            total_text_size += file_info.size();
        }
        cur_size += file.size();
        change_progress();
    }
    emit preparing_finished(true);
}

void preparer::collect_files() {
    QDirIterator scanner(root_directory, QDirIterator::Subdirectories);
    while (scanner.hasNext()) {
        if (check_interruption()) {
            break;
        }
        QFileInfo f(scanner.next());
        if (!scanner.fileInfo().isDir()) {
            total_size += f.size();
            add_file(f);
        }
    }
}

void preparer::change_progress() {
     qint64 new_percent = qint64(double(100) * (double(cur_size) / double(total_size)));
     if (new_percent != cur_percent) {
         cur_percent = new_percent;
         emit set_progress_bar(cur_percent);
     }
}

void preparer::add_file(QFileInfo& file) {
    files.push_back(file);
}

uint32_t preparer::get_trigram(char a, char b, char c) {
    return  (static_cast<uint32_t>(a) + static_cast<uint32_t>(b << 8) + static_cast<uint32_t>(c << 16));
}

void preparer::find_string(const QString& str) {
    is_interrupted = false;
    emit set_progress_bar(0);
    qint64 text_read = 0;
    bool found_smth = false;
    std::string word = str.toStdString() + '\0';
    size_t w_length = word.size();
    std::vector <size_t> v(word.size());
    for (size_t i = 1; i < w_length; i++) {
        size_t j = v[i - 1];
        while (j > 0 && word[i] != word[j]) {
            j = v[j - 1];
        }
        if (word[i] == word[j]) {
            j++;
        }
        v[i] = j;
    }
    for (auto const &file_info: trigram_file_list) {
        if (check_interruption()) {
            emit scaning_finished(false);
            return;
        }
        bool flag = true;
        for (size_t i = 2; i < word.size() - 1; i++) {
            uint32_t trigram = get_trigram(word[i - 2], word[i - 1], word[i]);
            if (file_info.second.find(trigram) == file_info.second.end()) {
                flag = false;
                break;
            }
        }

        if (!flag) {
            text_read += file_info.first.size();
            emit set_progress_bar(static_cast<int>(100 * text_read / total_text_size));
            continue;
        }

        QFile file(file_info.first.filePath());
        file.open(QIODevice::ReadOnly);

        char buf[1024];
        size_t last = v.back();
        flag = false;
        while(!file.atEnd()) {
            if (check_interruption()) {
                emit scaning_finished(false);
                return;
            }
            qint64 sz = file.read(buf, sizeof(buf));
            for (qint64 i = 0; i < sz; i++) {
                size_t j = last;
                while (j > 0 && buf[i] != word[j]) {
                    j = v[j - 1];
                }
                if (buf[i] == word[j]) {
                    j++;
                }
                last = j;
                if (last + 1 == word.size()) {
                    flag = true;
                    found_smth = true;
                    emit add_file_list(file_info.first.filePath());
                    break;
                }
            }
            if (flag) {
                break;
            }
        }
        text_read += file_info.first.size();
        emit set_progress_bar(static_cast<int>(100 * text_read / total_text_size));
    }

    emit scaning_finished(found_smth);
}

void preparer::stop() {
    is_interrupted = true;
}
