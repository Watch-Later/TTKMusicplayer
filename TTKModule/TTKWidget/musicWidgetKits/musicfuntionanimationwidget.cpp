#include "musicfuntionanimationwidget.h"
#include "musicleftitemlistuiobject.h"
#include "musicfunctionlistuiobject.h"
#include "musicimageutils.h"
#include "musicwidgetheaders.h"

#include <qmath.h>

#include <QButtonGroup>
#include <QPropertyAnimation>

MusicBackgroundWidget::MusicBackgroundWidget(QWidget *parent)
    : QWidget(parent)
{

}

void MusicBackgroundWidget::backgroundTransparent(int value)
{
    m_backgroundAlpha = MusicUtils::Image::reRenderValue<int>(0xFF, 0x10, MV_MAX - value);
    update();
}

void MusicBackgroundWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.fillRect(rect(), QColor(255, 255, 255, m_backgroundAlpha));
}



MusicLineBackgroundWidget::MusicLineBackgroundWidget(QWidget *parent)
    : QWidget(parent)
{
    m_transparent = false;
}

void MusicLineBackgroundWidget::transparent(bool state)
{
    m_transparent = state;
    update();
}

void MusicLineBackgroundWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(QPen(QBrush(QColor(0, 0, 0)), 0.1, Qt::SolidLine));

    if(!m_transparent)
    {
        painter.fillRect(rect(), Qt::white);
        painter.drawLine(0, height(), width(), height());
    }
}



MusicAbstractAnimationWidget::MusicAbstractAnimationWidget(QWidget *parent)
    : QWidget(parent),
      m_pix(":/toolSets/btn_arrow_normal"),
      m_curIndex(0),
      m_preIndex(0),
      m_x(0),
      m_perWidth(0.0f),
      m_totalWidth(0.0f),
      m_isAnimation(true),
      m_showState(true),
      m_showLine(true)
{
    m_animation = new QPropertyAnimation(this, "");
    m_animation->setDuration(100);

    connect(m_animation, SIGNAL(valueChanged(QVariant)), SLOT(animationChanged(QVariant)));
    connect(m_animation, SIGNAL(finished()), SLOT(finished()));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    m_buttonGroup = new QButtonGroup(this);
    QtButtonGroupConnect(m_buttonGroup, this, switchToSelectedItemStyle);
}

MusicAbstractAnimationWidget::~MusicAbstractAnimationWidget()
{
    qDeleteAll(m_container);
    delete m_animation;
    delete m_buttonGroup;
}

void MusicAbstractAnimationWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if(m_showState)
    {
        m_perWidth = m_container[0]->width() + m_container[0]->x();

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.setPen(QPen(QBrush(QColor(0, 0, 0)), 0.1, Qt::SolidLine));

        int offset = m_perWidth - (m_container[0]->width() + m_pix.width()) / 2;
            offset = m_isAnimation ? (offset + m_x) : (offset + m_curIndex * m_perWidth);
        if(m_showLine)
        {
            painter.drawLine(0, height(), offset, height());
            painter.drawLine(offset + m_pix.width(), height(), m_totalWidth, height());
        }
        painter.drawPixmap(offset, height() - m_pix.height(), m_pix);
    }
}

void MusicAbstractAnimationWidget::switchToSelectedItemStyle(int index)
{
    m_isAnimation = true;
    m_preIndex = m_curIndex;
    m_curIndex = index;
    m_animation->setStartValue(m_preIndex * m_perWidth);
    m_animation->setEndValue(index * m_perWidth);
    m_animation->start();

    Q_EMIT buttonClicked(index);
}

void MusicAbstractAnimationWidget::animationChanged(const QVariant &value)
{
    m_x = value.toInt();
    update();
}

void MusicAbstractAnimationWidget::finished()
{
    m_isAnimation = false;
}



MusicFuntionAnimationWidget::MusicFuntionAnimationWidget(QWidget *parent)
    : MusicAbstractAnimationWidget(parent)
{
    QHBoxLayout *ly = TTKObject_cast(QHBoxLayout*, layout());

    QStringList names;
    names << tr("Playlist") << tr("Local") << tr("Cloud") << tr("Radio") << tr("Download");
    for(int i = 0; i < names.count(); ++i)
    {
        QToolButton *btn = new QToolButton(this);
        btn->setToolTip(names[i]);
        btn->setFixedSize(20, 20);
        btn->setCursor(Qt::PointingHandCursor);
        ly->addWidget(btn);
        m_buttonGroup->addButton(btn, i);
        m_container << btn;
    }

    switchToSelectedItemStyle(0);
}

void MusicFuntionAnimationWidget::paintEvent(QPaintEvent *event)
{
    m_totalWidth = width();
    MusicAbstractAnimationWidget::paintEvent(event);
}

void MusicFuntionAnimationWidget::switchToSelectedItemStyle(int index)
{
    m_container[0]->setStyleSheet(MusicUIObject::MQSSItemMusic);
    m_container[1]->setStyleSheet(MusicUIObject::MQSSItemLocal);
    m_container[2]->setStyleSheet(MusicUIObject::MQSSItemCloud);
    m_container[3]->setStyleSheet(MusicUIObject::MQSSItemRadio);
    m_container[4]->setStyleSheet(MusicUIObject::MQSSItemDownload);

    switch(index)
    {
        case 0: m_container[0]->setStyleSheet(MusicUIObject::MQSSItemMusicClicked); break;
        case 1: m_container[1]->setStyleSheet(MusicUIObject::MQSSItemLocalClicked); break;
        case 2: m_container[2]->setStyleSheet(MusicUIObject::MQSSItemCloudClicked); break;
        case 3: m_container[3]->setStyleSheet(MusicUIObject::MQSSItemRadioClicked); break;
        case 4: m_container[4]->setStyleSheet(MusicUIObject::MQSSItemDownloadClicked); break;
        default: break;
    }

    MusicAbstractAnimationWidget::switchToSelectedItemStyle(index);
}



MusicOptionAnimationWidget::MusicOptionAnimationWidget(QWidget *parent)
    : MusicAbstractAnimationWidget(parent)
{
    m_pix = QPixmap(54, 2);
    m_pix.fill(QColor(0x80, 0xB7, 0xF1));
    m_showLine = false;

    QHBoxLayout *ly = TTKObject_cast(QHBoxLayout*, layout());

    for(int i = 0; i < 6; ++i)
    {
        QToolButton *btn = new QToolButton(this);
        btn->setFixedSize(54, 23);
        btn->setCursor(Qt::PointingHandCursor);
        ly->addWidget(btn);
        m_buttonGroup->addButton(btn, i);
        m_container << btn;
    }

    switchToSelectedItemStyle(0);
}

void MusicOptionAnimationWidget::musicButtonStyleClear(bool fore)
{
    m_container[0]->setStyleSheet(fore ? MusicUIObject::MQSSFuncSongFore : MusicUIObject::MQSSFuncSongBack);
    m_container[1]->setStyleSheet(fore ? MusicUIObject::MQSSFuncRadioFore : MusicUIObject::MQSSFuncRadioBack);
    m_container[2]->setStyleSheet(fore ? MusicUIObject::MQSSFuncListFore : MusicUIObject::MQSSFuncListBack);
    m_container[3]->setStyleSheet(fore ? MusicUIObject::MQSSFuncMVFore : MusicUIObject::MQSSFuncMVBack);
    m_container[4]->setStyleSheet(fore ? MusicUIObject::MQSSFuncLiveFore : MusicUIObject::MQSSFuncLiveBack);
    m_container[5]->setStyleSheet(MusicUIObject::MQSSFuncLrcFore);
}

void MusicOptionAnimationWidget::musicButtonStyle(int index)
{
    switch(index)
    {
        case 0: m_container[0]->setStyleSheet(MusicUIObject::MQSSFuncSongForeClicked); break;
        case 1: m_container[1]->setStyleSheet(MusicUIObject::MQSSFuncRadioForeClicked); break;
        case 2: m_container[2]->setStyleSheet(MusicUIObject::MQSSFuncListForeClicked); break;
        case 3: m_container[3]->setStyleSheet(MusicUIObject::MQSSFuncMVForeClicked); break;
        case 4: m_container[4]->setStyleSheet(MusicUIObject::MQSSFuncLiveForeClicked); break;
        case 5: m_container[5]->setStyleSheet(MusicUIObject::MQSSFuncLrcForeClicked); break;
        default: break;
    }
}

void MusicOptionAnimationWidget::paintEvent(QPaintEvent *event)
{
    m_totalWidth = width();
    MusicAbstractAnimationWidget::paintEvent(event);
}

void MusicOptionAnimationWidget::switchToSelectedItemStyle(int index)
{
    MusicAbstractAnimationWidget::switchToSelectedItemStyle(index);
    m_showState = (index != 5);
    update();
}



MusicSkinAnimationWidget::MusicSkinAnimationWidget(QWidget *parent)
    : MusicAbstractAnimationWidget(parent)
{
    QHBoxLayout *ly = TTKObject_cast(QHBoxLayout*, layout());

    QStringList names;
    names << tr("Recommend") << tr("Stack") << tr("Daily") << tr("Online");
    for(int i = 0; i < names.count(); ++i)
    {
        QToolButton *btn = new QToolButton(this);
        btn->setText(names[i]);
        btn->setFixedSize(80, 30);
        btn->setCursor(Qt::PointingHandCursor);
        ly->addWidget(btn);
        m_buttonGroup->addButton(btn, i);
        m_container << btn;
    }
    ly->addStretch(1);

    switchToSelectedItemStyle(0);
}

void MusicSkinAnimationWidget::paintEvent(QPaintEvent *event)
{
    m_totalWidth = width();
    MusicAbstractAnimationWidget::paintEvent(event);
}

void MusicSkinAnimationWidget::switchToSelectedItemStyle(int index)
{
    for(QWidget *widget : qAsConst(m_container))
    {
        widget->setStyleSheet(MusicUIObject::MQSSColorStyle03 + MusicUIObject::MQSSBackgroundStyle01);
    }

    if(index < 0 || index >= m_container.count())
    {
        return;
    }

    m_container[index]->setStyleSheet(MusicUIObject::MQSSColorStyle08 + MusicUIObject::MQSSBackgroundStyle01);
    MusicAbstractAnimationWidget::switchToSelectedItemStyle(index);
}
