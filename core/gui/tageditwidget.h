#ifndef PHOTON_TAGEDITWIDGET_H
#define PHOTON_TAGEDITWIDGET_H

#include <QWidget>

namespace photon {

class TagEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TagEditWidget(QWidget *parent = nullptr);
    ~TagEditWidget();

    void setIsReadOnly(bool);
    bool isReadOnly() const;
    QSize sizeHint() const override;

public slots:
    void addTag(const QString &);
    void removeTag(const QString &);
    void setTags(const QStringList &);


signals:
    void tagAdded();
    void tagRemoved();
    void tagsChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_TAGEDITWIDGET_H
