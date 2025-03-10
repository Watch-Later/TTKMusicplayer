﻿#include "musicsongssummariziedwidget.h"
#include "musicsongslistfunctionwidget.h"
#include "musicsongslistplaytablewidget.h"
#include "musicsongsearchdialog.h"
#include "musicmessagebox.h"
#include "musicconnectionpool.h"
#include "musicprogresswidget.h"
#include "musicsongchecktoolswidget.h"
#include "musicplayedlistpopwidget.h"
#include "musiclrcdownloadbatchwidget.h"
#include "musicapplication.h"
#include "musictoastlabel.h"
#include "musicfileutils.h"
#include "musicformats.h"

#define ITEM_MIN_COUNT             4
#define ITEM_MAX_COUNT             10
#define RECENT_ITEM_MAX_COUNT      50

MusicSongsSummariziedWidget::MusicSongsSummariziedWidget(QWidget *parent)
    : MusicSongsToolBoxWidget(parent),
      MusicItemSearchInterfaceClass(),
      m_playToolIndex(MUSIC_NORMAL_LIST),
      m_lastSearchIndex(MUSIC_NORMAL_LIST),
      m_selectImportIndex(MUSIC_NORMAL_LIST),
      m_selectDeleteIndex(MUSIC_NORMAL_LIST),
      m_toolDeleteChanged(false),
      m_listFunctionWidget(nullptr),
      m_songSearchWidget(nullptr)
{
    m_listMaskWidget = new MusicSongsToolBoxMaskWidget(this);
    setInputModule(m_listMaskWidget);

    connect(m_listMaskWidget, SIGNAL(itemIndexChanged(int)), SLOT(itemIndexChanged(int)));
    connect(m_scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(sliderValueChanaged(int)));

    G_CONNECTION_PTR->setValue(className(), this);
    G_CONNECTION_PTR->connect(MusicQueryTableWidget::className(), className());
}

MusicSongsSummariziedWidget::~MusicSongsSummariziedWidget()
{
    G_CONNECTION_PTR->removeValue(this);
    delete m_listMaskWidget;
    delete m_listFunctionWidget;
    delete m_songSearchWidget;

    while(!m_containerItems.isEmpty())
    {
        delete m_containerItems.takeLast().m_itemObject;
    }
}

bool MusicSongsSummariziedWidget::addMusicItemList(const MusicSongItemList &items)
{
    TTKIntSet inDeed;
    inDeed << MUSIC_NORMAL_LIST << MUSIC_LOVEST_LIST << MUSIC_NETWORK_LIST << MUSIC_RECENT_LIST;
    for(const MusicSongItem &item : qAsConst(items))
    {
        inDeed.remove(item.m_itemIndex);
    }
    //
    if(!inDeed.isEmpty())
    {
        //if less than four count(0, 1, 2, 3), find and add default items
        m_containerItems << items;
        for(const int item : qAsConst(inDeed))
        {
            MusicSongItem songItem;
            switch(item)
            {
                case MUSIC_NORMAL_LIST:
                {
                    songItem.m_itemIndex = item;
                    songItem.m_itemName = tr("Default Item");
                    break;
                }
                case MUSIC_LOVEST_LIST:
                {
                    songItem.m_itemIndex = item;
                    songItem.m_itemName = tr("Lovest Item");
                    break;
                }
                case MUSIC_NETWORK_LIST:
                {
                    songItem.m_itemIndex = item;
                    songItem.m_itemName = tr("Net Item");
                    break;
                }
                case MUSIC_RECENT_LIST:
                {
                    songItem.m_itemIndex = item;
                    songItem.m_itemName = tr("Recent Item");
                    break;
                }
                default: break;
            }
            m_containerItems << songItem;
        }
        std::sort(m_containerItems.begin(), m_containerItems.end());
    }
    else
    {
        m_containerItems = items;
    }

    for(int i = 0; i < m_containerItems.count(); ++i)
    {
        createWidgetItem(&m_containerItems[i]);
    }

    return inDeed.isEmpty();
}

void MusicSongsSummariziedWidget::appendMusicItemList(const MusicSongItemList &items)
{
    for(int i = 0; i < items.count(); ++i)
    {
        m_containerItems << items[i];
        MusicSongItem *item = &m_containerItems.back();
        item->m_itemIndex = ++m_itemIndexRaise;
        checkCurrentNameExist(item->m_itemName);
        createWidgetItem(item);
    }
}

void MusicSongsSummariziedWidget::importMusicSongsByPath(const QStringList &files)
{
    if(files.isEmpty())
    {
        return;
    }

    closeSearchWidgetInNeed();

    MusicProgressWidget progress;
    progress.setTitle(tr("Import file mode"));
    progress.setRange(0, files.count());
    progress.show();

    MusicSongItem *item = &m_containerItems[m_selectImportIndex];

    int i = 0;
    for(const QString &path : qAsConst(files))
    {
        if(item->m_songs.contains(MusicSong(path)))
        {
            continue;
        }

        progress.setValue(++i);
        item->m_songs << MusicObject::generateSongList(path);
    }

    item->m_itemObject->updateSongsList(item->m_songs);
    setItemTitle(item);
    setCurrentIndex(m_selectImportIndex);

    MusicToastLabel::popup(tr("Import music songs done!"));
}

void MusicSongsSummariziedWidget::importMusicSongsByUrl(const QString &path)
{
    if(path.isEmpty())
    {
        return;
    }

    const QFileInfo fin(path);
    if(fin.isDir())
    {
        QStringList files;
        for(const QFileInfo &fin : MusicUtils::File::fileInfoListByPath(path))
        {
            if(MusicFormats::supportMusicFormats().contains(FILE_SUFFIX(fin)))
            {
               files << fin.absoluteFilePath();
            }
        }

        importMusicSongsByPath(files);
    }
    else if(MusicUtils::String::isNetworkUrl(path))
    {
        closeSearchWidgetInNeed();

        MusicSongItem *item = &m_containerItems[MUSIC_NETWORK_LIST];
        const QString &prefix = MusicUtils::String::stringSplitToken(path, TTK_SEPARATOR, "?");
        const QByteArray &md5 = MusicUtils::Algorithm::md5(path.toUtf8());
        const MusicSong song(path + "#" + md5 + "." + MusicUtils::String::stringSplitToken(path),
                             TTK_DEFAULT_STR, MusicUtils::String::stringPrefix(prefix));
        if(item->m_songs.contains(song))
        {
            return;
        }

        item->m_songs << song;
        item->m_itemObject->updateSongsList(item->m_songs);
        setItemTitle(item);
    }
    else
    {
        QStringList files;
        if(MusicFormats::supportMusicFormats().contains(FILE_SUFFIX(fin)))
        {
           files << fin.absoluteFilePath();
        }

        importMusicSongsByPath(files);
    }
}

QStringList MusicSongsSummariziedWidget::musicSongsFileName(int index) const
{
    QStringList list;

    if(index < 0 || index >= m_containerItems.count())
    {
        return list;
    }

    for(const MusicSong &song : qAsConst(m_containerItems[index].m_songs))
    {
        list << song.name();
    }
    return list;
}

QStringList MusicSongsSummariziedWidget::musicSongsFilePath(int index) const
{
    QStringList list;
    if(index < 0 || index >= m_containerItems.count())
    {
        return list;
    }

    for(const MusicSong &song : qAsConst(m_containerItems[index].m_songs))
    {
        list << song.path();
    }
    return list;
}

int MusicSongsSummariziedWidget::mapSongIndexByFilePath(int toolIndex, const QString &path) const
{
    if(toolIndex < 0 || toolIndex >= m_containerItems.count() || path.isEmpty())
    {
        return -1;
    }

    const MusicSongList *songs = &m_containerItems[toolIndex].m_songs;
    for(int i = 0; i < songs->count(); ++i)
    {
        if(MusicSong(path) == songs->at(i))
        {
            return i;
        }
    }
    return -1;
}

QString MusicSongsSummariziedWidget::mapFilePathBySongIndex(int toolIndex, int index) const
{
    if(toolIndex < 0 || toolIndex >= m_containerItems.count())
    {
        return QString();
    }

    const MusicSongList *songs = &m_containerItems[toolIndex].m_songs;
    if(index < 0 || index >= songs->count())
    {
        return QString();
    }

    return songs->at(index).path();
}

void MusicSongsSummariziedWidget::removeSearchResult(int &row)
{
    if(!hasSearchResult() || !isSearchPlayIndex())
    {
        return;
    }

    const TTKIntList &result = m_searchResultCache.value(m_searchResultLevel);
    if(row >= result.count() || row < 0)
    {
        row = -1;
        return;
    }

    row = result[row];

    clearSearchResult();
    closeSearchWidget();
}

void MusicSongsSummariziedWidget::setCurrentMusicSongTreeIndex(int index)
{
    const int before = m_playToolIndex;
    m_playToolIndex = index;

    if(before >= 0)
    {
        MusicSongsListPlayTableWidget *widget = TTKObject_cast(MusicSongsListPlayTableWidget*, m_containerItems[before].m_itemObject);
        if(widget && !m_containerItems[before].m_songs.isEmpty())
        {
            widget->adjustPlayWidgetRow();
        }
    }
}

void MusicSongsSummariziedWidget::playLocation(int index)
{
    if(!hasSearchResult())
    {
        selectRow(index);
        resizeScrollIndex(index * 30);
    }
}

void MusicSongsSummariziedWidget::selectRow(int index)
{
    setCurrentIndex(m_playToolIndex);
    if(m_playToolIndex < 0)
    {
        return;
    }

    closeSearchWidgetInNeed();
    m_containerItems[m_playToolIndex].m_itemObject->selectRow(index);
}

void MusicSongsSummariziedWidget::updateTimeLabel(const QString &current, const QString &total) const
{
    if(m_playToolIndex < 0)
    {
        return;
    }
    TTKObject_cast(MusicSongsListPlayTableWidget*, m_containerItems[m_playToolIndex].m_itemObject)->updateTimeLabel(current, total);
}

void MusicSongsSummariziedWidget::addNewRowItem()
{
    if(m_containerItems.count() <= ITEM_MAX_COUNT)
    {
        QString name = tr("Default Item");
        checkCurrentNameExist(name);
        addNewRowItem(name);
    }
}

void MusicSongsSummariziedWidget::deleteRowItem(int index)
{
    const int id = foundMappedIndex(index);
    if(id == -1)
    {
        return;
    }

    MusicMessageBox message;
    message.setText(tr("Are you sure to delete?"));
    if(!message.exec())
    {
        return;
    }

    MusicSongItem item = m_containerItems[id];
    for(const MusicSong &song : qAsConst(item.m_songs))
    {
        MusicPlayedListPopWidget::instance()->remove(item.m_itemIndex, song);
    }

    if(m_playToolIndex == id)
    {
        setCurrentIndex(MUSIC_NORMAL_LIST);
        m_itemList.front().m_widgetItem->setItemExpand(false);
        MusicApplication::instance()->musicPlayIndex(TTK_NORMAL_LEVEL);
    }
    else if(m_playToolIndex > id)
    {
        setCurrentIndex(--m_playToolIndex);
    }

    item = m_containerItems.takeAt(id);
    removeItem(item.m_itemObject);
    delete item.m_itemObject;

    resetToolIndex();
}

void MusicSongsSummariziedWidget::deleteRowItems()
{
    MusicMessageBox message;
    message.setText(tr("Are you sure to delete?"));
    if(!message.exec())
    {
        return;
    }

    if(m_playToolIndex != MUSIC_NORMAL_LIST && MusicObject::playlistRowValid(m_playToolIndex))
    {
        setCurrentIndex(MUSIC_NORMAL_LIST);
        m_itemList.front().m_widgetItem->setItemExpand(false);
        MusicApplication::instance()->musicPlayIndex(TTK_NORMAL_LEVEL);
    }

    for(int i = m_containerItems.count() - 1; i > 3; --i)
    {
        MusicSongItem item = m_containerItems.takeLast();
        removeItem(item.m_itemObject);
        delete item.m_itemObject;
    }
}

void MusicSongsSummariziedWidget::deleteRowItemAll(int index)
{
    const int id = foundMappedIndex(index);
    if(id == -1)
    {
        return;
    }

    closeSearchWidgetInNeed();

    m_selectDeleteIndex = id;
    m_toolDeleteChanged = true;

    MusicSongsListPlayTableWidget *widget = TTKObject_cast(MusicSongsListPlayTableWidget*, m_containerItems[id].m_itemObject);
    if(widget->rowCount() > 0)
    {
        widget->setCurrentCell(0, 1);
        widget->setDeleteItemAll();
    }
    m_toolDeleteChanged = false;

    if(m_containerItems[id].m_songs.isEmpty() && m_playToolIndex == id)
    {
        MusicApplication::instance()->musicPlayIndex(TTK_NORMAL_LEVEL);
    }
}

void MusicSongsSummariziedWidget::changRowItemName(int index, const QString &name)
{
    const int id = foundMappedIndex(index);
    if(id == -1)
    {
        return;
    }

    MusicSongItem *item = &m_containerItems[id];
    item->m_itemName = name;
    setItemTitle(item);
}

void MusicSongsSummariziedWidget::swapDragItemIndex(int before, int after)
{
    before = foundMappedIndex(before);
    after = foundMappedIndex(after);
    if(before == after)
    {
        return;
    }

    //adjust the m_playToolIndex while the item has dragged and dropped
    if(before < after)
    {
        if((before < m_playToolIndex && m_playToolIndex < after) || m_playToolIndex == after)
        {
            m_currentIndex = --m_playToolIndex;
        }
        else if(m_playToolIndex == before)
        {
            m_currentIndex = m_playToolIndex = after;
        }
    }
    else
    {
        if((after < m_playToolIndex && m_playToolIndex < before) || m_playToolIndex == after)
        {
            m_currentIndex = ++m_playToolIndex;
        }
        else if(m_playToolIndex == before)
        {
            m_currentIndex = m_playToolIndex = after;
        }
    }

    swapItem(before, after);
    const MusicSongItem &item = m_containerItems.takeAt(before);
    m_containerItems.insert(after, item);

    resetToolIndex();
}

void MusicSongsSummariziedWidget::addToPlayLater(int index)
{
    const int id = foundMappedIndex(index);
    if(id == -1)
    {
        return;
    }

    const MusicSongItem *item = &m_containerItems[id];
    const MusicSongList *songs = &item->m_songs;
    for(int i = songs->count() - 1; i >= 0; --i)
    {
        MusicPlayedListPopWidget::instance()->insert(item->m_itemIndex, songs->at(i));
    }
}

void MusicSongsSummariziedWidget::addToPlayedList(int index)
{
    const int id = foundMappedIndex(index);
    if(id == -1)
    {
        return;
    }

    const MusicSongItem *item = &m_containerItems[id];
    for(const MusicSong &song : qAsConst(item->m_songs))
    {
        MusicPlayedListPopWidget::instance()->append(item->m_itemIndex, song);
    }
}

void MusicSongsSummariziedWidget::musicImportSongsByFiles(int index)
{
    if(index == TTK_LOW_LEVEL)
    {
        m_selectImportIndex = m_currentIndex;
    }
    else
    {
        const int id = foundMappedIndex(index);
        if(id == -1)
        {
            return;
        }

        m_selectImportIndex = id;
    }

    MusicApplication::instance()->musicImportSongsByFiles();
    m_selectImportIndex = MUSIC_NORMAL_LIST;
}

void MusicSongsSummariziedWidget::musicImportSongsByDir(int index)
{
    if(index == TTK_LOW_LEVEL)
    {
        m_selectImportIndex = m_currentIndex;
    }
    else
    {
        const int id = foundMappedIndex(index);
        if(id == -1)
        {
            return;
        }

        m_selectImportIndex = id;
    }

    MusicApplication::instance()->musicImportSongsByDir();
    m_selectImportIndex = MUSIC_NORMAL_LIST;
}

void MusicSongsSummariziedWidget::musicSongsCheckTestTools()
{
    GENERATE_SINGLE_WIDGET_CLASS(MusicSongCheckToolsWidget);
}

void MusicSongsSummariziedWidget::musicLrcBatchDownload()
{
    GENERATE_SINGLE_WIDGET_CLASS(MusicLrcDownloadBatchWidget);
}

void MusicSongsSummariziedWidget::searchResultChanged(int, int column)
{
    if(m_currentIndex == -1)
    {
        return;
    }

    if(!isSearchPlayIndex())
    {
        const QStringList searchedSongs(musicSongsFileName(m_lastSearchIndex));
        TTKIntList result;
        for(int i = 0; i < searchedSongs.count(); ++i)
        {
            result << i;
        }

        MusicSongItem *item = &m_containerItems[m_lastSearchIndex];
        TTKObject_cast(MusicSongsListPlayTableWidget*, item->m_itemObject)->updateSearchFileName(&item->m_songs, result);

        if(item->m_songs.isEmpty())
        {
            item->m_itemObject->updateSongsList(item->m_songs);
        }

        clearSearchResult();
        m_lastSearchIndex = m_currentIndex;
    }

    const QString &text = m_songSearchWidget->text();
    const QStringList searchedSongs(musicSongsFileName(m_currentIndex));
    TTKIntList result;
    for(int i = 0; i < searchedSongs.count(); ++i)
    {
        if(searchedSongs[i].contains(text, Qt::CaseInsensitive))
        {
            result << i;
        }
    }

    m_searchResultLevel = column;
    m_searchResultCache.insert(column, result);

    MusicSongItem *item = &m_containerItems[m_currentIndex];
    TTKObject_cast(MusicSongsListPlayTableWidget*, item->m_itemObject)->updateSearchFileName(&item->m_songs, result);

    if(column == 0)
    {
        if(item->m_songs.isEmpty())
        {
            item->m_itemObject->updateSongsList(item->m_songs);
        }

        clearSearchResult();
    }
}

void MusicSongsSummariziedWidget::updateCurrentIndex()
{
    const QStringList &lastPlayIndex = G_SETTING_PTR->value(MusicSettingManager::LastPlayIndex).toStringList();
    if(lastPlayIndex.count() != 3)
    {
        return;
    }

    m_playToolIndex = lastPlayIndex[1].toInt();
    const int index = lastPlayIndex[2].toInt();
    setCurrentIndex(index);
    setMusicPlayCount(index);

    MusicApplication::instance()->showCurrentSong();
}

void MusicSongsSummariziedWidget::addSongToLovestListAt(bool state, int row)
{
    if(m_currentIndex < 0 || m_currentIndex >= m_containerItems.count() || hasSearchResult())
    {
        return;
    }

    const MusicSong &song = m_containerItems[m_currentIndex].m_songs[row];
    MusicSongItem *item = &m_containerItems[MUSIC_LOVEST_LIST];

    ///if current play list contains, call main add and remove function
    if(MusicSong(MusicApplication::instance()->currentFilePath()) == song)
    {
        MusicApplication::instance()->musicAddSongToLovestList(state);
        return;
    }

    MusicSongsListPlayTableWidget *widget = TTKObject_cast(MusicSongsListPlayTableWidget*, item->m_itemObject);
    if(state)    ///Add to lovest list
    {
        item->m_songs << song;
        widget->updateSongsList(item->m_songs);
        setItemTitle(item);
    }
    else        ///Remove to lovest list
    {
        if(item->m_songs.removeOne(song))
        {
            widget->removeItems();
            widget->updateSongsList(item->m_songs);
            setItemTitle(item);
            MusicApplication::instance()->setLoveDeleteItemAt(song.path(), m_playToolIndex == MUSIC_LOVEST_LIST);
        }
    }
}

void MusicSongsSummariziedWidget::musicSongToLovestListAt(bool state, int row)
{
    if(m_playToolIndex < 0 || m_playToolIndex >= m_containerItems.count())
    {
        return;
    }

    const MusicSong &song = m_containerItems[m_playToolIndex].m_songs[row];
    MusicSongItem *item = &m_containerItems[MUSIC_LOVEST_LIST];
    MusicSongsListPlayTableWidget *widget = TTKObject_cast(MusicSongsListPlayTableWidget*, item->m_itemObject);
    if(state)    ///Add to lovest list
    {
        item->m_songs << song;
        widget->updateSongsList(item->m_songs);
        setItemTitle(item);
    }
    else        ///Remove to lovest list
    {
        if(item->m_songs.removeOne(song))
        {
            widget->removeItems();
            widget->updateSongsList(item->m_songs);
            setItemTitle(item);
            MusicApplication::instance()->setLoveDeleteItemAt(song.path(), m_playToolIndex == MUSIC_LOVEST_LIST);
        }
    }
}

void MusicSongsSummariziedWidget::addSongBufferToPlaylist(const MusicResultDataItem &songItem)
{
    MusicSongItem *item = &m_containerItems[MUSIC_NETWORK_LIST];
    const QByteArray &md5 = MusicUtils::Algorithm::md5(songItem.m_id.toUtf8());
    MusicSong song(songItem.m_nickName + "#" + md5 + "." + songItem.m_description, songItem.m_updateTime, songItem.m_name);
    song.setFormat(songItem.m_description);
    song.setSizeStr(songItem.m_playCount);

    int index = item->m_songs.indexOf(song);
    if(index == -1)
    {
        item->m_songs << song;
        item->m_itemObject->updateSongsList(item->m_songs);
        setItemTitle(item);
        index = item->m_songs.count() - 1;
    }

    if(songItem.m_tags == MUSIC_PLAY_NOW)
    {
        ///when download finished just play it at once
        setCurrentIndex(MUSIC_NETWORK_LIST);
        MusicApplication::instance()->musicPlayIndexClicked(index, 0);
    }
}

void MusicSongsSummariziedWidget::addSongToPlaylist(const QStringList &items)
{
    if(items.isEmpty())
    {
        return;
    }

    QStringList files(items);
    importMusicSongsByPath(files);

    const MusicSongItem *item = &m_containerItems[MUSIC_NORMAL_LIST];
    const MusicSongList *musicSongs = &item->m_songs;
    const MusicSong &song = MusicSong(items.back());

    int index = musicSongs->count() - 1;
    if(musicSongs->contains(song))
    {
        index = musicSongs->indexOf(song);
    }

    /// just play it at once
    setCurrentIndex(MUSIC_NORMAL_LIST);
    MusicApplication::instance()->musicPlayIndexClicked(index, 0);
}

void MusicSongsSummariziedWidget::setDeleteItemAt(const TTKIntList &index, bool fileRemove)
{
    if(index.isEmpty() || hasSearchResult())
    {
        return;
    }

    const int currentIndex = m_toolDeleteChanged ? m_selectDeleteIndex : m_currentIndex;
    MusicSongItem *item = &m_containerItems[currentIndex];
    QStringList deleteFiles;
    for(int i = index.count() - 1; i >= 0; --i)
    {
        const MusicSong &song = item->m_songs.takeAt(index[i]);
        deleteFiles << song.path();
        if(currentIndex != m_playToolIndex && currentIndex == MUSIC_LOVEST_LIST)
        {
            const int playIndex = m_containerItems[m_playToolIndex].m_itemObject->playRowIndex();
            const MusicSongList &songs = m_containerItems[m_playToolIndex].m_songs;
            if(playIndex > -1 && playIndex < songs.count())
            {
                if(songs[playIndex] == song)
                {
                    MusicApplication::instance()->musicAddSongToLovestList(false);
                }
            }
        }

        if(fileRemove)
        {
            QFile::remove(currentIndex == MUSIC_NETWORK_LIST ? MusicObject::generateNetworkSongPath(song.path()) : song.path());
        }
    }

    MusicApplication::instance()->setDeleteItemAt(deleteFiles, fileRemove, currentIndex == m_playToolIndex, currentIndex);

    setItemTitle(item);
    //create upload file widget if current items is all been deleted
    TTKObject_cast(MusicSongsListPlayTableWidget*, item->m_itemObject)->createUploadFileModule();
}

void MusicSongsSummariziedWidget::setMusicIndexSwaped(int before, int after, int play, MusicSongList &songs)
{
    MusicSongList *names = &m_containerItems[m_currentIndex].m_songs;
    if(before > after)
    {
        for(int i = before; i > after; --i)
        {
            QtContainerSwap(names, i, i - 1);
        }
    }
    else
    {
        for(int i = before; i < after; ++i)
        {
            QtContainerSwap(names, i, i + 1);
        }
    }
    songs = *names;

    if(m_currentIndex == m_playToolIndex)
    {
        MusicPlayedListPopWidget::instance()->setCurrentIndex(m_currentIndex, songs[play]);
    }
}

void MusicSongsSummariziedWidget::isCurrentIndex(bool &state)
{
    const int cIndex = m_toolDeleteChanged ? m_selectDeleteIndex : m_currentIndex;
    state = (cIndex == m_playToolIndex);
}

void MusicSongsSummariziedWidget::isSearchResultEmpty(bool &empty)
{
    empty = !hasSearchResult();
}

void MusicSongsSummariziedWidget::setMusicPlayCount(int index)
{
    if(index < 0 || m_playToolIndex < 0)
    {
        return;
    }

    MusicSongList *songs = &m_containerItems[m_playToolIndex].m_songs;
    if(!songs->isEmpty() && index < songs->count())
    {
        MusicSong *song = &(*songs)[index];
        song->setPlayCount(song->playCount() + 1);
    }
}

void MusicSongsSummariziedWidget::setRecentMusicSongs(int index)
{
    if(index < 0 || m_playToolIndex < 0 || m_playToolIndex == MUSIC_NETWORK_LIST || m_playToolIndex == MUSIC_RECENT_LIST)
    {
        return;
    }

    MusicSongList *songs = &m_containerItems[m_playToolIndex].m_songs;
    if(songs->isEmpty() || index >= songs->count())
    {
        return;
    }

    MusicSongItem *item = &m_containerItems[MUSIC_RECENT_LIST];
    MusicSong recentSong(songs->at(index));
    MusicSongList *recentSongs = &item->m_songs;
    MusicSongsListPlayTableWidget *widget = TTKObject_cast(MusicSongsListPlayTableWidget*, item->m_itemObject);
    if(!recentSongs->contains(recentSong))
    {
        if(recentSongs->count() >= RECENT_ITEM_MAX_COUNT)
        {
            recentSongs->takeFirst();
            widget->removeItems();
        }

        recentSong.setPlayCount(recentSong.playCount() + 1);
        recentSongs->append(recentSong);
        widget->updateSongsList(*recentSongs);

        const QString title(QString("%1[%2]").arg(item->m_itemName).arg(recentSongs->count()));
        setTitle(widget, title);
    }
    else
    {
        for(int i = 0; i < recentSongs->count(); ++i)
        {
            MusicSong *song = &(*recentSongs)[i];
            if(recentSong == *song)
            {
                song->setPlayCount(song->playCount() + 1);
                break;
            }
        }
    }
}

void MusicSongsSummariziedWidget::queryMusicItemList(MusicSongItemList &songs)
{
    songs = m_containerItems;
}

void MusicSongsSummariziedWidget::updateCurrentArtist()
{
    if(m_playToolIndex < 0)
    {
        return;
    }
    TTKObject_cast(MusicSongsListPlayTableWidget*, m_containerItems[m_playToolIndex].m_itemObject)->updateCurrentArtist();
}

void MusicSongsSummariziedWidget::showFloatWidget()
{
    if(m_listFunctionWidget == nullptr)
    {
        m_listFunctionWidget = new MusicSongsListFunctionWidget(this);
        connect(m_listFunctionWidget, SIGNAL(deleteObject()), SLOT(deleteFloatWidget()));
        resizeWindow();
        m_listFunctionWidget->show();
    }
    else
    {
        resizeWindow();
        m_listFunctionWidget->active();
    }
}

void MusicSongsSummariziedWidget::musicListSongSortBy(int index)
{
    const int id = foundMappedIndex(index);
    if(id == -1)
    {
        return;
    }

    closeSearchWidgetInNeed();

    MusicSongsListPlayTableWidget *widget = TTKObject_cast(MusicSongsListPlayTableWidget*, m_containerItems[id].m_itemObject);
    MusicSong::Sort sort = MusicSong::Sort::ByFileName;
    index = m_containerItems[id].m_sort.m_type;
    if(index != -1)
    {
        sort = TTKStatic_cast(MusicSong::Sort, index);
    }

    MusicSongList *songs = &m_containerItems[id].m_songs;
    const MusicSong song(MusicApplication::instance()->currentFilePath());

    for(int i = 0; i < songs->count(); ++i)
    {
        (*songs)[i].setSort(sort);
    }

    if(m_containerItems[id].m_sort.m_order == Qt::DescendingOrder)
    {
        std::sort(songs->begin(), songs->end());
    }
    else
    {
        std::sort(songs->begin(), songs->end(), std::greater<MusicSong>());
    }

    widget->removeItems();
    widget->setSongsList(songs);

    index = songs->indexOf(song);
    if(m_currentIndex == m_playToolIndex)
    {
        MusicApplication::instance()->musicPlaySort(index);
    }
}

void MusicSongsSummariziedWidget::showSearchWidget()
{
    if(m_songSearchWidget == nullptr)
    {
        m_songSearchWidget = new MusicSongSearchDialog(this);
        resizeWindow();
    }

    m_songSearchWidget->setVisible(!m_songSearchWidget->isVisible());
}

void MusicSongsSummariziedWidget::sliderValueChanaged(int value)
{
    if(value >= 40 * (m_currentIndex + 1) && m_currentIndex > -1 && m_currentIndex < m_containerItems.count())
    {
        MusicSongItem *item = &m_containerItems[m_currentIndex];
        m_listMaskWidget->setItemIndex(item->m_itemIndex);
        m_listMaskWidget->setSongSort(&item->m_sort);
        m_listMaskWidget->setTitle(QString("%1[%2]").arg(item->m_itemName).arg(item->m_songs.count()));
        m_listMaskWidget->setItemExpand(true);
        m_listMaskWidget->raise();
        m_listMaskWidget->show();
    }
    else
    {
        m_listMaskWidget->hide();
    }
}

void MusicSongsSummariziedWidget::deleteFloatWidget()
{
    delete m_listFunctionWidget;
    m_listFunctionWidget = nullptr;
}

void MusicSongsSummariziedWidget::resizeEvent(QResizeEvent *event)
{
    MusicSongsToolBoxWidget::resizeEvent(event);
    resizeWindow();
}

void MusicSongsSummariziedWidget::contextMenuEvent(QContextMenuEvent *event)
{
    MusicSongsToolBoxWidget::contextMenuEvent(event);

    QMenu menu(this);
    menu.setStyleSheet(MusicUIObject::MQSSMenuStyle02);
    menu.addAction(tr("Create Item"), this, SLOT(addNewRowItem()));
    menu.addAction(tr("Import Item"), MusicApplication::instance(), SLOT(musicImportSongsItemList()));
    menu.addAction(tr("Music Test Tools"), this, SLOT(musicSongsCheckTestTools()));
    menu.addAction(tr("Lrc Batch Download"), this, SLOT(musicLrcBatchDownload()));
    menu.addAction(tr("Delete All"), this, SLOT(deleteRowItems()))->setEnabled(m_containerItems.count() > ITEM_MIN_COUNT);

    MusicUtils::Widget::adjustMenuPosition(&menu);
    menu.exec(QCursor::pos());
}

void MusicSongsSummariziedWidget::closeSearchWidget()
{
    if(m_songSearchWidget)
    {
        m_songSearchWidget->close();
    }
}

void MusicSongsSummariziedWidget::closeSearchWidgetInNeed()
{
    if(hasSearchResult())
    {
        closeSearchWidget();
    }
}

void MusicSongsSummariziedWidget::checkCurrentNameExist(QString &name)
{
    QString check = name;
    for(int i = 1; i <= ITEM_MAX_COUNT; ++i)
    {
        bool hasName = false;
        for(const MusicSongItem &songItem : qAsConst(m_containerItems))
        {
            if(check == songItem.m_itemName)
            {
                hasName = true;
                check = name + QString::number(i);
                break;
            }
        }

        if(!hasName)
        {
            name = check;
            break;
        }
    }
}

void MusicSongsSummariziedWidget::addNewRowItem(const QString &name)
{
    MusicSongItem item;
    item.m_itemName = name;
    m_containerItems << item;
    createWidgetItem(&m_containerItems.back());
}

void MusicSongsSummariziedWidget::createWidgetItem(MusicSongItem *item)
{
    MusicSongsListPlayTableWidget *object = new MusicSongsListPlayTableWidget(TTK_NORMAL_LEVEL, this);
    object->setMovedScrollBar(m_scrollArea->verticalScrollBar());
    object->setSongSort(&item->m_sort);

    item->m_itemObject = object;
    item->m_itemIndex = m_itemIndexRaise;

    addCellItem(object, item->m_itemName);
    setSongSort(object, &item->m_sort);
    object->setToolIndex(foundMappedIndex(item->m_itemIndex));

    connect(object, SIGNAL(isCurrentIndex(bool&)), SLOT(isCurrentIndex(bool&)));
    connect(object, SIGNAL(isSearchResultEmpty(bool&)), SLOT(isSearchResultEmpty(bool&)));
    connect(object, SIGNAL(deleteItemAt(TTKIntList,bool)), SLOT(setDeleteItemAt(TTKIntList,bool)));
    connect(object, SIGNAL(queryMusicIndexSwaped(int,int,int,MusicSongList&)), SLOT(setMusicIndexSwaped(int,int,int,MusicSongList&)));
    connect(object, SIGNAL(addSongToLovestListAt(bool,int)), SLOT(addSongToLovestListAt(bool,int)));
    connect(object, SIGNAL(showFloatWidget()), SLOT(showFloatWidget()));
    connect(object, SIGNAL(musicListSongSortBy(int)), SLOT(musicListSongSortBy(int)));

    ///connect to items
    setInputModule(m_itemList.back().m_widgetItem);

    object->setSongsList(&item->m_songs);
    setTitle(object, QString("%1[%2]").arg(item->m_itemName).arg(item->m_songs.count()));
}

void MusicSongsSummariziedWidget::setItemTitle(MusicSongItem *item)
{
    const QString title(QString("%1[%2]").arg(item->m_itemName).arg(item->m_songs.count()));
    setTitle(item->m_itemObject, title);

    if(m_listMaskWidget->isVisible() && m_listMaskWidget->itemIndex() == item->m_itemIndex)
    {
        m_listMaskWidget->setTitle(title);
    }
}

void MusicSongsSummariziedWidget::setInputModule(QObject *object) const
{
    connect(object, SIGNAL(addNewRowItem()), SLOT(addNewRowItem()));
    connect(object, SIGNAL(deleteRowItemAll(int)), SLOT(deleteRowItemAll(int)));
    connect(object, SIGNAL(deleteRowItem(int)), SLOT(deleteRowItem(int)));
    connect(object, SIGNAL(changRowItemName(int,QString)), SLOT(changRowItemName(int,QString)));
    connect(object, SIGNAL(musicAddNewFiles(int)), SLOT(musicImportSongsByFiles(int)));
    connect(object, SIGNAL(musicAddNewDir(int)), SLOT(musicImportSongsByDir(int)));
    connect(object, SIGNAL(musicListSongSortBy(int)), SLOT(musicListSongSortBy(int)));
    connect(object, SIGNAL(swapDragItemIndex(int,int)), SLOT(swapDragItemIndex(int,int)));
    connect(object, SIGNAL(addToPlayLater(int)), SLOT(addToPlayLater(int)));
    connect(object, SIGNAL(addToPlayedList(int)), SLOT(addToPlayedList(int)));
}

void MusicSongsSummariziedWidget::resizeWindow()
{
    if(m_listFunctionWidget)
    {
        m_listFunctionWidget->move(width() - m_listFunctionWidget->width() - 15, height() - 40 - m_listFunctionWidget->height());
    }

    if(m_songSearchWidget)
    {
        m_songSearchWidget->move(0, height() - m_songSearchWidget->height());
    }
}

void MusicSongsSummariziedWidget::resetToolIndex()
{
    PlayedItemList pairs;
    for(const MusicSongItem &item : qAsConst(m_containerItems))
    {
        const int mappedIndex = foundMappedIndex(item.m_itemIndex);
        item.m_itemObject->setToolIndex(mappedIndex);
        if(item.m_itemIndex != mappedIndex)
        {
            pairs << MakePlayedItem(item.m_itemIndex, mappedIndex);
        }
    }
    MusicPlayedListPopWidget::instance()->resetToolIndex(pairs);
}
