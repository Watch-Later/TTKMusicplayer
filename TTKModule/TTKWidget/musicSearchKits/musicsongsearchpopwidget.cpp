#include "musicsongsearchpopwidget.h"
#include "musicsongsearchrecordconfigmanager.h"
#include "musicwidgetheaders.h"

MusicSongSearchPopTableWidget::MusicSongSearchPopTableWidget(QWidget *parent)
    : MusicAbstractTableWidget(parent)
{
    setColumnCount(2);

    QHeaderView *headerview = horizontalHeader();
    headerview->resizeSection(0, 215);
    headerview->resizeSection(1, 62);

    MusicUtils::Widget::setTransparent(this, 255);
}

MusicSongSearchPopTableWidget::~MusicSongSearchPopTableWidget()
{
    removeItems();
}

void MusicSongSearchPopTableWidget::addCellItem(int index, const QString &name, const QString &time)
{
    setRowHeight(index, TTK_ITEM_SIZE_M);

    QHeaderView *headerview = horizontalHeader();
    QTableWidgetItem *item = new QTableWidgetItem(MusicUtils::Widget::elidedText(font(), "  " + name, Qt::ElideRight, headerview->sectionSize(0) - 20));
    item->setToolTip(name);
    item->setForeground(QColor(MusicUIObject::MQSSColor02));
    QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
    setItem(index, 0, item);

                      item = new QTableWidgetItem(time);
    item->setForeground(QColor(MusicUIObject::MQSSColor02));
    QtItemSetTextAlignment(item, Qt::AlignCenter);
    setItem(index, 1, item);
}

void MusicSongSearchPopTableWidget::itemCellClicked(int row, int column)
{
    MusicAbstractTableWidget::itemCellClicked(row, column);

    Q_EMIT setText(item(row, 0)->toolTip().trimmed());

    QWidget *widget = TTKObject_cast(QWidget*, parent());
    widget->lower();
    widget->hide();
}

void MusicSongSearchPopTableWidget::removeItems()
{
    MusicAbstractTableWidget::removeItems();
    setColumnCount(2);
}


MusicSongSearchPopWidget::MusicSongSearchPopWidget(QWidget *parent)
    : QWidget(parent)
{
    move(405, 45);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);

    m_popTableWidget = new MusicSongSearchPopTableWidget(this);
    m_popTableWidget->setFixedWidth(285);
    m_clearButton = new QPushButton("   " + tr("Clear History"), this);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    m_clearButton->setFixedHeight(35);
    m_clearButton->setStyleSheet(MusicUIObject::MQSSCustomStyle01 + MusicUIObject::MQSSFontStyle02 + MusicUIObject::MQSSColorStyle03 + MusicUIObject::MQSSBorderStyle01 + MusicUIObject::MQSSBackgroundStyle10);
#ifdef Q_OS_UNIX
    m_clearButton->setFocusPolicy(Qt::NoFocus);
#endif

    QFrame *frame = new QFrame(this);
    frame->setFixedHeight(1);
    frame->setStyleSheet(MusicUIObject::MQSSBackgroundStyle09);
    frame->setFrameShape(QFrame::HLine);

    layout->addWidget(m_popTableWidget);
    layout->addWidget(frame);
    layout->addWidget(m_clearButton);
    setLayout(layout);

    connect(m_clearButton, SIGNAL(clicked()), SLOT(clearButtonClicked()));
    connect(m_popTableWidget, SIGNAL(setText(QString)), SIGNAL(setText(QString)));
}

MusicSongSearchPopWidget::~MusicSongSearchPopWidget()
{
    delete m_popTableWidget;
    delete m_clearButton;
}

void MusicSongSearchPopWidget::initialize()
{
    m_clearButton->show();
    m_popTableWidget->removeItems();

    MusicSongSearchRecordConfigManager search(this);
    if(!search.fromFile())
    {
        return;
    }

    MusicSearchRecordList records;
    search.readBuffer(records);

    const int count = records.count();
    resize(m_popTableWidget->width() + 2, count == 0 ? 0 : (count < 6 ? count * TTK_ITEM_SIZE_M + 45 : 7 * TTK_ITEM_SIZE_M + 8));

    m_popTableWidget->setRowCount(count);
    for(int i = 0; i < count; ++i)
    {
        m_popTableWidget->addCellItem(i, records[i].m_name, utcTimeToLocal(records[i].m_timestamp));
    }
}

void MusicSongSearchPopWidget::createSuggestItems(const QStringList &names)
{
    m_clearButton->hide();
    m_popTableWidget->removeItems();

    const int count = names.count();
    resize(m_popTableWidget->width() + 2, count == 0 ? 0 : (count < 6 ? count * TTK_ITEM_SIZE_M + 8 : 6 * TTK_ITEM_SIZE_M + 8));

    m_popTableWidget->setRowCount(count);
    for(int i = 0; i < count; ++i)
    {
        m_popTableWidget->addCellItem(i, names[i], QString());
    }
}

QString MusicSongSearchPopWidget::utcTimeToLocal(const QString &time) const
{
    const qint64 t = (TTKTime::timestamp() - time.toLongLong()) / MT_S2MS;
    if(t < MT_M2S)
    {
        return QString::number(t) + tr("ss");
    }
    else if(MT_M2S <= t && t < MT_H2S)
    {
        return QString::number(t / MT_M2S) + tr("mm");
    }
    else if(MT_H2S <= t && t < MT_D2S)
    {
        return QString::number(t / MT_H2S) + tr("hh");
    }
    else
    {
        return QString::number(t / MT_D2S) + tr("day");
    }
}

void MusicSongSearchPopWidget::clearButtonClicked()
{
    MusicSongSearchRecordConfigManager search(this);
    if(!search.fromFile())
    {
        return;
    }

    search.writeBuffer(MusicSearchRecordList());
    close();
}

void MusicSongSearchPopWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::gray);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}

void MusicSongSearchPopWidget::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    lower();
    hide();
}
