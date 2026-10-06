#include <QVBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSignalBlocker>
#include "fixtureeditorwidget.h"
#include "fixture.h"
#include "colorcalibrationdialog.h"
#include "capability/colorcapability.h"
#include "scene/sceneobject.h"
#include "vector3edit.h"
#include "propertycombobox.h"
#include "propertywidgets.h"
#include "tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class FixtureEditorWidget::Impl
{
public:
    Impl();
    QVector<Fixture*> fixtures;
    PropertyForm *form;
    QLineEdit *nameEdit;
    QTextEdit *commentEdit;
    QLineEdit *identifierEdit;
    TagEditorWidget *tagEditor;
    QLabel *manufacturerLabel;
    QLabel *descriptionLabel;
    QSpinBox *universeSpin;
    QSpinBox *offsetSpin;
    PropertyComboBox *modeCombo;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
    QDoubleSpinBox *panOffsetSpin;
    QDoubleSpinBox *tiltOffsetSpin;
    QCheckBox *panFlipCheck;
    QCheckBox *tiltFlipCheck;
    QCheckBox *panInvertCheck;
    QCheckBox *tiltInvertCheck;
    PropertyComboBox *modelCombo;
    PropertyComboBox *beamCombo;
    QPushButton *calibrateColorsButton;
};

FixtureEditorWidget::Impl::Impl()
{
    form = new PropertyForm;

    form->addSection("General");

    nameEdit = new QLineEdit;
    form->addRow("Name", nameEdit);

    identifierEdit = new QLineEdit;
    form->addRow("Identifier", identifierEdit);

    commentEdit = new QTextEdit;
    commentEdit->setMaximumHeight(60);
    commentEdit->setAcceptRichText(false);
    form->addRow("Comment", commentEdit);

    manufacturerLabel = new QLabel;
    manufacturerLabel->setProperty("readOnlyField", true);
    form->addRow("Manufacturer", manufacturerLabel);

    descriptionLabel = new QLabel;
    descriptionLabel->setProperty("readOnlyField", true);
    form->addRow("Description", descriptionLabel);

    form->addSection("Patch");

    universeSpin = new QSpinBox;
    universeSpin->setMinimum(1);
    universeSpin->setMaximum(9999);
    form->addRow("Universe", universeSpin);

    offsetSpin = new QSpinBox;
    offsetSpin->setMinimum(1);
    offsetSpin->setMaximum(511);
    form->addRow("Starting Channel", offsetSpin);

    modeCombo = new PropertyComboBox;
    form->addRow("DMX Mode", modeCombo);

    form->addSection("Appearance");

    modelCombo = new PropertyComboBox;
    // Index 0 = Auto (empty override); the rest are visualiser model types.
    modelCombo->addItems(QStringList() << "Auto" << "mover" << "par" << "uplight"
                                       << "strobe" << "blinder" << "bar" << "wash" << "beeeye");
    form->addRow("Model", modelCombo);

    beamCombo = new PropertyComboBox;
    // Index 0 = Auto (follow the visualiser's global beam toggle); 1 = basic
    // cone, 2 = volumetric, 3 = no beam at all. Tokens stored on the fixture
    // are "", "cones", "volumetric", "none".
    beamCombo->addItems(QStringList() << "Auto" << "Cones" << "Volumetric" << "None");
    form->addRow("Beam Style", beamCombo);

    form->addSection("Color");

    calibrateColorsButton = new QPushButton("Calibrate Colors...");
    calibrateColorsButton->setToolTip(
        "Tunes how this fixture's color LEDs (Red, Green, Blue, Amber, Lime,\n"
        "White, ...) mix to reproduce a set of named reference hues, driving\n"
        "a real patched fixture live so the result can be judged by eye.\n"
        "Only available with a single fixture selected that has a color\n"
        "capability.");
    form->addRow("", calibrateColorsButton);

    // Shows the union of the current selection's tags. Adding a chip applies
    // it to every selected fixture; removing one removes it from whichever
    // fixtures have it - each fixture's own tags outside that union are left
    // alone. Replaces the old "[multiple]" sentinel, which could otherwise be
    // committed back as a literal tag.
    tagEditor = new TagEditorWidget(
        [this](){
            QStringList unionTags;
            for(auto *fixture : fixtures)
                for(const auto &tag : fixture->tags())
                    if(!unionTags.contains(tag))
                        unionTags.append(tag);
            unionTags.sort();
            return unionTags;
        },
        [this](const QStringList &newTags){
            QStringList before;
            for(auto *fixture : fixtures)
                for(const auto &tag : fixture->tags())
                    if(!before.contains(tag))
                        before.append(tag);

            QStringList added = newTags;
            for(const auto &tag : before)
                added.removeAll(tag);
            QStringList removed = before;
            for(const auto &tag : newTags)
                removed.removeAll(tag);

            for(auto *fixture : fixtures)
            {
                QStringList tags = fixture->tags();
                for(const auto &tag : added)
                    if(!tags.contains(tag))
                        tags.append(tag);
                for(const auto &tag : removed)
                    tags.removeAll(tag);
                fixture->setTags(tags);
            }
        },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    form->addRow("Tags", tagEditor);

    form->addSection("Transform");

    positionEdit = new Vector3Edit(Vector3Edit::Distance);
    form->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit(Vector3Edit::Angle);
    form->addRow("Rotation", rotationEdit);

    panOffsetSpin = new QDoubleSpinBox;
    panOffsetSpin->setRange(-180.0, 180.0);
    panOffsetSpin->setDecimals(1);
    panOffsetSpin->setSuffix(QStringLiteral("°"));
    panOffsetSpin->setToolTip(
        "Degrees added to the pan channel before it's written to DMX.\n"
        "Lets a fixture mounted at an odd angle still be controlled as if it\n"
        "were mounted normally: Pan centered (0%) points wherever it actually\n"
        "needs to - shown live by the front-facing arrow drawn on the fixture\n"
        "in the visualiser while it's selected.");
    form->addRow("Pan Offset", panOffsetSpin);

    panFlipCheck = new QCheckBox;
    panFlipCheck->setToolTip(
        "Reverses the pan channel's direction of travel before it's written\n"
        "to DMX, for a fixture mounted flipped so \"more\" pan currently moves\n"
        "it the wrong way. Applied before Pan Offset, so the offset always\n"
        "reads in whichever direction the fixture now actually responds to.");
    form->addRow("Pan Flip", panFlipCheck);

    panInvertCheck = new QCheckBox;
    panInvertCheck->setToolTip(
        "Flips only the visualiser's preview of the pan direction, without\n"
        "touching DMX - for eyeballing a flip before committing to Pan Flip.");
    form->addRow("Pan Invert (preview only)", panInvertCheck);

    tiltOffsetSpin = new QDoubleSpinBox;
    tiltOffsetSpin->setRange(-180.0, 180.0);
    tiltOffsetSpin->setDecimals(1);
    tiltOffsetSpin->setSuffix(QStringLiteral("°"));
    tiltOffsetSpin->setToolTip(
        "Degrees added to the tilt channel before it's written to DMX - the\n"
        "same idea as Pan Offset, for a fixture hung or mounted at an odd\n"
        "tilt angle.");
    form->addRow("Tilt Offset", tiltOffsetSpin);

    tiltFlipCheck = new QCheckBox;
    tiltFlipCheck->setToolTip("Same idea as Pan Flip, for the tilt channel - changes real DMX.");
    form->addRow("Tilt Flip", tiltFlipCheck);

    tiltInvertCheck = new QCheckBox;
    tiltInvertCheck->setToolTip(
        "Same idea as Pan Invert, for the tilt channel - visualiser-only,\n"
        "does not change the real DMX value.");
    form->addRow("Tilt Invert (preview only)", tiltInvertCheck);
}

FixtureEditorWidget::FixtureEditorWidget(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_impl->form);

    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &FixtureEditorWidget::setName);
    connect(m_impl->identifierEdit, &QLineEdit::textEdited, this, &FixtureEditorWidget::setIdentifier);
    connect(m_impl->commentEdit, &QTextEdit::textChanged, this, &FixtureEditorWidget::updateComments);
    connect(m_impl->universeSpin, &QSpinBox::valueChanged, this, &FixtureEditorWidget::setUniverse);
    connect(m_impl->offsetSpin, &QSpinBox::valueChanged, this, &FixtureEditorWidget::setOffset);
    connect(m_impl->modeCombo, &QComboBox::activated, this, &FixtureEditorWidget::setMode);
    connect(m_impl->modelCombo, &QComboBox::activated, this, &FixtureEditorWidget::setModelType);
    connect(m_impl->beamCombo, &QComboBox::activated, this, &FixtureEditorWidget::setBeamStyle);
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &FixtureEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &FixtureEditorWidget::setRotation);
    connect(m_impl->panOffsetSpin, &QDoubleSpinBox::valueChanged, this, &FixtureEditorWidget::setPanOffset);
    connect(m_impl->tiltOffsetSpin, &QDoubleSpinBox::valueChanged, this, &FixtureEditorWidget::setTiltOffset);
    connect(m_impl->panFlipCheck, &QCheckBox::toggled, this, &FixtureEditorWidget::setPanFlip);
    connect(m_impl->tiltFlipCheck, &QCheckBox::toggled, this, &FixtureEditorWidget::setTiltFlip);
    connect(m_impl->panInvertCheck, &QCheckBox::toggled, this, &FixtureEditorWidget::setPanInvert);
    connect(m_impl->tiltInvertCheck, &QCheckBox::toggled, this, &FixtureEditorWidget::setTiltInvert);
    connect(m_impl->calibrateColorsButton, &QPushButton::clicked, this, &FixtureEditorWidget::openColorCalibration);
}

FixtureEditorWidget::~FixtureEditorWidget()
{
    delete m_impl;
}

void FixtureEditorWidget::setFixtures(QVector<Fixture*> t_fixtures)
{
    for (auto *fix : m_impl->fixtures) {
        disconnect(fix, &SceneObject::positionChanged, this, &FixtureEditorWidget::refreshTransform);
        disconnect(fix, &SceneObject::rotationChanged, this, &FixtureEditorWidget::refreshTransform);
        disconnect(fix, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);
    }
    m_impl->fixtures = t_fixtures;
    for (auto *fix : m_impl->fixtures) {
        connect(fix, &SceneObject::positionChanged, this, &FixtureEditorWidget::refreshTransform);
        connect(fix, &SceneObject::rotationChanged, this, &FixtureEditorWidget::refreshTransform);
        // Keeps the tag row live if a tag is added/removed from outside this
        // editor - e.g. dropped onto this fixture's row in the Project panel.
        connect(fix, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);
    }
    m_impl->modeCombo->clear();

    if(m_impl->fixtures.isEmpty())
    {
        m_impl->nameEdit->setText("");
        m_impl->nameEdit->setEnabled(false);
        m_impl->identifierEdit->setText("");
        m_impl->identifierEdit->setEnabled(false);
        m_impl->commentEdit->setText("");
        m_impl->commentEdit->setEnabled(false);
        m_impl->manufacturerLabel->setText("");
        m_impl->descriptionLabel->setText("");
        m_impl->universeSpin->setValue(0);
        m_impl->universeSpin->setEnabled(false);
        m_impl->offsetSpin->setValue(1);
        m_impl->offsetSpin->setEnabled(false);
        m_impl->modeCombo->setEnabled(false);
        m_impl->modelCombo->setEnabled(false);
        m_impl->beamCombo->setEnabled(false);
        m_impl->positionEdit->setEnabled(false);
        m_impl->rotationEdit->setEnabled(false);
        m_impl->panOffsetSpin->setValue(0.0);
        m_impl->panOffsetSpin->setEnabled(false);
        m_impl->tiltOffsetSpin->setValue(0.0);
        m_impl->tiltOffsetSpin->setEnabled(false);
        m_impl->panFlipCheck->setChecked(false);
        m_impl->panFlipCheck->setEnabled(false);
        m_impl->tiltFlipCheck->setChecked(false);
        m_impl->tiltFlipCheck->setEnabled(false);
        m_impl->panInvertCheck->setChecked(false);
        m_impl->panInvertCheck->setEnabled(false);
        m_impl->tiltInvertCheck->setChecked(false);
        m_impl->tiltInvertCheck->setEnabled(false);
        m_impl->calibrateColorsButton->setEnabled(false);
        m_impl->tagEditor->setEnabled(false);
        m_impl->tagEditor->refresh();
        return;
    }


    m_impl->nameEdit->setEnabled(true);
    m_impl->identifierEdit->setEnabled(true);
    m_impl->commentEdit->setEnabled(true);
    m_impl->universeSpin->setEnabled(true);
    m_impl->offsetSpin->setEnabled(true);
    m_impl->modeCombo->setEnabled(true);
    m_impl->modelCombo->setEnabled(true);
    m_impl->beamCombo->setEnabled(true);
    m_impl->positionEdit->setEnabled(true);
    m_impl->rotationEdit->setEnabled(true);
    m_impl->panOffsetSpin->setEnabled(true);
    m_impl->tiltOffsetSpin->setEnabled(true);
    m_impl->panFlipCheck->setEnabled(true);
    m_impl->tiltFlipCheck->setEnabled(true);
    m_impl->panInvertCheck->setEnabled(true);
    m_impl->tiltInvertCheck->setEnabled(true);
    m_impl->tagEditor->setEnabled(true);

    auto it = m_impl->fixtures.cbegin();
    Fixture *firstFixture = *it;

    m_impl->calibrateColorsButton->setEnabled(
        m_impl->fixtures.length() == 1 && !firstFixture->findCapability(Capability_Color).isEmpty());

    QString name = firstFixture->name();
    bool multiName = false;

    QString manufacturer = firstFixture->manufacturer();
    bool multiManufacturer = false;

    QString description = firstFixture->description();
    bool multiDescription = false;

    QString identifier = firstFixture->identifier();
    bool multiIdentifier = false;

    QString comment = firstFixture->comments();
    bool multiComment = false;

    int universe = firstFixture->universe();
    bool multiUniverse = false;

    int offset = firstFixture->dmxOffset()+1;
    bool multiOffset = false;

    bool multiMode = false;
    int mode = firstFixture->mode();
    auto modes = firstFixture->modes();

    QVector3D position = firstFixture->position();
    bool multiPosition = false;

    QVector3D rotation = firstFixture->rotation();
    bool multiRotation = false;

    float panOffset = firstFixture->panOffset();
    bool multiPanOffset = false;

    float tiltOffset = firstFixture->tiltOffset();
    bool multiTiltOffset = false;

    bool panFlip = firstFixture->panFlip();
    bool multiPanFlip = false;

    bool tiltFlip = firstFixture->tiltFlip();
    bool multiTiltFlip = false;

    bool panInvert = firstFixture->panInvert();
    bool multiPanInvert = false;

    bool tiltInvert = firstFixture->tiltInvert();
    bool multiTiltInvert = false;

    if(m_impl->fixtures.length() > 1)
    {
        for(++it; it != m_impl->fixtures.cend(); ++it)
        {
            auto currentFixture = *it;

            if(!multiName && currentFixture->name() != name)
            {
                name = "[multiple]";
                multiName = true;
            }

            if(!multiManufacturer && currentFixture->manufacturer() != manufacturer)
            {
                manufacturer = "[multiple]";
                multiManufacturer = true;
            }

            if(!multiDescription && currentFixture->description() != description)
            {
                description = "[multiple]";
                multiDescription = true;
            }

            if(!multiIdentifier && currentFixture->identifier() != identifier)
            {
                identifier = "[multiple]";
                multiIdentifier = true;
            }

            if(!multiComment && currentFixture->comments() != comment)
            {
                comment = "[multiple]";
                multiComment = true;
            }

            if(!multiUniverse && currentFixture->universe() != universe)
            {
                universe = 0;
                multiUniverse = true;
            }

            if(!multiOffset && currentFixture->dmxOffset() != offset)
            {
                offset = 1;
                multiOffset = true;
            }

            if(!multiMode && currentFixture->modes() != modes)
            {
                modes = QVector<FixtureMode>();
                multiMode = true;
            }

            if(!multiPosition && currentFixture->position() != position)
            {
                position = QVector3D();
                multiPosition = true;
            }

            if(!multiRotation && currentFixture->rotation() != rotation)
            {
                rotation = QVector3D();
                multiRotation = true;
            }

            if(!multiPanOffset && !qFuzzyCompare(currentFixture->panOffset() + 1.0f, panOffset + 1.0f))
            {
                panOffset = 0.0f;
                multiPanOffset = true;
            }

            if(!multiTiltOffset && !qFuzzyCompare(currentFixture->tiltOffset() + 1.0f, tiltOffset + 1.0f))
            {
                tiltOffset = 0.0f;
                multiTiltOffset = true;
            }

            if(!multiPanFlip && currentFixture->panFlip() != panFlip)
            {
                panFlip = false;
                multiPanFlip = true;
            }

            if(!multiTiltFlip && currentFixture->tiltFlip() != tiltFlip)
            {
                tiltFlip = false;
                multiTiltFlip = true;
            }

            if(!multiPanInvert && currentFixture->panInvert() != panInvert)
            {
                panInvert = false;
                multiPanInvert = true;
            }

            if(!multiTiltInvert && currentFixture->tiltInvert() != tiltInvert)
            {
                tiltInvert = false;
                multiTiltInvert = true;
            }
        }
    }

    m_impl->nameEdit->setText(name);
    m_impl->identifierEdit->setText(identifier);
    m_impl->commentEdit->setText(comment);
    m_impl->manufacturerLabel->setText(manufacturer);
    m_impl->descriptionLabel->setText(description);
    m_impl->universeSpin->setValue(universe);
    m_impl->offsetSpin->setValue(offset);
    m_impl->positionEdit->setValue(position);
    m_impl->rotationEdit->setValue(rotation);
    m_impl->panOffsetSpin->setValue(double(panOffset));
    m_impl->tiltOffsetSpin->setValue(double(tiltOffset));
    m_impl->panFlipCheck->setChecked(panFlip);
    m_impl->tiltFlipCheck->setChecked(tiltFlip);
    m_impl->panInvertCheck->setChecked(panInvert);
    m_impl->tiltInvertCheck->setChecked(tiltInvert);
    m_impl->tagEditor->refresh();

    for(const auto &mode : modes)
    {
        m_impl->modeCombo->addItem(mode.name + " (" + QString::number(mode.channels.length()) + ")");
    }

    m_impl->modeCombo->setCurrentIndex(mode);

    const QString modelType = firstFixture->modelType();
    const int modelIndex = modelType.isEmpty() ? 0 : qMax(0, m_impl->modelCombo->findText(modelType));
    m_impl->modelCombo->setCurrentIndex(modelIndex);

    const QString beamStyle = firstFixture->beamStyle();
    int beamIndex = 0;   // Auto
    if(beamStyle == "cones")
        beamIndex = 1;
    else if(beamStyle == "volumetric")
        beamIndex = 2;
    else if(beamStyle == "none")
        beamIndex = 3;
    m_impl->beamCombo->setCurrentIndex(beamIndex);
}

void FixtureEditorWidget::setName(const QString &name)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setName(name);
    }
}

void FixtureEditorWidget::setDefinition(const QString &path)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->loadFixtureDefinition(path);
    }
}

void FixtureEditorWidget::updateComments()
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setComments(m_impl->commentEdit->toPlainText());
    }
}

void FixtureEditorWidget::setIdentifier(const QString &t_identifier)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setIdentifier(t_identifier);
    }
}

void FixtureEditorWidget::setUniverse(uint t_universe)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setUniverse(t_universe);
    }
}

void FixtureEditorWidget::setOffset(uint t_channel)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setDMXOffset(t_channel-1);
    }
}


void FixtureEditorWidget::setMode(int t_index)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setMode(t_index);
    }
}

void FixtureEditorWidget::setModelType(int t_index)
{
    // Index 0 = Auto (empty override); others are the model-type token.
    const QString type = (t_index <= 0) ? QString() : m_impl->modelCombo->itemText(t_index);
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setModelType(type);
    }
}

void FixtureEditorWidget::setBeamStyle(int t_index)
{
    // Index 0 = Auto (empty override, follow the global toggle); 1 = cones,
    // 2 = volumetric, 3 = none (no beam rendered at all).
    QString style;
    if(t_index == 1)
        style = "cones";
    else if(t_index == 2)
        style = "volumetric";
    else if(t_index == 3)
        style = "none";
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setBeamStyle(style);
    }
}

void FixtureEditorWidget::setPosition(const QVector3D &t_position)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setPosition(t_position);
    }
}

void FixtureEditorWidget::setRotation(const QVector3D &t_rotation)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setRotation(t_rotation);
    }
}

void FixtureEditorWidget::setPanOffset(double t_offset)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setPanOffset(float(t_offset));
    }
}

void FixtureEditorWidget::setTiltOffset(double t_offset)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setTiltOffset(float(t_offset));
    }
}

void FixtureEditorWidget::setPanFlip(bool t_flip)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setPanFlip(t_flip);
    }
}

void FixtureEditorWidget::setTiltFlip(bool t_flip)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setTiltFlip(t_flip);
    }
}

void FixtureEditorWidget::setPanInvert(bool t_invert)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setPanInvert(t_invert);
    }
}

void FixtureEditorWidget::setTiltInvert(bool t_invert)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setTiltInvert(t_invert);
    }
}

void FixtureEditorWidget::refreshTransform()
{
    if (m_impl->fixtures.isEmpty())
        return;
    QSignalBlocker pb(m_impl->positionEdit);
    QSignalBlocker rb(m_impl->rotationEdit);
    m_impl->positionEdit->setValue(m_impl->fixtures.first()->position());
    m_impl->rotationEdit->setValue(m_impl->fixtures.first()->rotation());
}

void FixtureEditorWidget::openColorCalibration()
{
    if(m_impl->fixtures.length() != 1)
        return;

    ColorCalibrationDialog dialog(m_impl->fixtures.first(), this);
    dialog.exec();
}

} // namespace photon
