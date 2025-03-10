#include "musicsongchecktoolstablewidget.h"
#include "musicsongssummariziedwidget.h"
#include "musicitemdelegate.h"
#include "musicnumberutils.h"
#include "musicconnectionpool.h"

static TTKPushButtonItemDelegate *MakeButtonItemDelegate(QObject *parent)
{
    TTKPushButtonItemDelegate *delegate = new TTKPushButtonItemDelegate(parent);
    delegate->setStyleSheet(MusicUIObject::MQSSBorderStyle03 + MusicUIObject::MQSSBorderStyle06 + MusicUIObject::MQSSBackgroundStyle10);
    return delegate;
}


MusicSongCheckToolsRenameTableWidget::MusicSongCheckToolsRenameTableWidget(QWidget *parent)
    : MusicFillItemTableWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setColumnCount(4);

    QHeaderView *headerview = horizontalHeader();
    headerview->resizeSection(0, 30);
    headerview->resizeSection(1, 290);
    headerview->resizeSection(2, 290);
#ifdef Q_OS_UNIX
    headerview->resizeSection(3, 51);
#else
    headerview->resizeSection(3, 54);
#endif

    setItemDelegateForColumn(3, MakeButtonItemDelegate(this));
}

void MusicSongCheckToolsRenameTableWidget::addCellItems(const MusicSongCheckToolsRenameList &items)
{
    setRowCount(items.count());
    QHeaderView *headerview = horizontalHeader();

    for(int i = 0; i < items.count(); ++i)
    {
        const MusicSongCheckToolsRename &v = items[i];

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(TTK_CHECKED_ROLE, Qt::Unchecked);
        setItem(i, 0, item);

                          item = new QTableWidgetItem;
        item->setToolTip(v.m_locaName);
        item->setText(MusicUtils::Widget::elidedText(font(), item->toolTip(), Qt::ElideRight, headerview->sectionSize(1) - 10));
        QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(i, 1, item);

                item = new QTableWidgetItem;
        item->setToolTip(v.m_recommendName);
        item->setText(MusicUtils::Widget::elidedText(font(), item->toolTip(), Qt::ElideRight, headerview->sectionSize(2) - 10));
        QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(i, 2, item);

                item = new QTableWidgetItem;
        item->setData(TTK_DISPLAY_ROLE, tr("Delete"));
        setItem(i, 3, item);
    }
}

void MusicSongCheckToolsRenameTableWidget::itemCellClicked(int row, int column)
{
    MusicFillItemTableWidget::itemCellClicked(row, column);
    if(column == 3)
    {
        deleteCurrentRow();
    }
}

void MusicSongCheckToolsRenameTableWidget::deleteCurrentRow()
{
    if(!isValid())
    {
        return;
    }

    removeRow(currentRow());
}



MusicSongCheckToolsDuplicateTableWidget::MusicSongCheckToolsDuplicateTableWidget(QWidget *parent)
    : MusicFillItemTableWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setColumnCount(7);

    QHeaderView *headerview = horizontalHeader();
    headerview->resizeSection(0, 30);
    headerview->resizeSection(1, 290);
    headerview->resizeSection(2, 65);
    headerview->resizeSection(3, 65);
    headerview->resizeSection(4, 80);
    headerview->resizeSection(5, 80);
#ifdef Q_OS_UNIX
    headerview->resizeSection(6, 51);
#else
    headerview->resizeSection(6, 54);
#endif

    setItemDelegateForColumn(5, MakeButtonItemDelegate(this));
    setItemDelegateForColumn(6, MakeButtonItemDelegate(this));

    G_CONNECTION_PTR->setValue(className(), this);
    G_CONNECTION_PTR->connect(className(), MusicSongsSummariziedWidget::className());
}

MusicSongCheckToolsDuplicateTableWidget::~MusicSongCheckToolsDuplicateTableWidget()
{
    G_CONNECTION_PTR->removeValue(this);
}

void MusicSongCheckToolsDuplicateTableWidget::addCellItems(const MusicSongCheckToolsDuplicateList &songs)
{
    setRowCount(songs.count());
    QHeaderView *headerview = horizontalHeader();

    for(int i = 0; i < songs.count(); ++i)
    {
        const MusicSongCheckToolsDuplicate &v = songs[i];

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(TTK_CHECKED_ROLE, Qt::Unchecked);
        setItem(i, 0, item);

                          item = new QTableWidgetItem;
        item->setToolTip(v.m_song.name());
        item->setText(MusicUtils::Widget::elidedText(font(), item->toolTip(), Qt::ElideRight, headerview->sectionSize(1) - 45));
        QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(i, 1, item);

                item = new QTableWidgetItem;
        item->setText(v.m_song.playTime());
        QtItemSetTextAlignment(item, Qt::AlignCenter);
        setItem(i, 2, item);

                item = new QTableWidgetItem;
        item->setText(v.m_song.sizeStr());
        QtItemSetTextAlignment(item, Qt::AlignCenter);
        setItem(i, 3, item);

                item = new QTableWidgetItem;
        item->setText(v.m_bitrate);
        QtItemSetTextAlignment(item, Qt::AlignCenter);
        setItem(i, 4, item);

                item = new QTableWidgetItem;
        item->setData(TTK_DISPLAY_ROLE, tr("Play"));
        item->setData(TTK_DATA_ROLE, v.m_song.path());
        setItem(i, 5, item);

                item = new QTableWidgetItem;
        item->setData(TTK_DISPLAY_ROLE, tr("Delete"));
        setItem(i, 6, item);
    }
}

void MusicSongCheckToolsDuplicateTableWidget::itemCellClicked(int row, int column)
{
    MusicFillItemTableWidget::itemCellClicked(row, column);
    switch(column)
    {
        case 5: musicPlay(); break;
        case 6: deleteCurrentRow(); break;
        default: break;
    }
}

void MusicSongCheckToolsDuplicateTableWidget::musicPlay()
{
    if(!isValid())
    {
        return;
    }

    const QTableWidgetItem *it = item(currentRow(), 5);
    if(it)
    {
        const QString &path = it->data(TTK_DATA_ROLE).toString();
        Q_EMIT addSongToPlaylist(QStringList(QFile::exists(path) ? path : QString()));
    }
}

void MusicSongCheckToolsDuplicateTableWidget::deleteCurrentRow()
{
    if(!isValid())
    {
        return;
    }

    removeRow(currentRow());
}



MusicSongCheckToolsQualityTableWidget::MusicSongCheckToolsQualityTableWidget(QWidget *parent)
    : MusicFillItemTableWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setColumnCount(8);

    QHeaderView *headerview = horizontalHeader();
    headerview->resizeSection(0, 30);
    headerview->resizeSection(1, 220);
    headerview->resizeSection(2, 65);
    headerview->resizeSection(3, 65);
    headerview->resizeSection(4, 80);
    headerview->resizeSection(5, 70);
    headerview->resizeSection(6, 80);
#ifdef Q_OS_UNIX
    headerview->resizeSection(7, 51);
#else
    headerview->resizeSection(7, 54);
#endif

    setItemDelegateForColumn(6, MakeButtonItemDelegate(this));
    setItemDelegateForColumn(7, MakeButtonItemDelegate(this));

    G_CONNECTION_PTR->setValue(className(), this);
    G_CONNECTION_PTR->connect(className(), MusicSongsSummariziedWidget::className());
}

MusicSongCheckToolsQualityTableWidget::~MusicSongCheckToolsQualityTableWidget()
{
    G_CONNECTION_PTR->removeValue(this);
}

void MusicSongCheckToolsQualityTableWidget::addCellItems(const MusicSongCheckToolsQualityList &songs)
{
    setRowCount(songs.count());
    QHeaderView *headerview = horizontalHeader();

    for(int i = 0; i < songs.count(); ++i)
    {
        const MusicSongCheckToolsQuality &v = songs[i];

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(TTK_CHECKED_ROLE, Qt::Unchecked);
        setItem(i, 0, item);

                          item = new QTableWidgetItem;
        item->setToolTip(v.m_song.name());
        item->setText(MusicUtils::Widget::elidedText(font(), item->toolTip(), Qt::ElideRight, headerview->sectionSize(1) - 10));
        QtItemSetTextAlignment(item, Qt::AlignLeft | Qt::AlignVCenter);
        setItem(i, 1, item);

                item = new QTableWidgetItem;
        item->setText(v.m_song.playTime());
        QtItemSetTextAlignment(item, Qt::AlignCenter);
        setItem(i, 2, item);

                item = new QTableWidgetItem;
        item->setText(v.m_song.sizeStr());
        QtItemSetTextAlignment(item, Qt::AlignCenter);
        setItem(i, 3, item);

                item = new QTableWidgetItem;
        item->setText(v.m_bitrate);
        QtItemSetTextAlignment(item, Qt::AlignCenter);
        setItem(i, 4, item);

                item = new QTableWidgetItem;
        QColor color;
        QString bitrate;
        MusicUtils::Number::bitrateToQuality(MusicUtils::Number::bitrateToLevel(v.m_bitrate), bitrate, color);
        item->setText(bitrate);
        item->setForeground(color);
        QtItemSetTextAlignment(item, Qt::AlignCenter);
        setItem(i, 5, item);

                item = new QTableWidgetItem;
        item->setData(TTK_DISPLAY_ROLE, tr("Play"));
        item->setData(TTK_DATA_ROLE, v.m_song.path());
        setItem(i, 6, item);

                item = new QTableWidgetItem;
        item->setData(TTK_DISPLAY_ROLE, tr("Delete"));
        setItem(i, 7, item);
    }
}

void MusicSongCheckToolsQualityTableWidget::itemCellClicked(int row, int column)
{
    MusicFillItemTableWidget::itemCellClicked(row, column);
    switch(column)
    {
        case 6: musicPlay(); break;
        case 7: deleteCurrentRow(); break;
        default: break;
    }
}

void MusicSongCheckToolsQualityTableWidget::musicPlay()
{
    if(!isValid())
    {
        return;
    }

    const QTableWidgetItem *it = item(currentRow(), 6);
    if(it)
    {
        const QString &path = it->data(TTK_DATA_ROLE).toString();
        Q_EMIT addSongToPlaylist(QStringList(QFile::exists(path) ? path : QString()));
    }
}

void MusicSongCheckToolsQualityTableWidget::deleteCurrentRow()
{
    if(!isValid())
    {
        return;
    }

    removeRow(currentRow());
}
