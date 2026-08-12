#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QSignalBlocker>
#include "fixtureeditorwidget.h"
#include "fixture.h"
#include "scene/sceneobject.h"
#include "gui/vector3edit.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class FixtureEditorWidget::Impl
{
public:
    Impl();
    QVector<Fixture*> fixtures;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    QTextEdit *commentEdit;
    QLineEdit *identifierEdit;
    TagEditorWidget *tagEditor;
    QLabel *manufacturerLabel;
    QLabel *descriptionLabel;
    QSpinBox *universeSpin;
    QSpinBox *offsetSpin;
    QComboBox *modeCombo;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
    QComboBox *modelCombo;
    QComboBox *beamCombo;
};

FixtureEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);

    identifierEdit = new QLineEdit;
    formLayout->addRow("Identifier", identifierEdit);

    commentEdit = new QTextEdit;
    commentEdit->setMaximumHeight(70);
    commentEdit->setAcceptRichText(false);
    formLayout->addRow("Comment", commentEdit);

    manufacturerLabel = new QLabel;
    formLayout->addRow("Manufacturer", manufacturerLabel);

    descriptionLabel = new QLabel;
    formLayout->addRow("Description", descriptionLabel);

    universeSpin = new QSpinBox;
    universeSpin->setMinimum(1);
    universeSpin->setMaximum(9999);
    formLayout->addRow("Universe", universeSpin);

    offsetSpin = new QSpinBox;
    offsetSpin->setMinimum(1);
    offsetSpin->setMaximum(511);
    formLayout->addRow("Starting Channel", offsetSpin);

    modeCombo = new QComboBox;
    formLayout->addRow("DMX Mode", modeCombo);

    modelCombo = new QComboBox;
    // Index 0 = Auto (empty override); the rest are visualiser model types.
    modelCombo->addItems(QStringList() << "Auto" << "mover" << "par" << "uplight"
                                       << "strobe" << "blinder" << "bar" << "wash" << "beeeye");
    formLayout->addRow("Model", modelCombo);

    beamCombo = new QComboBox;
    // Index 0 = Auto (follow the visualiser's global beam toggle); 1 = basic
    // cone, 2 = volumetric. Tokens stored on the fixture are "", "cones", "volumetric".
    beamCombo->addItems(QStringList() << "Auto" << "Cones" << "Volumetric");
    formLayout->addRow("Beam Style", beamCombo);

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
    formLayout->addRow("Tags", tagEditor);

    positionEdit = new Vector3Edit;
    formLayout->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit;
    formLayout->addRow("Rotation", rotationEdit);

}

FixtureEditorWidget::FixtureEditorWidget(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    setLayout(m_impl->formLayout);

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
    m_impl->tagEditor->setEnabled(true);

    auto it = m_impl->fixtures.cbegin();
    Fixture *firstFixture = *it;

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
    // Index 0 = Auto (empty override, follow the global toggle); 1 = cones, 2 = volumetric.
    QString style;
    if(t_index == 1)
        style = "cones";
    else if(t_index == 2)
        style = "volumetric";
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

void FixtureEditorWidget::refreshTransform()
{
    if (m_impl->fixtures.isEmpty())
        return;
    QSignalBlocker pb(m_impl->positionEdit);
    QSignalBlocker rb(m_impl->rotationEdit);
    m_impl->positionEdit->setValue(m_impl->fixtures.first()->position());
    m_impl->rotationEdit->setValue(m_impl->fixtures.first()->rotation());
}

} // namespace photon
