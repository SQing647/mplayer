#ifndef LABEL_ROLL_H
#define LABEL_ROLL_H

#include <QtWidgets/QLabel>
#include<QTimer>
#include<QPainter>
#include<iostream>
using namespace std;

class label_roll : public QLabel
{
public:
    label_roll(QWidget *parent = 0);
    ~label_roll();

protected:
    void paintEvent(QPaintEvent *event);
    void updateIndex();

private:
    int m_charWidth;
    int m_curIndex;
    QString m_showText;
};

#endif // LABEL_ROLL_H
