#include "musicpluginwidget.h"
#include "ui_musicpluginwidget.h"
#include "musicitemdelegate.h"
#include "musicpluginproperty.h"

#include <qmmp/decoderfactory.h>
#include <qmmp/effectfactory.h>
#include <qmmp/visualfactory.h>
#include <qmmp/inputsourcefactory.h>
#include <qmmp/outputfactory.h>
#include <qmmp/decoder.h>
#include <qmmp/effect.h>
#include <qmmp/visual.h>
#include <qmmp/inputsource.h>
#include <qmmp/output.h>

#include <QScrollBar>

/*! @brief The class of the plugin item widget.
 * @author Greedysky <greedysky@163.com>
 */
class MusicPluginItem : public QTreeWidgetItem
{
public:
    MusicPluginItem(QTreeWidgetItem *parent, DecoderFactory *factory, const QString &path)
        : QTreeWidgetItem(parent, PDecoder),
          m_factory(factory)
    {
        MusicPluginProperty property;
        property.m_name = factory->properties().name;
        property.m_hasSettings = factory->properties().hasSettings;
        property.m_description = factory->properties().description;
        property.m_type = path;
        initialize(Decoder::isEnabled(factory), true, property);
    }

    MusicPluginItem(QTreeWidgetItem *parent, EffectFactory *factory, const QString &path)
        : QTreeWidgetItem(parent, PEffect),
          m_factory(factory)
    {
        MusicPluginProperty property;
        property.m_name = factory->properties().name;
        property.m_hasSettings = factory->properties().hasSettings;
        property.m_type = path;
        initialize(Effect::isEnabled(factory), false, property);
    }

    MusicPluginItem(QTreeWidgetItem *parent, VisualFactory *factory, const QString &path)
        : QTreeWidgetItem(parent, PVisual),
          m_factory(factory)
    {
        MusicPluginProperty property;
        property.m_name = factory->properties().name;
        property.m_hasSettings = factory->properties().hasSettings;
        property.m_type = path;
        initialize(Visual::isEnabled(factory), false, property);
    }

    MusicPluginItem(QTreeWidgetItem *parent, InputSourceFactory *factory, const QString &path)
        : QTreeWidgetItem(parent, PTransports),
          m_factory(factory)
    {
        MusicPluginProperty property;
        property.m_name = factory->properties().name;
        property.m_hasSettings = factory->properties().hasSettings;
        property.m_type = path;
        initialize(InputSource::isEnabled(factory), true, property);
    }

    MusicPluginItem(QTreeWidgetItem *parent, OutputFactory *factory, const QString &path)
        : QTreeWidgetItem(parent, POutput),
          m_factory(factory)
    {
        MusicPluginProperty property;
        property.m_name = factory->properties().name;
        property.m_hasSettings = factory->properties().hasSettings;
        property.m_type = path;
        initialize(Output::currentFactory() == factory, true, property);
    }

    enum Module
    {
        PDecoder = QTreeWidgetItem::UserType,
        PEffect,
        PVisual,
        PTransports,
        POutput
    };

    bool hasSettings() const
    {
        switch(type())
        {
            case PDecoder: return TTKStatic_cast(DecoderFactory*, m_factory)->properties().hasSettings;
            case PEffect: return false;
            case PVisual: return false;
            case PTransports: return TTKStatic_cast(InputSourceFactory*, m_factory)->properties().hasSettings;
            case POutput: return TTKStatic_cast(OutputFactory*, m_factory)->properties().hasSettings;
            default: return false;
        }
    }

    void showSettingWidget() const
    {
        switch(type())
        {
            case PDecoder: TTKStatic_cast(DecoderFactory*, m_factory)->showSettings(treeWidget()); break;
            case PEffect: break;
            case PVisual: break;
            case PTransports: TTKStatic_cast(InputSourceFactory*, m_factory)->showSettings(treeWidget()); break;
            case POutput: TTKStatic_cast(OutputFactory*, m_factory)->showSettings(treeWidget()); break;
            default: break;
        }
    }

    void setEnabled(bool enabled)
    {
        switch(type())
        {
            case PDecoder: Decoder::setEnabled(TTKStatic_cast(DecoderFactory*, m_factory), enabled); break;
            case PEffect: break;
            case PVisual: break;
            case PTransports: InputSource::setEnabled(TTKStatic_cast(InputSourceFactory*, m_factory), enabled); break;
            case POutput:
            {
                if(enabled)
                {
                    Output::setCurrentFactory(TTKStatic_cast(OutputFactory*, m_factory));
                }
                break;
            }
            default: break;
        }
    }

    void initialize(bool state, bool enable, const MusicPluginProperty &property)
    {
        setData(0, TTK_CHECKED_ROLE, state ? Qt::Checked : Qt::Unchecked);
        setData(0, TTK_ENABLED_ROLE, enable);
        setData(1, TTK_DISPLAY_ROLE, property.m_name);
        setData(2, TTK_DISPLAY_ROLE, property.m_type.section('/', -1));

        if(!property.m_description.isEmpty())
        {
            setToolTip(1, property.m_description);
            setToolTip(2, property.m_description);
        }

        if(property.m_hasSettings)
        {
            setIcon(3, QIcon(":/contextMenu/btn_setting"));
        }

        const QColor &color = enable ? (state ? QColor(0xE6, 0x73, 0x00) : QColor(0x00, 0x00, 0x00)) : QColor(0xBB, 0xBB, 0xBB);
        setData(1, Qt::ForegroundRole, color);
        setData(2, Qt::ForegroundRole, color);
    }

private:
    void *m_factory;

};



MusicPluginWidget::MusicPluginWidget(QWidget *parent)
    : MusicAbstractMoveDialog(parent),
      m_ui(new Ui::MusicPluginWidget)
{
    m_ui->setupUi(this);
    setFixedSize(size());
    setBackgroundLabel(m_ui->background);

    m_ui->topTitleCloseButton->setIcon(QIcon(":/functions/btn_close_hover"));
    m_ui->topTitleCloseButton->setStyleSheet(MusicUIObject::MQSSToolButtonStyle04);
    m_ui->topTitleCloseButton->setCursor(QCursor(Qt::PointingHandCursor));
    m_ui->topTitleCloseButton->setToolTip(tr("Close"));
    connect(m_ui->topTitleCloseButton, SIGNAL(clicked()), SLOT(close()));

#if TTK_QT_VERSION_CHECK(5,0,0)
    m_ui->treeWidget->header()->setSectionsMovable(false);
    m_ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Fixed);
#else
    m_ui->treeWidget->header()->setMovable(false);
    m_ui->treeWidget->header()->setResizeMode(0, QHeaderView::Fixed);
#endif
    m_ui->treeWidget->setHeaderLabels({QString(), tr("Description"), tr("Name"), QString()});

    TTKCheckBoxItemDelegate *delegateCheck = new TTKCheckBoxItemDelegate(this);
    delegateCheck->setStyleSheet(MusicUIObject::MQSSCheckBoxStyle01);
    delegateCheck->setModuleMode(TTKAbstractItemDelegate::DisplayMode | TTKAbstractItemDelegate::TreeMode);
    m_ui->treeWidget->setItemDelegateForColumn(0, delegateCheck);

    TTKLabelItemDelegate *delegateTitle = new TTKLabelItemDelegate(this);
    delegateTitle->setModuleMode(TTKAbstractItemDelegate::ElideMode);
    delegateTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    delegateTitle->setStyleSheet(MusicUIObject::MQSSBackgroundStyle01);
    m_ui->treeWidget->setItemDelegateForColumn(1, delegateTitle);

    TTKLabelItemDelegate *delegateName = new TTKLabelItemDelegate(this);
    delegateName->setModuleMode(TTKAbstractItemDelegate::ElideMode);
    delegateName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    delegateName->setStyleSheet(MusicUIObject::MQSSBackgroundStyle01);
    m_ui->treeWidget->setItemDelegateForColumn(2, delegateName);

    QItemDelegate *delegateSetting = new QItemDelegate(this);
    m_ui->treeWidget->setItemDelegateForColumn(3, delegateSetting);

    m_ui->treeWidget->setColumnWidth(0, 65);
    m_ui->treeWidget->setColumnWidth(1, 210);
    m_ui->treeWidget->setColumnWidth(2, 120);
    m_ui->treeWidget->setColumnWidth(3, 70);

    m_ui->settingButton->setStyleSheet(MusicUIObject::MQSSPushButtonStyle03);
    m_ui->treeWidget->setStyleSheet(MusicUIObject::MQSSGroupBoxStyle01 + MusicUIObject::MQSSSpinBoxStyle01 + MusicUIObject::MQSSSliderStyle06 +
                                    MusicUIObject::MQSSRadioButtonStyle01 + MusicUIObject::MQSSCheckBoxStyle01 + MusicUIObject::MQSSComboBoxStyle01 +
                                    MusicUIObject::MQSSPushButtonStyle12 + MusicUIObject::MQSSLineEditStyle01);
    m_ui->treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ui->treeWidget->verticalScrollBar()->setStyleSheet(MusicUIObject::MQSSScrollBarStyle03);
    m_ui->treeWidget->setFocusPolicy(Qt::NoFocus);

    loadPluginsInfo();

    connect(m_ui->settingButton, SIGNAL(clicked()), SLOT(pluginButtonClicked()));
    connect(m_ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(pluginItemChanged(QTreeWidgetItem*,int)));
}

void MusicPluginWidget::pluginItemChanged(QTreeWidgetItem *item, int column)
{
    if(column == 0 && (item->type() == MusicPluginItem::PDecoder || item->type() == MusicPluginItem::PTransports || item->type() == MusicPluginItem::POutput))
    {
        if(item->type() == MusicPluginItem::POutput)
        {
            QTreeWidgetItem *parent = item->parent();
            if(!parent)
            {
                return;
            }

            for(int i = 0; i < parent->childCount(); ++i)
            {
                QTreeWidgetItem *it = parent->child(i);
                it->setData(column, TTK_CHECKED_ROLE, Qt::Unchecked);
                it->setData(1, Qt::ForegroundRole, QColor(0x00, 0x00, 0x00));
                it->setData(2, Qt::ForegroundRole, QColor(0x00, 0x00, 0x00));
            }
        }

        const Qt::CheckState status = TTKStatic_cast(Qt::CheckState, item->data(column, TTK_CHECKED_ROLE).toInt());
        item->setData(column, TTK_CHECKED_ROLE, status == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        TTKDynamic_cast(MusicPluginItem*, item)->setEnabled(status != Qt::Checked);

        const QColor &color = (status != Qt::Checked) ? QColor(0xE6, 0x73, 0x00) : QColor(0x00, 0x00, 0x00);
        item->setData(1, Qt::ForegroundRole, color);
        item->setData(2, Qt::ForegroundRole, color);
    }

    MusicPluginItem *it = TTKDynamic_cast(MusicPluginItem*, item);
    m_ui->settingButton->setEnabled(it ? it->hasSettings() : false);
}

void MusicPluginWidget::pluginButtonClicked()
{
    MusicPluginItem *item = TTKDynamic_cast(MusicPluginItem*, m_ui->treeWidget->currentItem());
    if(item)
    {
        item->showSettingWidget();
    }
}

void MusicPluginWidget::loadPluginsInfo()
{
    m_ui->treeWidget->blockSignals(true);

    QTreeWidgetItem *item = nullptr;
    item = new QTreeWidgetItem(m_ui->treeWidget, {tr("Decoder")});
    item->setFirstColumnSpanned(true);
    item->setExpanded(true);
    for(DecoderFactory *factory : Decoder::factories())
    {
        new MusicPluginItem(item, factory, Decoder::file(factory));
    }
    m_ui->treeWidget->addTopLevelItem(item);

    item = new QTreeWidgetItem(m_ui->treeWidget, {tr("Effect")});
    item->setFirstColumnSpanned(true);
    item->setExpanded(true);
    for(EffectFactory *factory : Effect::factories())
    {
        new MusicPluginItem(item, factory, Effect::file(factory));
    }
    m_ui->treeWidget->addTopLevelItem(item);

    item = new QTreeWidgetItem(m_ui->treeWidget, {tr("Visualization")});
    item->setFirstColumnSpanned(true);
    item->setExpanded(true);
    for(VisualFactory *factory : Visual::factories())
    {
        new MusicPluginItem(item, factory, Visual::file(factory));
    }
    m_ui->treeWidget->addTopLevelItem(item);

    item = new QTreeWidgetItem(m_ui->treeWidget, {tr("Transport")});
    item->setFirstColumnSpanned(true);
    item->setExpanded(true);
    for(InputSourceFactory *factory : InputSource::factories())
    {
        new MusicPluginItem(item, factory, InputSource::file(factory));
    }
    m_ui->treeWidget->addTopLevelItem(item);

    item = new QTreeWidgetItem(m_ui->treeWidget, {tr("Output")});
    item->setFirstColumnSpanned(true);
    item->setExpanded(true);
    for(OutputFactory *factory : Output::factories())
    {
        new MusicPluginItem(item, factory, Output::file(factory));
    }
    m_ui->treeWidget->addTopLevelItem(item);

    m_ui->treeWidget->blockSignals(false);
}
