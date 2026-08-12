#ifndef PHOTON_PIXELARRANGEDIALOG_H
#define PHOTON_PIXELARRANGEDIALOG_H

#include <QDialog>
#include "photon-global.h"

namespace photon {

// Small parameter dialogs for the Pixel Layout editor's "Arrange" commands -
// one shape, one dialog, matching this codebase's existing widget-
// construction style (QFormLayout of keira::NumberScrubFields) rather than a
// single unified shape-picker dialog.
class PHOTONCORE_EXPORT LinearArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LinearArrangeDialog(QWidget *parent = nullptr);
    ~LinearArrangeDialog();

    double length() const;
    double center() const;
    double angle() const;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT GridArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GridArrangeDialog(QWidget *parent = nullptr);
    ~GridArrangeDialog();

    int rows() const;
    int columns() const;
    double width() const;
    double height() const;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT RadialArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RadialArrangeDialog(QWidget *parent = nullptr);
    ~RadialArrangeDialog();

    double radius() const;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT ArcArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ArcArrangeDialog(QWidget *parent = nullptr);
    ~ArcArrangeDialog();

    double radius() const;
    double startAngle() const;
    double sweepAngle() const;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT HoneycombArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HoneycombArrangeDialog(QWidget *parent = nullptr);
    ~HoneycombArrangeDialog();

    int rows() const;
    int columns() const;
    double spacing() const;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT BeeEyeArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BeeEyeArrangeDialog(QWidget *parent = nullptr);
    ~BeeEyeArrangeDialog();

    double spacing() const;

private:
    class Impl;
    Impl *m_impl;
};

// Transform commands - unlike Linear/Grid (which replace the selection with
// a freshly-generated shape), these apply directly to each selected pixel's
// current position.
class PHOTONCORE_EXPORT MoveArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MoveArrangeDialog(QWidget *parent = nullptr);
    ~MoveArrangeDialog();

    double dx() const;
    double dy() const;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT ScaleArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ScaleArrangeDialog(QWidget *parent = nullptr);
    ~ScaleArrangeDialog();

    double scaleX() const;
    double scaleY() const;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT RotateArrangeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RotateArrangeDialog(QWidget *parent = nullptr);
    ~RotateArrangeDialog();

    double angle() const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELARRANGEDIALOG_H
