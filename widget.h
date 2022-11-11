#ifndef WIDGET_H
#define WIDGET_H

#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include <signal.h>
#include<pthread.h>
#include<stdio.h>
#include <QWidget>
#include<QDebug>
#include<QFileDialog>
#include<QDir>
#include<QList>
#include<QVector>
#include<QString>
#include<QDirIterator>
#include<QFile>
#include "label_roll.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
public:

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void curtime(int n);
    void music_singer(char *s);
    void music_album(char *s);
    void music_length(int n);
    void music_name(char *s);

private slots:
    void on_pause_clicked();

    void on_mute_clicked();

    void on_volume_valueChanged(int value);

    void on_deal_back_clicked();

    void on_deal_next_clicked();

    void on_voice_clicked();

    void on_add_music_clicked();

    void on_music_long_sliderMoved(int position);

    void on_listWidget_doubleClicked(const QModelIndex &index);

    void on_words_clicked();

private:
    Ui::Widget *ui;
};

class lyric
{
    int time;
    QString word;
};

#endif // WIDGET_H
