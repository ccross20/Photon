#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QColorDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include "waveformheader_p.h"
#include "sequence/sequence.h"
#include "plugin/pluginfactory.h"
#include "audio/audioprocessor.h"
#include "gui/menufactory.h"
#include "photoncore.h"
#include "sequence/cuelayer.h"
#include "sequencewaveformeditor.h"
#include "audio/songdata.h"
#include "numberscrubfield.h"

namespace photon {

namespace {

// Column -> glyph: magnet (snappable), eye (visible), pencil (editable). Written as
// explicit UTF-16 escapes (surrogate pairs for the two codepoints above the BMP,
// plus a trailing U+FE0F variation selector to force colour-emoji presentation)
// rather than pasted emoji characters, so this can't depend on how the source file
// itself is encoded/read. Column 0 (the colour swatch) is handled separately in
// paint()/editorEvent() rather than through a glyph.
QString columnGlyph(int t_column)
{
    switch(t_column)
    {
        case 2: return QStringLiteral(u"\xD83E\xDDF2");                 // U+1F9F2 magnet - isSnappable
        case 3: return QStringLiteral(u"\xD83D\xDC41\xFE0F");           // U+1F441 eye - isVisible
        case 4: return QStringLiteral(u"\x270F\xFE0F");                 // U+270F pencil - isEditable
        default: return QString();
    }
}

// Cycled through (not randomised) as each new layer is created, so successive
// layers are reliably distinct rather than occasionally clashing.
const QVector<QColor> &layerColorPalette()
{
    static const QVector<QColor> kPalette = {
        QColor(230, 25, 75),    // red
        QColor(60, 180, 75),    // green
        QColor(255, 145, 0),    // orange
        QColor(0, 130, 200),    // blue
        QColor(245, 205, 40),   // yellow
        QColor(145, 30, 180),   // purple
        QColor(0, 200, 190),    // teal
        QColor(240, 50, 190),   // magenta
    };
    return kPalette;
}

} // namespace

CueLayerDelegate::CueLayerDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{

}

void CueLayerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect iconRect = QRect(option.rect.topLeft(),option.rect.size()).adjusted(2,2,-2,-2);

    if(index.column() == 0)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(option.palette.color(QPalette::Mid), 1));
        painter->setBrush(index.data().value<QColor>());
        painter->drawRoundedRect(iconRect, 3, 3);
        painter->restore();
        return;
    }

    const QString glyph = columnGlyph(index.column());
    if(glyph.isEmpty())
        return;

    QFont font = painter->font();
    font.setPixelSize(qMin(iconRect.width(), iconRect.height()));

    painter->save();
    painter->setFont(font);
    // On/off reads via opacity rather than recolouring - these are colour emoji
    // glyphs, so a pen-colour change wouldn't affect them.
    painter->setOpacity(index.data().toBool() ? 1.0 : 0.25);
    painter->drawText(iconRect, Qt::AlignCenter, glyph);
    painter->restore();
}

bool CueLayerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(event->type() != QEvent::MouseButtonPress)
        return false;

    if(index.column() == 0)
    {
        const QColor picked = QColorDialog::getColor(index.data().value<QColor>(), nullptr, "Layer Color");
        if(picked.isValid())
            model->setData(index, picked);
        return true;
    }

    // The editable column (4) is exclusive - at most one layer is ever the
    // editable one - so clicking it always selects that layer rather than
    // toggling it off, unlike the independent snappable/visible flags.
    if(index.column() == 4)
    {
        model->setData(index, true);
        return true;
    }

    bool curValue = index.data().toBool();
    model->setData(index,!curValue);
    return true;
}

QSize CueLayerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
        return QSize(32,32);
}



void CueLayerModel::cueLayerAdded(photon::CueLayer *t_layer)
{
    beginResetModel();
    endResetModel();
}

void CueLayerModel::cueLayerRemoved(photon::CueLayer *t_layer)
{
    beginResetModel();
    endResetModel();
}

QVariant CueLayerModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || !m_sequence)
        return QVariant();
    auto beat = m_sequence->cueLayers()[index.row()];

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column()) {
            case 0:
                return beat->color();
            case 1:
                return beat->name();
            case 2:
                return beat->isSnappable();
            case 3:
                return beat->isVisible();
            case 4:
                return beat->isEditable();
        }

    }

    return QVariant();
}

QVariant CueLayerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

QModelIndex CueLayerModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!m_sequence)
        return QModelIndex();
    return createIndex(row, column, m_sequence->cueLayers()[row]);
}

QModelIndex CueLayerModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int CueLayerModel::rowCount(const QModelIndex &parent) const
{
    if(!m_sequence || parent.isValid())
        return 0;
    return m_sequence->cueLayers().length();
}

int CueLayerModel::columnCount(const QModelIndex &parent) const
{
    return 5;
}


Qt::ItemFlags CueLayerModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool CueLayerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || !m_sequence)
        return false;
    auto beat = m_sequence->cueLayers()[index.row()];

    if(role == Qt::EditRole)
    {
        switch (index.column()) {
            case 0:
                beat->setColor(value.value<QColor>());
            break;
            case 1:
                beat->setName(value.toString());
            break;
            case 2:
                beat->setIsSnappable(value.toBool());
            break;
            case 3:
                beat->setIsVisible(value.toBool());
            break;
            case 4:
                // Routes through Sequence so it clears isEditable on every other
                // layer and emits editableCueLayerChanged - a direct
                // beat->setIsEditable() wouldn't enforce "only one editable layer"
                // or tell the waveform editor which layer to add markers to.
                m_sequence->setEditableCueLayer(beat);
            break;
        }
        return true;
    }

    return false;
}

void CueLayerModel::setSequence(Sequence *t_sequence)
{
    beginResetModel();
    m_sequence = t_sequence;

    connect(m_sequence, &Sequence::cueLayerAdded, this, &CueLayerModel::cueLayerAdded);
    connect(m_sequence, &Sequence::cueLayerRemoved, this, &CueLayerModel::cueLayerRemoved);
    connect(m_sequence, &Sequence::editableCueLayerChanged, this, &CueLayerModel::cueLayerRemoved);

    endResetModel();
}




WaveformHeader::WaveformHeader(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->delegate = new CueLayerDelegate;
    m_impl->cueModel = new CueLayerModel;
    m_impl->tree = new QTreeView;
    m_impl->tree->setHeaderHidden(true);
    m_impl->tree->setModel(m_impl->cueModel);
    m_impl->tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_impl->tree->setSelectionMode(QAbstractItemView::SingleSelection);
    // This is a flat list (no children), but QTreeView still reserves indentation
    // space in column 0 for the branch/expand arrow unless told otherwise - without
    // this, the colour swatch column gets squeezed down to a couple of pixels.
    m_impl->tree->setRootIsDecorated(false);
    m_impl->tree->setItemDelegateForColumn(0, m_impl->delegate);
    m_impl->tree->setItemDelegateForColumn(2, m_impl->delegate);
    m_impl->tree->setItemDelegateForColumn(3, m_impl->delegate);
    m_impl->tree->setItemDelegateForColumn(4, m_impl->delegate);
    m_impl->tree->setColumnWidth(0,24);
    m_impl->tree->setColumnWidth(2,32);
    m_impl->tree->setColumnWidth(3,32);
    m_impl->tree->setColumnWidth(4,32);

    auto vLayout = new QVBoxLayout;

    m_impl->addButton = new QPushButton("Add");
    m_impl->layerMenuButton = new QPushButton(QStringLiteral(u"\x22EE")); // vertical ellipsis
    m_impl->layerMenuButton->setMaximumWidth(28);

    auto buttonRow = new QHBoxLayout;
    buttonRow->addWidget(m_impl->addButton);
    buttonRow->addWidget(m_impl->layerMenuButton);

    vLayout->addWidget(m_impl->tree);
    vLayout->addLayout(buttonRow);
    vLayout->addStretch();
    setLayout(vLayout);

    connect(m_impl->addButton, &QPushButton::clicked, this, &WaveformHeader::addClicked);

    QMenu *layerMenu = new QMenu(m_impl->layerMenuButton);
    layerMenu->addAction("Delete Selected Layer", this, &WaveformHeader::deleteSelectedLayerClicked);
    layerMenu->addAction("Convert Beats to Markers", this, &WaveformHeader::convertBeatsToMarkersClicked);
    layerMenu->addSeparator();
    // Act on the markers selected in the waveform, or the whole editable
    // layer when none are; paste goes into the editable layer.
    QAction *cutAction = layerMenu->addAction("Cut Markers", this, &WaveformHeader::cutMarkersRequested);
    cutAction->setShortcut(QKeySequence::Cut);
    QAction *copyAction = layerMenu->addAction("Copy Markers", this, &WaveformHeader::copyMarkersRequested);
    copyAction->setShortcut(QKeySequence::Copy);
    QAction *pasteAction = layerMenu->addAction("Paste Markers", this, &WaveformHeader::pasteMarkersRequested);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(layerMenu, &QMenu::aboutToShow, this, [this, cutAction, copyAction, pasteAction]() {
        CueLayer *layer = m_impl->sequence ? m_impl->sequence->editableCueLayer() : nullptr;
        const bool hasMarkers = layer && !layer->markers().isEmpty();
        cutAction->setEnabled(hasMarkers);
        copyAction->setEnabled(hasMarkers);
        pasteAction->setEnabled(layer && SequenceWaveformEditor::clipboardHasMarkers());
    });
    m_impl->layerMenuButton->setMenu(layerMenu);
}

WaveformHeader::~WaveformHeader()
{
    delete m_impl;
}

void WaveformHeader::addClicked()
{
    const QVector<QColor> &palette = layerColorPalette();

    CueLayer *layer = new CueLayer;
    layer->setName("New Layer");
    // Cycle through the palette by how many layers already exist, rather than
    // a separately-tracked counter, so this stays stable across the header
    // being recreated (e.g. closing/reopening the panel).
    layer->setColor(palette[m_impl->sequence->cueLayers().count() % palette.size()]);
    m_impl->sequence->addCueLayer(layer);
    m_impl->sequence->setEditableCueLayer(layer);
}

void WaveformHeader::deleteSelectedLayerClicked()
{
    const QModelIndex index = m_impl->tree->currentIndex();
    if(!index.isValid())
        return;

    CueLayer *layer = static_cast<CueLayer*>(index.internalPointer());
    if(!layer)
        return;

    if(QMessageBox::question(this, "Delete Layer",
           QString("Delete cue layer \"%1\"? This cannot be undone.").arg(layer->name()))
       != QMessageBox::Yes)
        return;

    m_impl->sequence->removeCueLayer(layer);
    delete layer;
}

void WaveformHeader::convertBeatsToMarkersClicked()
{
    if(!m_impl->sequence)
        return;

    if(m_impl->convertDialog)
    {
        m_impl->convertDialog->raise();
        m_impl->convertDialog->activateWindow();
        return;
    }

    if(!m_impl->sequence->editableCueLayer())
    {
        QMessageBox::information(this, "Convert Beats to Markers", "No layer is currently editable.");
        return;
    }
    if(m_impl->sequence->songData()->beats().isEmpty())
    {
        QMessageBox::information(this, "Convert Beats to Markers",
            "This sequence has no analysed beat grid to convert.");
        return;
    }

    // Same rate choices and offset (in beats) as the DJ Connector node, so a
    // marker grid can match what a live graph is pulsing to. Remembered for
    // the session so repeat conversions don't need re-entering.
    struct Rate { const char *label; double multiplier; };
    static const Rate kRates[] = { {"/4", 0.25}, {"/2", 0.5}, {"1", 1.0}, {"x2", 2.0}, {"x4", 4.0}, {"x8", 8.0} };
    static int s_rateIndex = 2;
    static double s_offset = 0.0;

    // Non-modal so the waveform can still be zoomed and scrolled to check the
    // previewed markers against the audio before committing.
    auto *dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("Convert Beats to Markers");
    m_impl->convertDialog = dialog;
    auto *form = new QFormLayout(dialog);

    auto *rateCombo = new QComboBox;
    for(const Rate &rate : kRates)
        rateCombo->addItem(rate.label);
    rateCombo->setCurrentIndex(s_rateIndex);
    form->addRow("Rate", rateCombo);

    auto *offsetField = new NumberScrubField;
    offsetField->setDecimals(2);
    offsetField->setSoftRange(-4.0, 4.0);
    offsetField->setValue(s_offset);
    form->addRow("Offset (beats)", offsetField);

    auto *countLabel = new QLabel;
    form->addRow(countLabel);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("Add Markers");
    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    form->addRow(buttons);

    // Recomputed from the sequence each time rather than captured, in case its
    // beat grid is re-analysed while the dialog is open.
    auto pulseTimes = [this, rateCombo, offsetField]() {
        const BeatGrid &grid = m_impl->sequence->songData()->beats();
        return grid.pulseTimes(kRates[rateCombo->currentIndex()].multiplier, offsetField->value());
    };
    auto refreshPreview = [this, pulseTimes, countLabel]() {
        const QVector<double> times = pulseTimes();
        countLabel->setText(QString("%1 markers").arg(times.size()));
        emit previewMarkersChanged(times);
    };
    connect(rateCombo, &QComboBox::currentIndexChanged, dialog, refreshPreview);
    connect(offsetField, &NumberScrubField::valueChanged, dialog, refreshPreview);

    connect(dialog, &QDialog::accepted, this, [this, pulseTimes, rateCombo, offsetField]() {
        s_rateIndex = rateCombo->currentIndex();
        s_offset = offsetField->value();

        CueLayer *layer = m_impl->sequence->editableCueLayer();
        if(!layer)
            return;
        QList<float> markers;
        for(double time : pulseTimes())
            markers.append(static_cast<float>(time));
        layer->addMarkers(markers);
        layer->sort();
    });
    connect(dialog, &QDialog::finished, this, [this]() {
        emit previewMarkersChanged({});
    });

    refreshPreview();
    dialog->show();
}

void WaveformHeader::addAudioProcessor(AudioProcessor *t_processor)
{
    t_processor->init(m_impl->sequence);
    t_processor->startProcessing();

    connect(t_processor, &AudioProcessor::completed,this,[t_processor](){
        delete t_processor;
    });
}

void WaveformHeader::setSequence(Sequence *t_sequence)
{
    if(m_impl->convertDialog)
        m_impl->convertDialog->close();
    m_impl->sequence = t_sequence;
    m_impl->cueModel->setSequence(t_sequence);
}

} // namespace photon
