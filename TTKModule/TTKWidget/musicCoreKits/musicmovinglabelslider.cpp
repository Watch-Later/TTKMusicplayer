#include "musicmovinglabelslider.h"
#include "ttktime.h"

MusicMovingLabelSlider::MusicMovingLabelSlider(QWidget *parent)
    : MusicMovingLabelSlider(Qt::Horizontal, parent)
{

}

MusicMovingLabelSlider::MusicMovingLabelSlider(Qt::Orientation orientation, QWidget *parent)
    : MusicMovingClickedSlider(orientation, parent)
{
    m_textLabel = new QLabel(this);
    m_textLabel->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    m_textLabel->setGeometry(0, 0, 40, 20);
    m_textLabel->setAlignment(Qt::AlignCenter);
    m_textLabel->setFocusPolicy(Qt::NoFocus);
    m_textLabel->setStyleSheet(MusicUIObject::MQSSLabelStyle01);
}

MusicMovingLabelSlider::~MusicMovingLabelSlider()
{
    delete m_textLabel;
}

void MusicMovingLabelSlider::mousePressEvent(QMouseEvent *event)
{
    MusicMovingClickedSlider::mousePressEvent(event);
#ifdef Q_OS_UNIX
    m_textLabel->show();
#endif
    m_textLabel->raise();
}

void MusicMovingLabelSlider::mouseMoveEvent(QMouseEvent *event)
{
    MusicMovingClickedSlider::mouseMoveEvent(event);

    const QPoint &curPos = mapFromGlobal(QCursor::pos());
    const QPoint &glbPos = mapToGlobal(QPoint(0, 0));
    const QSize &sizePos = size();
    QPoint changePos;

    if(orientation() == Qt::Vertical)
    {
        changePos = limitLableGeometry(curPos.y(), glbPos.y(), sizePos.height());
        m_textLabel->move((glbPos + QPoint(sizePos.width(), 0)).x(), changePos.x());
    }
    else
    {
        changePos = limitLableGeometry(curPos.x(), glbPos.x(), sizePos.width());
        m_textLabel->move(changePos.x(), (glbPos - QPoint(0, m_textLabel->height())).y());
    }
    m_textLabel->setText(TTKTime::formatDuration(changePos.y()));
}

void MusicMovingLabelSlider::enterEvent(QtEnterEvent *event)
{
    MusicMovingClickedSlider::enterEvent(event);
#ifndef Q_OS_UNIX
    m_textLabel->show();
#endif
}

void MusicMovingLabelSlider::leaveEvent(QEvent *event)
{
    MusicMovingClickedSlider::leaveEvent(event);
    m_textLabel->hide();
}

QPoint MusicMovingLabelSlider::limitLableGeometry(int x, int y, int z)
{
    QPoint pt;
    if(0 < x && x < z)
    {
        pt.setX(y + x);
        pt.setY(qint64(x) * maximum() / z);
    }
    if(x <= 0)
    {
        pt.setX(y);
        pt.setY(0);
    }
    if(x >= z)
    {
        pt.setX(y + z);
        pt.setY(maximum());
    }
    return pt;
}
