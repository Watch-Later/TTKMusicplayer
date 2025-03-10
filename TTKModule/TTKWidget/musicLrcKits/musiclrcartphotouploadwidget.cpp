#include "musiclrcartphotouploadwidget.h"
#include "ui_musiclrcartphotouploadwidget.h"
#include "musictoastlabel.h"
#include "musicfileutils.h"

MusicLrcArtPhotoUploadWidget::MusicLrcArtPhotoUploadWidget(QWidget *parent)
    : MusicAbstractMoveDialog(parent),
      m_ui(new Ui::MusicLrcArtPhotoUploadWidget)
{
    m_ui->setupUi(this);
    setFixedSize(size());
    setBackgroundLabel(m_ui->background);
    
    m_ui->topTitleCloseButton->setIcon(QIcon(":/functions/btn_close_hover"));
    m_ui->topTitleCloseButton->setStyleSheet(MusicUIObject::MQSSToolButtonStyle04);
    m_ui->topTitleCloseButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->topTitleCloseButton->setToolTip(tr("Close"));
    connect(m_ui->topTitleCloseButton, SIGNAL(clicked()), SLOT(close()));

    m_ui->artSearchEdit->setStyleSheet(MusicUIObject::MQSSLineEditStyle01);
    m_ui->uploadButton->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);
    m_ui->closeButton->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);
    m_ui->selectButton->setStyleSheet(MusicUIObject::MQSSPushButtonStyle04);

#ifdef Q_OS_UNIX
    m_ui->uploadButton->setFocusPolicy(Qt::NoFocus);
    m_ui->closeButton->setFocusPolicy(Qt::NoFocus);
    m_ui->selectButton->setFocusPolicy(Qt::NoFocus);
#endif

    m_ui->uploadButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->closeButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->selectButton->setCursor(QCursor(Qt::PointingHandCursor));

    m_ui->stateLabel->setStyleSheet(MusicUIObject::MQSSBackgroundStyle12 + MusicUIObject::MQSSColorStyle07);

    m_ui->uploadButton->hide();
    m_ui->closeButton->hide();
    m_ui->stateLabel->hide();

    connect(m_ui->selectButton, SIGNAL(clicked()), SLOT(selectButtonClicked()));
    connect(m_ui->closeButton, SIGNAL(clicked()), SLOT(close()));
    connect(m_ui->uploadButton, SIGNAL(clicked()), SLOT(uploadButtonClicked()));
    connect(m_ui->imageLabel, SIGNAL(deltaValueChanged(float)), SLOT(deltaValueChanged(float)));
}

MusicLrcArtPhotoUploadWidget::~MusicLrcArtPhotoUploadWidget()
{
    delete m_ui;
}

void MusicLrcArtPhotoUploadWidget::deltaValueChanged(float v)
{
    m_ui->deltaValueLabel->setText(QString::number(TTKStatic_cast(int, v*100)) + "%");
}

void MusicLrcArtPhotoUploadWidget::selectButtonClicked()
{
    const QString &path = MusicUtils::File::getOpenFileName(this);
    if(path.isEmpty())
    {
        return;
    }

    const QPixmap pix(path);
    if(pix.width() < WINDOW_WIDTH_MIN || pix.height() < WINDOW_HEIGHT_MIN)
    {
        m_ui->stateLabel->show();
        m_ui->uploadButton->hide();
        m_ui->closeButton->hide();
    }
    else
    {
        m_ui->stateLabel->hide();
        m_ui->uploadButton->show();
        m_ui->closeButton->show();
        m_ui->introduceLabel->hide();
        m_ui->selectButton->hide();
        m_ui->imageLabel->setImagePath(path);
    }
}

void MusicLrcArtPhotoUploadWidget::uploadButtonClicked()
{
    const QDir dir(BACKGROUND_DIR_FULL);
    int count = 0;
    const QString &name = m_ui->artSearchEdit->text().trimmed();
    if(name.isEmpty())
    {
        MusicToastLabel::popup(tr("The art is empty!"));
        return;
    }

    for(const QFileInfo &fin : dir.entryInfoList())
    {
        if(fin.fileName().contains(name))
        {
            ++count;
        }
    }

    const QString &fileName = QString("%1%2%3").arg(BACKGROUND_DIR_FULL, name).arg(count);
    m_ui->imageLabel->saveImagePath(fileName + JPG_FILE);
    QFile::rename(fileName + JPG_FILE, fileName + SKN_FILE);
    close();
}
