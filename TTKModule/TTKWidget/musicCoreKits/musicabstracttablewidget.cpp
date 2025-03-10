#include "musicabstracttablewidget.h"

MusicAbstractTableWidget::MusicAbstractTableWidget(QWidget *parent)
    : QTableWidget(parent),
      m_previousColorRow(-1),
      m_previousClickRow(-1),
      m_backgroundColor(255, 255, 255, 0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setColumnCount(3);
    setRowCount(0);

    QHeaderView *headerview = horizontalHeader();
    headerview->setMinimumSectionSize(0);
    headerview->setVisible(false);
    headerview->resizeSection(0, 20);
    headerview->resizeSection(1, 247);
    headerview->resizeSection(2, 45);
    verticalHeader()->setVisible(false);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setMouseTracking(true);  //Open the capture mouse function
    setStyleSheet(MusicUIObject::MQSSTableWidgetStyle01 + MusicUIObject::MQSSScrollBarStyle01 + MusicUIObject::MQSSLineEditStyle01);

    QFont font = this->font();
    font.setBold(false);
    setFont(font);

    setShowGrid(false);//Does not display the grid
    setFrameShape(QFrame::NoFrame);//Set No Border
    setEditTriggers(QAbstractItemView::NoEditTriggers);//No edit
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setFocusPolicy(Qt::NoFocus);

    MusicUtils::Widget::setTransparent(this, 50);
#if defined Q_OS_UNIX && !TTK_QT_VERSION_CHECK(5,7,0) //Fix linux selection-background-color stylesheet bug
    MusicUtils::Widget::setTransparent(this, QColor(20, 20, 20, 10));
#endif
    connect(this, SIGNAL(cellEntered(int,int)), SLOT(itemCellEntered(int,int)));
    connect(this, SIGNAL(cellClicked(int,int)), SLOT(itemCellClicked(int,int)));
}

MusicAbstractTableWidget::~MusicAbstractTableWidget()
{

}

TTKIntList MusicAbstractTableWidget::multiSelectedIndexList() const
{
    TTKIntSet rows;
    for(const QModelIndex& index : selectedIndexes())
    {
        rows.insert(index.row());
    }

    TTKIntList indexs = rows.values();
    std::sort(indexs.begin(), indexs.end());
    return indexs;
}

void MusicAbstractTableWidget::itemCellEntered(int row, int column)
{
    if(item(m_previousColorRow, 0))
    {
       setRowColor(m_previousColorRow, m_backgroundColor);
    }

    if(item(row, column))
    {
       setRowColor(row, QColor(20, 20, 20, 20));
    }

    m_previousColorRow = row;
}

void MusicAbstractTableWidget::itemCellClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}

void MusicAbstractTableWidget::removeItems()
{
    clearContents();
    setRowCount(0);

    m_previousColorRow = -1;
    m_previousClickRow = -1;
    m_backgroundColor = Qt::transparent;
}

void MusicAbstractTableWidget::setRowColor(int row, const QColor &color) const
{
    for(int i = 0; i < columnCount(); ++i)
    {
        QTableWidgetItem *it = item(row, i);
        if(it)
        {
            it->setBackground(color);
        }
    }
}
