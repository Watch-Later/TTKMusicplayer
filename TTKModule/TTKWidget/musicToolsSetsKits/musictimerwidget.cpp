#include "musictimerwidget.h"
#include "ui_musictimerwidget.h"
#include "musicsettingmanager.h"
#include "musicapplicationmodule.h"
#include "musictoolsetsuiobject.h"

#include <QButtonGroup>

MusicTimerWidget::MusicTimerWidget(QWidget *parent)
    : MusicAbstractMoveDialog(parent),
      m_ui(new Ui::MusicTimerWidget)
{
    m_ui->setupUi(this);
    setFixedSize(size());
    setBackgroundLabel(m_ui->background);

    m_ui->topTitleCloseButton->setIcon(QIcon(":/functions/btn_close_hover"));
    m_ui->topTitleCloseButton->setStyleSheet(MusicUIObject::MQSSToolButtonStyle04);
    m_ui->topTitleCloseButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->topTitleCloseButton->setToolTip(tr("Close"));
    connect(m_ui->topTitleCloseButton, SIGNAL(clicked()), SLOT(close()));

    m_ui->timerToPlay->setStyleSheet(MusicUIObject::MQSSTimeToPlay);
    m_ui->timerToStop->setStyleSheet(MusicUIObject::MQSSTimeToStop);
    m_ui->timerToShutdown->setStyleSheet(MusicUIObject::MQSSTimeToDown);
    m_ui->confirm->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);
    m_ui->cancel->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);
    m_ui->timerToPlay->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->timerToStop->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->timerToShutdown->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->confirm->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->cancel->setCursor(QCursor(Qt::PointingHandCursor));

    connect(m_ui->confirm, SIGNAL(clicked()), SLOT(saveParameterSettings()));
    connect(m_ui->cancel, SIGNAL(clicked()), SLOT(close()));

#ifdef Q_OS_UNIX
    m_ui->timerToPlay->setFocusPolicy(Qt::NoFocus);
    m_ui->timerToStop->setFocusPolicy(Qt::NoFocus);
    m_ui->timerToShutdown->setFocusPolicy(Qt::NoFocus);
    m_ui->confirm->setFocusPolicy(Qt::NoFocus);
    m_ui->cancel->setFocusPolicy(Qt::NoFocus);
#endif

    initComboParameter();
    initFirstWidget();
    initSecondWidget();
    initThreeWidget();
    initParemeter();

    QButtonGroup *buttonGroup0 = new QButtonGroup(this);
    buttonGroup0->addButton(m_ui->timerToPlay, 0);
    buttonGroup0->addButton(m_ui->timerToStop, 1);
    buttonGroup0->addButton(m_ui->timerToShutdown, 2);
    QtButtonGroupConnect(buttonGroup0, this, changeWidgetIndex);

    QButtonGroup *buttonGroup1 = new QButtonGroup(this);
    buttonGroup1->addButton(m_ui->noSetRadioButton1, 0);
    buttonGroup1->addButton(m_ui->setRadioButton1, 1);
    QtButtonGroupConnect(buttonGroup1, this, buttonClicked);

    QButtonGroup *buttonGroup2 = new QButtonGroup(this);
    buttonGroup2->addButton(m_ui->noSetRadioButton2, 2);
    buttonGroup2->addButton(m_ui->setRadioButton2, 3);
    QtButtonGroupConnect(buttonGroup2, this, buttonClicked);

    QButtonGroup *buttonGroup3 = new QButtonGroup(this);
    buttonGroup3->addButton(m_ui->noSetRadioButton3, 4);
    buttonGroup3->addButton(m_ui->setRadioButton3, 5);
    QtButtonGroupConnect(buttonGroup3, this, buttonClicked);

    changeWidgetIndex(0);
}

MusicTimerWidget::~MusicTimerWidget()
{
    delete m_ui;
}

void MusicTimerWidget::initParemeter()
{
    if(G_SETTING_PTR->value(MusicSettingManager::TimerAutoPlayMode).toInt() == 1)
    {
        m_ui->noSetRadioButton1->setChecked(true);
        setEnabledFirstControl(false);
    }
    else
    {
        m_ui->setRadioButton1->setChecked(true);
    }

    m_ui->hourComboBox1->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoPlayHour).toInt());
    m_ui->secComboBox1->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoPlaySecond).toInt());
    m_ui->repeatComboBox1->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoPlayRepeat).toInt());
    m_ui->plistComboBox->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoPlayItemIndex).toInt());

    if(G_SETTING_PTR->value(MusicSettingManager::TimerAutoStopMode).toInt() == 1)
    {
        m_ui->noSetRadioButton2->setChecked(true);
        setEnabledSecondControl(false);
    }
    else
    {
        m_ui->setRadioButton2->setChecked(true);
    }

    m_ui->hourComboBox2->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoStopHour).toInt());
    m_ui->secComboBox2->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoStopSecond).toInt());
    m_ui->repeatComboBox2->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoStopRepeat).toInt());

    if(G_SETTING_PTR->value(MusicSettingManager::TimerAutoShutdownMode).toInt() == 1)
    {
        m_ui->noSetRadioButton3->setChecked(true);
        setEnabledThreeControl(false);
    }
    else
    {
        m_ui->setRadioButton3->setChecked(true);
    }

    m_ui->hourComboBox3->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoShutdownHour).toInt());
    m_ui->secComboBox3->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoShutdownSecond).toInt());
    m_ui->repeatComboBox3->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoShutdownRepeat).toInt());
}

void MusicTimerWidget::writeParemeter() const
{
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoIndex, m_ui->stackedWidget->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoPlayMode, m_ui->noSetRadioButton1->isChecked() ? 1 : 0);
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoPlayHour, m_ui->hourComboBox1->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoPlaySecond, m_ui->secComboBox1->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoPlayRepeat, m_ui->repeatComboBox1->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoPlayItemIndex, m_ui->plistComboBox->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoPlaySongIndex, m_ui->psongComboBox->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoStopMode, m_ui->noSetRadioButton2->isChecked() ? 1 : 0);
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoStopHour, m_ui->hourComboBox2->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoStopSecond, m_ui->secComboBox2->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoStopRepeat, m_ui->repeatComboBox2->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoShutdownMode, m_ui->noSetRadioButton3->isChecked() ? 1 : 0);
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoShutdownHour, m_ui->hourComboBox3->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoShutdownSecond, m_ui->secComboBox3->currentIndex());
    G_SETTING_PTR->setValue(MusicSettingManager::TimerAutoShutdownRepeat, m_ui->repeatComboBox3->currentIndex());
}

void MusicTimerWidget::setSongStringList(const QStringList &list)
{
    m_ui->psongComboBox->addItems(list);
    m_ui->psongComboBox->setCurrentIndex(G_SETTING_PTR->value(MusicSettingManager::TimerAutoPlaySongIndex).toInt());
}

void MusicTimerWidget::initComboParameter()
{
    for(int i = 0; i < MT_D; ++i)
    {
        m_hour << tr("%1H").arg(i).rightJustified(3, '0');
    }

    for(int i = 0; i < MT_H; ++i)
    {
        m_second << tr("%1S").arg(i).rightJustified(3, '0');
    }
    m_repeat << tr("Once") << tr("Month") << tr("Week") << tr("Day");
}

void MusicTimerWidget::changeWidgetIndex(int index)
{
    m_ui->timerToPlay->setStyleSheet(MusicUIObject::MQSSTimeToPlay);
    m_ui->timerToStop->setStyleSheet(MusicUIObject::MQSSTimeToStop);
    m_ui->timerToShutdown->setStyleSheet(MusicUIObject::MQSSTimeToDown);

    switch(index)
    {
        case 0: m_ui->timerToPlay->setStyleSheet(MusicUIObject::MQSSTimeToPlayClicked); break;
        case 1: m_ui->timerToStop->setStyleSheet(MusicUIObject::MQSSTimeToStopClicked); break;
        case 2: m_ui->timerToShutdown->setStyleSheet(MusicUIObject::MQSSTimeToDownClicked); break;
        default: break;
    }

    m_ui->stackedWidget->setCurrentIndex(index);
}

void MusicTimerWidget::initFirstWidget()
{
    m_ui->hourComboBox1->addItems(m_hour);
    m_ui->secComboBox1->addItems(m_second);
    m_ui->repeatComboBox1->addItems(m_repeat);
    m_ui->plistComboBox->addItem(tr("Defualt"));
    m_ui->noSetRadioButton1->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);
    m_ui->setRadioButton1->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);

    MusicUtils::Widget::generateComboBoxFormat(m_ui->hourComboBox1);
    MusicUtils::Widget::generateComboBoxFormat(m_ui->secComboBox1);
    MusicUtils::Widget::generateComboBoxFormat(m_ui->repeatComboBox1);
    MusicUtils::Widget::generateComboBoxFormat(m_ui->plistComboBox);
    MusicUtils::Widget::generateComboBoxFormat(m_ui->psongComboBox);
#ifdef Q_OS_UNIX
    m_ui->noSetRadioButton1->setFocusPolicy(Qt::NoFocus);
    m_ui->setRadioButton1->setFocusPolicy(Qt::NoFocus);
#endif
}

void MusicTimerWidget::initSecondWidget()
{
    m_ui->hourComboBox2->addItems(m_hour);
    m_ui->secComboBox2->addItems(m_second);
    m_ui->repeatComboBox2->addItems(m_repeat);
    m_ui->noSetRadioButton2->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);
    m_ui->setRadioButton2->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);

    MusicUtils::Widget::generateComboBoxFormat(m_ui->hourComboBox2);
    MusicUtils::Widget::generateComboBoxFormat(m_ui->secComboBox2);
    MusicUtils::Widget::generateComboBoxFormat(m_ui->repeatComboBox2);
#ifdef Q_OS_UNIX
    m_ui->noSetRadioButton2->setFocusPolicy(Qt::NoFocus);
    m_ui->setRadioButton2->setFocusPolicy(Qt::NoFocus);
#endif
}

void MusicTimerWidget::initThreeWidget()
{
    m_ui->hourComboBox3->addItems(m_hour);
    m_ui->secComboBox3->addItems(m_second);
    m_ui->repeatComboBox3->addItems(m_repeat);
    m_ui->noSetRadioButton3->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);
    m_ui->setRadioButton3->setStyleSheet(MusicUIObject::MQSSRadioButtonStyle01);

    MusicUtils::Widget::generateComboBoxFormat(m_ui->hourComboBox3);
    MusicUtils::Widget::generateComboBoxFormat(m_ui->secComboBox3);
    MusicUtils::Widget::generateComboBoxFormat(m_ui->repeatComboBox3);
#ifdef Q_OS_UNIX
    m_ui->noSetRadioButton3->setFocusPolicy(Qt::NoFocus);
    m_ui->setRadioButton3->setFocusPolicy(Qt::NoFocus);
#endif
}

void MusicTimerWidget::saveParameterSettings()
{
    writeParemeter();
    MusicApplicationModule::instance()->musicToolSetsParameter();
    close();
}

void MusicTimerWidget::buttonClicked(int index)
{
    switch(index)
    {
        case 0: setEnabledFirstControl(false); break;
        case 1: setEnabledFirstControl(true); break;
        case 2: setEnabledSecondControl(false); break;
        case 3: setEnabledSecondControl(true); break;
        case 4: setEnabledThreeControl(false); break;
        case 5: setEnabledThreeControl(true); break;
        default: break;
    }
}

void MusicTimerWidget::setEnabledFirstControl(bool enable)
{
    m_ui->hourComboBox1->setEnabled(enable);
    m_ui->secComboBox1->setEnabled(enable);
    m_ui->repeatComboBox1->setEnabled(enable);
    m_ui->plistComboBox->setEnabled(enable);
    m_ui->psongComboBox->setEnabled(enable);
}

void MusicTimerWidget::setEnabledSecondControl(bool enable)
{
    m_ui->hourComboBox2->setEnabled(enable);
    m_ui->secComboBox2->setEnabled(enable);
    m_ui->repeatComboBox2->setEnabled(enable);
}

void MusicTimerWidget::setEnabledThreeControl(bool enable)
{
    m_ui->hourComboBox3->setEnabled(enable);
    m_ui->secComboBox3->setEnabled(enable);
    m_ui->repeatComboBox3->setEnabled(enable);
}
