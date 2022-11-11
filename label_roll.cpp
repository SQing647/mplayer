#include "label_roll.h"
//实现歌曲名的滚动效果
label_roll::label_roll(QWidget *parent):QLabel(parent)
{
    setMinimumWidth(145);
    setMinimumHeight(40);

    m_curIndex = 0;//当前角码
    m_charWidth = fontMetrics().width("a");//每个字符的宽度

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &label_roll::updateIndex);
    timer->start(1000);
}

label_roll::~label_roll()
{

}

void label_roll::paintEvent(QPaintEvent *event)
{
    m_showText = this->text();
    QPainter painter(this);
    painter.drawText(0, 10, m_showText.mid(m_curIndex));
//    painter.drawText(width() - m_charWidth*m_curIndex, 10, m_showText.left(m_curIndex));
}

void label_roll::updateIndex()
{
    update();
    m_curIndex++;
    if (m_curIndex*m_charWidth > width())
        m_curIndex = 0;
}
