#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include "fixtureeditorwidget.h"
#include "fixture.h"
#include "gui/vector3edit.h"

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
    QLabel *manufacturerLabel;
    QLabel *descriptionLabel;
    QSpinBox *universeSpin;
    QSpinBox *offsetSpin;
    QComboBox *modeCombo;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
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
    offsetSpin->setMinimum(0);
    offsetSpin->setMaximum(511);
    formLayout->addRow("Offset", offsetSpin);

    modeCombo = new QComboBox;
    formLayout->addRow("DMX Mode", modeCombo);

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
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &FixtureEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &FixtureEditorWidget::setRotation);
}

FixtureEditorWidget::~FixtureEditorWidget()
{
    delete m_impl;
}

void FixtureEditorWidget::setFixtures(QVector<Fixture*> t_fixtures)
{
    m_impl->fixtures = t_fixtures;
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
        m_impl->offsetSpin->setValue(0);
        m_impl->offsetSpin->setEnabled(false);
        m_impl->modeCombo->setEnabled(false);
        m_impl->positionEdit->setEnabled(false);
        m_impl->rotationEdit->setEnabled(false);
        return;
    }


    m_impl->nameEdit->setEnabled(true);
    m_impl->identifierEdit->setEnabled(true);
    m_impl->commentEdit->setEnabled(true);
    m_impl->universeSpin->setEnabled(true);
    m_impl->offsetSpin->setEnabled(true);
    m_impl->modeCombo->setEnabled(true);
    m_impl->positionEdit->setEnabled(true);
    m_impl->rotationEdit->setEnabled(true);

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

    int offset = firstFixture->dmxOffset();
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
                offset = 0;
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

    for(const auto &mode : modes)
    {
        m_impl->modeCombo->addItem(mode.name + " (" + QString::number(mode.channels.length()) + ")");
    }

    m_impl->modeCombo->setCurrentIndex(mode);
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
        fixture->setDMXOffset(t_channel);
    }
}

void FixtureEditorWidget::setMode(int t_index)
{
    for(auto fixture : m_impl->fixtures)
    {
        fixture->setMode(t_index);
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


} // namespace photon
