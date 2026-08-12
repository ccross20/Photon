#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include "pixelarrangedialog.h"
#include "view/numberscrubfield.h"

namespace photon {

class LinearArrangeDialog::Impl
{
public:
    keira::NumberScrubField *length;
    keira::NumberScrubField *center;
    keira::NumberScrubField *angle;
};

LinearArrangeDialog::LinearArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Arrange as Linear");

    m_impl->length = new keira::NumberScrubField;
    m_impl->length->setRange(0, 10000);
    m_impl->length->setValue(.9);

    m_impl->center = new keira::NumberScrubField;
    m_impl->center->setRange(0, 1);
    m_impl->center->setValue(.5);

    m_impl->angle = new keira::NumberScrubField;
    m_impl->angle->setRange(-360, 360);
    m_impl->angle->setValue(0);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Length", m_impl->length);
    formLayout->addRow("Center", m_impl->center);
    formLayout->addRow("Angle", m_impl->angle);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

LinearArrangeDialog::~LinearArrangeDialog()
{
    delete m_impl;
}

double LinearArrangeDialog::length() const { return m_impl->length->value(); }
double LinearArrangeDialog::center() const { return m_impl->center->value(); }
double LinearArrangeDialog::angle() const { return m_impl->angle->value(); }


class GridArrangeDialog::Impl
{
public:
    keira::NumberScrubField *rows;
    keira::NumberScrubField *columns;
    keira::NumberScrubField *width;
    keira::NumberScrubField *height;
};

GridArrangeDialog::GridArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Arrange as Grid");

    m_impl->rows = new keira::NumberScrubField;
    m_impl->rows->setIsInteger(true);
    m_impl->rows->setRange(1, 1000);
    m_impl->rows->setValue(1);

    m_impl->columns = new keira::NumberScrubField;
    m_impl->columns->setIsInteger(true);
    m_impl->columns->setRange(1, 1000);
    m_impl->columns->setValue(1);

    m_impl->width = new keira::NumberScrubField;
    m_impl->width->setRange(0, 10000);
    m_impl->width->setValue(.9);

    m_impl->height = new keira::NumberScrubField;
    m_impl->height->setRange(0, 10000);
    m_impl->height->setValue(.9);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Rows", m_impl->rows);
    formLayout->addRow("Columns", m_impl->columns);
    formLayout->addRow("Width", m_impl->width);
    formLayout->addRow("Height", m_impl->height);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

GridArrangeDialog::~GridArrangeDialog()
{
    delete m_impl;
}

int GridArrangeDialog::rows() const { return static_cast<int>(m_impl->rows->value()); }
int GridArrangeDialog::columns() const { return static_cast<int>(m_impl->columns->value()); }
double GridArrangeDialog::width() const { return m_impl->width->value(); }
double GridArrangeDialog::height() const { return m_impl->height->value(); }


class RadialArrangeDialog::Impl
{
public:
    keira::NumberScrubField *radius;
};

RadialArrangeDialog::RadialArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Arrange as Radial");

    m_impl->radius = new keira::NumberScrubField;
    m_impl->radius->setRange(0, 10000);
    m_impl->radius->setValue(.45);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Radius", m_impl->radius);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

RadialArrangeDialog::~RadialArrangeDialog()
{
    delete m_impl;
}

double RadialArrangeDialog::radius() const { return m_impl->radius->value(); }


class ArcArrangeDialog::Impl
{
public:
    keira::NumberScrubField *radius;
    keira::NumberScrubField *startAngle;
    keira::NumberScrubField *sweepAngle;
};

ArcArrangeDialog::ArcArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Arrange as Arc");

    m_impl->radius = new keira::NumberScrubField;
    m_impl->radius->setRange(0, 10000);
    m_impl->radius->setValue(.45);

    m_impl->startAngle = new keira::NumberScrubField;
    m_impl->startAngle->setRange(-360, 360);
    m_impl->startAngle->setValue(0);

    m_impl->sweepAngle = new keira::NumberScrubField;
    m_impl->sweepAngle->setRange(-360, 360);
    m_impl->sweepAngle->setValue(180);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Radius", m_impl->radius);
    formLayout->addRow("Start Angle", m_impl->startAngle);
    formLayout->addRow("Sweep Angle", m_impl->sweepAngle);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

ArcArrangeDialog::~ArcArrangeDialog()
{
    delete m_impl;
}

double ArcArrangeDialog::radius() const { return m_impl->radius->value(); }
double ArcArrangeDialog::startAngle() const { return m_impl->startAngle->value(); }
double ArcArrangeDialog::sweepAngle() const { return m_impl->sweepAngle->value(); }


class HoneycombArrangeDialog::Impl
{
public:
    keira::NumberScrubField *rows;
    keira::NumberScrubField *columns;
    keira::NumberScrubField *spacing;
};

HoneycombArrangeDialog::HoneycombArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Arrange as Honeycomb");

    m_impl->rows = new keira::NumberScrubField;
    m_impl->rows->setIsInteger(true);
    m_impl->rows->setRange(1, 1000);
    m_impl->rows->setValue(1);

    m_impl->columns = new keira::NumberScrubField;
    m_impl->columns->setIsInteger(true);
    m_impl->columns->setRange(1, 1000);
    m_impl->columns->setValue(1);

    m_impl->spacing = new keira::NumberScrubField;
    m_impl->spacing->setRange(0, 10000);
    m_impl->spacing->setValue(.2);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Rows", m_impl->rows);
    formLayout->addRow("Columns", m_impl->columns);
    formLayout->addRow("Spacing", m_impl->spacing);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

HoneycombArrangeDialog::~HoneycombArrangeDialog()
{
    delete m_impl;
}

int HoneycombArrangeDialog::rows() const { return static_cast<int>(m_impl->rows->value()); }
int HoneycombArrangeDialog::columns() const { return static_cast<int>(m_impl->columns->value()); }
double HoneycombArrangeDialog::spacing() const { return m_impl->spacing->value(); }


class BeeEyeArrangeDialog::Impl
{
public:
    keira::NumberScrubField *spacing;
};

BeeEyeArrangeDialog::BeeEyeArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Arrange as Bee Eye");

    m_impl->spacing = new keira::NumberScrubField;
    m_impl->spacing->setRange(0, 10000);
    m_impl->spacing->setValue(.2);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Spacing", m_impl->spacing);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

BeeEyeArrangeDialog::~BeeEyeArrangeDialog()
{
    delete m_impl;
}

double BeeEyeArrangeDialog::spacing() const { return m_impl->spacing->value(); }


class MoveArrangeDialog::Impl
{
public:
    keira::NumberScrubField *dx;
    keira::NumberScrubField *dy;
};

MoveArrangeDialog::MoveArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Move");

    m_impl->dx = new keira::NumberScrubField;
    m_impl->dx->setRange(-10000, 10000);
    m_impl->dx->setValue(0);

    m_impl->dy = new keira::NumberScrubField;
    m_impl->dy->setRange(-10000, 10000);
    m_impl->dy->setValue(0);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("X", m_impl->dx);
    formLayout->addRow("Y", m_impl->dy);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

MoveArrangeDialog::~MoveArrangeDialog()
{
    delete m_impl;
}

double MoveArrangeDialog::dx() const { return m_impl->dx->value(); }
double MoveArrangeDialog::dy() const { return m_impl->dy->value(); }


class ScaleArrangeDialog::Impl
{
public:
    keira::NumberScrubField *scaleX;
    keira::NumberScrubField *scaleY;
};

ScaleArrangeDialog::ScaleArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Scale");

    m_impl->scaleX = new keira::NumberScrubField;
    m_impl->scaleX->setRange(-1000, 1000);
    m_impl->scaleX->setValue(1);

    m_impl->scaleY = new keira::NumberScrubField;
    m_impl->scaleY->setRange(-1000, 1000);
    m_impl->scaleY->setValue(1);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Scale X", m_impl->scaleX);
    formLayout->addRow("Scale Y", m_impl->scaleY);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

ScaleArrangeDialog::~ScaleArrangeDialog()
{
    delete m_impl;
}

double ScaleArrangeDialog::scaleX() const { return m_impl->scaleX->value(); }
double ScaleArrangeDialog::scaleY() const { return m_impl->scaleY->value(); }


class RotateArrangeDialog::Impl
{
public:
    keira::NumberScrubField *angle;
};

RotateArrangeDialog::RotateArrangeDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Rotate");

    m_impl->angle = new keira::NumberScrubField;
    m_impl->angle->setRange(-360, 360);
    m_impl->angle->setValue(0);

    auto *formLayout = new QFormLayout;
    formLayout->addRow("Angle", m_impl->angle);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

RotateArrangeDialog::~RotateArrangeDialog()
{
    delete m_impl;
}

double RotateArrangeDialog::angle() const { return m_impl->angle->value(); }

} // namespace photon
