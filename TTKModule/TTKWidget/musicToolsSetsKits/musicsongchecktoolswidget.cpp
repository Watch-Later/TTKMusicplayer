#include "musicsongchecktoolswidget.h"
#include "ui_musicsongchecktoolswidget.h"
#include "musictoolsetsuiobject.h"
#include "musicsongchecktoolsthread.h"
#include "musictoastlabel.h"

MusicSongCheckToolsWidget::MusicSongCheckToolsWidget(QWidget *parent)
    : MusicAbstractMoveWidget(parent),
      m_ui(new Ui::MusicSongCheckToolsWidget)
{
    Q_UNUSED(qRegisterMetaType<MusicSongCheckToolsRenameList>("MusicSongCheckToolsRenameList"));
    Q_UNUSED(qRegisterMetaType<MusicSongCheckToolsDuplicateList>("MusicSongCheckToolsDuplicateList"));
    Q_UNUSED(qRegisterMetaType<MusicSongCheckToolsQualityList>("MusicSongCheckToolsQualityList"));

    m_ui->setupUi(this);
    setFixedSize(size());
    setAttribute(Qt::WA_DeleteOnClose);
    setBackgroundLabel(m_ui->background);

    m_ui->topTitleCloseButton->setIcon(QIcon(":/functions/btn_close_hover"));
    m_ui->topTitleCloseButton->setStyleSheet(MusicUIObject::MQSSToolButtonStyle04);
    m_ui->topTitleCloseButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->topTitleCloseButton->setToolTip(tr("Close"));
    connect(m_ui->topTitleCloseButton, SIGNAL(clicked()), SLOT(close()));

    initRenameWidget();
    initQualityWidget();
    initDuplicateWidget();

    switchToSelectedItemStyle(0);
}

MusicSongCheckToolsWidget::~MusicSongCheckToolsWidget()
{
    G_SINGLE_MANAGER_PTR->removeObject(className());
    delete m_renameThread;
    delete m_duplicateThread;
    delete m_qualityThread;
    delete m_ui;
}

void MusicSongCheckToolsWidget::renameButtonClicked()
{
    switchToSelectedItemStyle(0);
}

void MusicSongCheckToolsWidget::renameButtonCheckClicked()
{
    if(m_ui->renameCheckButton->text() == tr("Start"))
    {
        renameReCheckButtonClicked();
    }
    else if(m_ui->renameCheckButton->text() == tr("Stop"))
    {
        m_ui->renameLoadingLabel->stop();
        m_ui->renameLoadingLabel->hide();
        m_ui->renameCheckButton->setText(tr("Start"));

        m_renameThread->setMode(MusicObject::Mode::Check);
        m_renameThread->stopAndQuitThread();
    }
    else if(m_ui->renameCheckButton->text() == tr("Apply"))
    {
        m_ui->renameLoadingLabel->stop();
        m_ui->renameLoadingLabel->hide();
        m_ui->renameReCheckButton->show();

        m_renameThread->setItemList(m_ui->renameTableWidget->checkedIndexList());
        m_renameThread->setMode(MusicObject::Mode::Apply);
        m_renameThread->stopAndQuitThread();
        m_renameThread->start();
    }
}

void MusicSongCheckToolsWidget::renameReCheckButtonClicked()
{
    m_ui->renameReCheckButton->hide();
    m_ui->renameLoadingLabel->start();
    m_ui->renameLoadingLabel->show();
    m_ui->renameCheckButton->setText(tr("Stop"));
    m_ui->renameSelectAllButton->setChecked(false);

    m_ui->renameTableWidget->removeItems();
    m_renameThread->stopAndQuitThread();
    m_localSongs = m_ui->selectedAreaWidget->selectedSongItems();

    m_renameThread->setMode(MusicObject::Mode::Check);
    m_renameThread->setRenameSongs(&m_localSongs);
    m_renameThread->start();
}

void MusicSongCheckToolsWidget::renameCheckFinished(const MusicSongCheckToolsRenameList &items)
{
    if(m_renameThread->mode() == MusicObject::Mode::Check || items.isEmpty())
    {
        m_ui->renameLoadingLabel->stop();
        m_ui->renameLoadingLabel->hide();
        m_ui->renameCheckButton->setText(tr("Apply"));
        m_ui->renameReCheckButton->show();
        m_ui->renameSelectAllButton->setEnabled(!items.isEmpty());

        m_ui->renameTableWidget->removeItems();
        m_ui->renameTableWidget->addCellItems(items);
    }
    else if(m_renameThread->mode() == MusicObject::Mode::Apply && !m_ui->renameTableWidget->checkedIndexList().isEmpty())
    {
        MusicToastLabel::popup(tr("Rename apply finished"));
    }
}

void MusicSongCheckToolsWidget::qualityButtonClicked()
{
    switchToSelectedItemStyle(2);
}

void MusicSongCheckToolsWidget::qualityButtonCheckClicked()
{
    if(m_ui->qualityCheckButton->text() == tr("Start"))
    {
        qualityReCheckButtonClicked();
    }
    else if(m_ui->qualityCheckButton->text() == tr("Stop"))
    {
        m_ui->qualityLoadingLabel->stop();
        m_ui->qualityLoadingLabel->hide();
        m_ui->qualityCheckButton->setText(tr("Start"));
        m_qualityThread->stopAndQuitThread();
    }
    else if(m_ui->qualityCheckButton->text() == tr("Apply"))
    {
        m_ui->qualityLoadingLabel->stop();
        m_ui->qualityLoadingLabel->hide();
        m_ui->qualityReCheckButton->show();
    }
}

void MusicSongCheckToolsWidget::qualityReCheckButtonClicked()
{
    m_ui->qualityReCheckButton->hide();
    m_ui->qualityLoadingLabel->start();
    m_ui->qualityLoadingLabel->show();
    m_ui->qualityCheckButton->setText(tr("Stop"));

    m_qualityThread->stopAndQuitThread();
    m_localSongs = m_ui->selectedAreaWidget->selectedSongItems();

    m_qualityThread->setQualitySongs(&m_localSongs);
    m_qualityThread->start();
}

void MusicSongCheckToolsWidget::qualityCheckFinished(const MusicSongCheckToolsQualityList &items)
{
    m_ui->qualityLoadingLabel->stop();
    m_ui->qualityLoadingLabel->hide();
    m_ui->qualityCheckButton->setText(tr("Apply"));
    m_ui->qualityReCheckButton->show();

    m_ui->qualityTableWidget->removeItems();
    m_ui->qualityTableWidget->addCellItems(items);
}

void MusicSongCheckToolsWidget::duplicateButtonClicked()
{
    switchToSelectedItemStyle(1);
}

void MusicSongCheckToolsWidget::duplicateButtonCheckClicked()
{
    if(m_ui->duplicateCheckButton->text() == tr("Start"))
    {
        duplicateReCheckButtonClicked();
    }
    else if(m_ui->duplicateCheckButton->text() == tr("Stop"))
    {
        m_ui->duplicateLoadingLabel->stop();
        m_ui->duplicateLoadingLabel->hide();
        m_ui->duplicateCheckButton->setText(tr("Start"));

        m_duplicateThread->setMode(MusicObject::Mode::Check);
        m_duplicateThread->stopAndQuitThread();
    }
    else if(m_ui->duplicateCheckButton->text() == tr("Apply"))
    {
        m_ui->duplicateLoadingLabel->stop();
        m_ui->duplicateLoadingLabel->hide();
        m_ui->duplicateReCheckButton->show();

        m_duplicateThread->setItemList(m_ui->duplicateTableWidget->checkedIndexList());
        m_duplicateThread->setMode(MusicObject::Mode::Apply);
        m_duplicateThread->stopAndQuitThread();
        m_duplicateThread->start();
    }
}

void MusicSongCheckToolsWidget::duplicateReCheckButtonClicked()
{
    m_ui->duplicateReCheckButton->hide();
    m_ui->duplicateLoadingLabel->start();
    m_ui->duplicateLoadingLabel->show();
    m_ui->duplicateCheckButton->setText(tr("Stop"));
    m_ui->duplicateSelectAllButton->setChecked(false);

    m_qualityThread->stopAndQuitThread();
    m_localSongs = m_ui->selectedAreaWidget->selectedSongItems();

    m_duplicateThread->setMode(MusicObject::Mode::Check);
    m_duplicateThread->setDuplicateSongs(&m_localSongs);
    m_duplicateThread->start();
}

void MusicSongCheckToolsWidget::duplicateCheckFinished(const MusicSongCheckToolsDuplicateList &items)
{
    if(m_duplicateThread->mode() == MusicObject::Mode::Check || items.isEmpty())
    {
        m_ui->duplicateLoadingLabel->stop();
        m_ui->duplicateLoadingLabel->hide();
        m_ui->duplicateCheckButton->setText(tr("Apply"));
        m_ui->duplicateReCheckButton->show();
        m_ui->duplicateSelectAllButton->setEnabled(!items.isEmpty());

        m_ui->duplicateTableWidget->removeItems();
        m_ui->duplicateTableWidget->addCellItems(items);
    }
    else if(m_duplicateThread->mode() == MusicObject::Mode::Apply && !m_ui->duplicateTableWidget->checkedIndexList().isEmpty())
    {
        MusicToastLabel::popup(tr("Duplicate apply finished"));
    }
}

void MusicSongCheckToolsWidget::initRenameWidget()
{
    m_ui->renameSelectAllButton->setStyleSheet(MusicUIObject::MQSSCheckBoxStyle01);
    m_ui->renameCheckButton->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);

    connect(m_ui->renameButton, SIGNAL(clicked()), SLOT(renameButtonClicked()));
    connect(m_ui->renameCheckButton, SIGNAL(clicked()), SLOT(renameButtonCheckClicked()));
    connect(m_ui->renameReCheckButton, SIGNAL(clicked()), SLOT(renameReCheckButtonClicked()));
    connect(m_ui->renameSelectAllButton, SIGNAL(clicked(bool)), m_ui->renameTableWidget, SLOT(checkedItemsStatus(bool)));

#ifdef Q_OS_UNIX
    m_ui->renameCheckButton->setFocusPolicy(Qt::NoFocus);
    m_ui->renameSelectAllButton->setFocusPolicy(Qt::NoFocus);
#endif

    m_ui->renameSelectAllButton->setEnabled(false);
    m_ui->renameLoadingLabel->setType(MusicGifLabelWidget::Module::CicleBlue);
    m_ui->renameLoadingLabel->hide();
    m_ui->renameReCheckButton->hide();

    m_renameThread = new MusicSongCheckToolsRenameThread(this);
    connect(m_renameThread, SIGNAL(finished(MusicSongCheckToolsRenameList)), SLOT(renameCheckFinished(MusicSongCheckToolsRenameList)));
}

void MusicSongCheckToolsWidget::initQualityWidget()
{
    m_ui->qualityCheckButton->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);

    connect(m_ui->qualityButton, SIGNAL(clicked()), SLOT(qualityButtonClicked()));
    connect(m_ui->qualityCheckButton, SIGNAL(clicked()), SLOT(qualityButtonCheckClicked()));
    connect(m_ui->qualityReCheckButton, SIGNAL(clicked()), SLOT(qualityReCheckButtonClicked()));

#ifdef Q_OS_UNIX
    m_ui->qualityCheckButton->setFocusPolicy(Qt::NoFocus);
#endif

    m_ui->qualityLoadingLabel->setType(MusicGifLabelWidget::Module::CicleBlue);
    m_ui->qualityLoadingLabel->hide();
    m_ui->qualityReCheckButton->hide();

    m_qualityThread = new MusicSongCheckToolsQualityThread(this);
    connect(m_qualityThread, SIGNAL(finished(MusicSongCheckToolsQualityList)), SLOT(qualityCheckFinished(MusicSongCheckToolsQualityList)));
}

void MusicSongCheckToolsWidget::initDuplicateWidget()
{
    m_ui->duplicateSelectAllButton->setStyleSheet(MusicUIObject::MQSSCheckBoxStyle01);
    m_ui->duplicateCheckButton->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);

    connect(m_ui->duplicateButton, SIGNAL(clicked()), SLOT(duplicateButtonClicked()));
    connect(m_ui->duplicateCheckButton, SIGNAL(clicked()), SLOT(duplicateButtonCheckClicked()));
    connect(m_ui->duplicateReCheckButton, SIGNAL(clicked()), SLOT(duplicateReCheckButtonClicked()));
    connect(m_ui->duplicateSelectAllButton, SIGNAL(clicked(bool)), m_ui->duplicateTableWidget, SLOT(checkedItemsStatus(bool)));

#ifdef Q_OS_UNIX
    m_ui->duplicateSelectAllButton->setFocusPolicy(Qt::NoFocus);
    m_ui->duplicateCheckButton->setFocusPolicy(Qt::NoFocus);
#endif

    m_ui->duplicateSelectAllButton->setEnabled(false);
    m_ui->duplicateLoadingLabel->setType(MusicGifLabelWidget::Module::CicleBlue);
    m_ui->duplicateLoadingLabel->hide();
    m_ui->duplicateReCheckButton->hide();

    m_duplicateThread = new MusicSongCheckToolsDuplicateThread(this);
    connect(m_duplicateThread, SIGNAL(finished(MusicSongCheckToolsDuplicateList)), SLOT(duplicateCheckFinished(MusicSongCheckToolsDuplicateList)));
}

void MusicSongCheckToolsWidget::switchToSelectedItemStyle(int index)
{
    m_ui->renameButton->setStyleSheet(MusicUIObject::MQSSCheckTestRename);
    m_ui->qualityButton->setStyleSheet(MusicUIObject::MQSSCheckTestQuality);
    m_ui->duplicateButton->setStyleSheet(MusicUIObject::MQSSCheckTestDuplicate);

    m_ui->stackedWidget->setCurrentIndex(index);
    switch(index)
    {
        case 0: m_ui->renameButton->setStyleSheet(MusicUIObject::MQSSCheckTestRenameClicked); break;
        case 1: m_ui->duplicateButton->setStyleSheet(MusicUIObject::MQSSCheckTestDuplicateClicked); break;
        case 2: m_ui->qualityButton->setStyleSheet(MusicUIObject::MQSSCheckTestQualityClicked); break;
        default: break;
    }
}
