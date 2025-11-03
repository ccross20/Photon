
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include "pixellayouteditor_p.h"
#include "pixel/pixelsourcelayout.h"
#include "pixel/pixellayout.h"
#include "pixel/pixelsource.h"
#include "scene/sceneobject.h"
#include "scene/sceneiterator.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/capability/fixturecapability.h"
#include "pixel/fixturepixelsource.h"

namespace photon {

PixelLayoutPropertyEditor::PixelLayoutPropertyEditor()
{
    QFormLayout *formLayout = new QFormLayout;

    m_positionEdit = new PointEdit;
    m_scaleEdit = new PointEdit;
    m_rotationSpin = new QDoubleSpinBox;
    m_rotationSpin->setMinimum(-10000);
    m_rotationSpin->setMaximum(10000);

    m_positionEdit->setEnabled(false);
    m_scaleEdit->setEnabled(false);
    m_rotationSpin->setEnabled(false);

    formLayout->addRow("Position",m_positionEdit);
    formLayout->addRow("Scale",m_scaleEdit);
    formLayout->addRow("Rotation",m_rotationSpin);

    setLayout(formLayout);

    connect(m_positionEdit, &PointEdit::valueChanged,this, &PixelLayoutPropertyEditor::positionUpdated);
    connect(m_scaleEdit, &PointEdit::valueChanged,this, &PixelLayoutPropertyEditor::scaleUpdated);
    connect(m_rotationSpin, &QDoubleSpinBox::valueChanged,this, &PixelLayoutPropertyEditor::rotationUpdated);
}

void PixelLayoutPropertyEditor::selectLayout(PixelSourceLayout* t_layout)
{
    m_layout = t_layout;

    m_positionEdit->blockSignals(true);
    m_scaleEdit->blockSignals(true);
    m_rotationSpin->blockSignals(true);

    if(m_layout)
    {
        m_positionEdit->setEnabled(true);
        m_scaleEdit->setEnabled(true);
        m_rotationSpin->setEnabled(true);
        m_positionEdit->setValue(m_layout->position());
        m_scaleEdit->setValue(m_layout->scale());
        m_rotationSpin->setValue(m_layout->rotation());
    }
    else
    {
        m_positionEdit->setEnabled(false);
        m_scaleEdit->setEnabled(false);
        m_rotationSpin->setEnabled(false);
    }


    m_positionEdit->blockSignals(false);
    m_scaleEdit->blockSignals(false);
    m_rotationSpin->blockSignals(false);
}

void PixelLayoutPropertyEditor::positionUpdated(const QPointF t_value)
{
    m_layout->setPosition(t_value);
}

void PixelLayoutPropertyEditor::scaleUpdated(const QPointF t_value)
{
    m_layout->setScale(t_value);
}

void PixelLayoutPropertyEditor::rotationUpdated(double t_value)
{
    m_layout->setRotation(t_value);
}

PixelLayoutScene::PixelLayoutScene(PixelLayout *t_layout) : QGraphicsScene(),m_layout(t_layout)
{
    for(auto source : m_layout->sourceLayouts())
    {
        sourceAdded(source);
    }

    connect(m_layout, &PixelLayout::sourceWasAdded, this, &PixelLayoutScene::sourceAdded);
    connect(m_layout, &PixelLayout::sourceWillBeRemoved, this, &PixelLayoutScene::sourceRemoved);
}

PixelSourceItem *PixelLayoutScene::findItemForSource(PixelSourceLayout *t_source) const
{
    for(auto item : m_sourceItems)
    {
        if(item->sourceLayout() == t_source)
        {
            return item;
        }
    }
    return nullptr;
}

void PixelLayoutScene::sourceAdded(photon::PixelSourceLayout *t_source)
{
    auto item = new PixelSourceItem(t_source);

    m_sourceItems << item;
    addItem(item);

    connect(t_source, &PixelSourceLayout::transformUpdated, this, &PixelLayoutScene::sourceTransformUpdated);
}

void PixelLayoutScene::sourceRemoved(photon::PixelSourceLayout *t_source)
{
    auto item = findItemForSource(t_source);
    if(item)
    {
        m_sourceItems.removeOne(item);
        removeItem(item);
        connect(t_source, &PixelSourceLayout::transformUpdated, this, &PixelLayoutScene::sourceTransformUpdated);
    }
}

void PixelLayoutScene::setScale(QPointF t_scale)
{
    m_scale = t_scale;
    m_inverseScale = QPointF{1.0/m_scale.x(), 1.0/m_scale.y()};

    for(auto item : m_sourceItems)
        item->setScale(m_scale);
}

void PixelLayoutScene::sourceTransformUpdated()
{
    for(auto src : m_sourceItems)
    {
        src->transformUpdated();
    }
}

void PixelLayoutScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);
    painter->fillRect(rect, Qt::red);
}

PixelLayoutView::PixelLayoutView(): QGraphicsView()
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
void PixelLayoutView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}


void PixelLayoutView::resizeEvent(QResizeEvent *t_event)
{
    QWidget::resizeEvent(t_event);

    double w = t_event->size().width();
    double h = t_event->size().height();
    //view->setSceneRect(QRect(0,0,w,h));
    scene()->setSceneRect(QRect(0,0,w,h));
    static_cast<PixelLayoutScene*>(scene())->setScale(QPointF{w,h});

    QTransform t;
    t.translate(-w/2.0,-h/2.0);
    //t.scale(w,h);
    //t.translate(-w/2.0,-h/2.0);
    translate(-w/2.0,-h/2.0);

    //view->setTransform(t);
}


PixelSourceItem::PixelSourceItem(PixelSourceLayout *t_sourceLayout) : QGraphicsItem(),m_layout(t_sourceLayout)
{
    transformUpdated();
}

void PixelSourceItem::setScale(QPointF t_scale)
{
    m_scale = t_scale;
    //m_layout->setScale(t_scale);
    m_inverseScale = QPointF{1.0/m_scale.x(), 1.0/m_scale.y()};
    transformUpdated();
}

void PixelSourceItem::transformUpdated()
{
    prepareGeometryChange();
    setPos(m_layout->position().x() * m_scale.x(), m_layout->position().y() * m_scale.y());
    setRotation(m_layout->rotation());

}

QRectF PixelSourceItem::boundingRect() const
{
    QTransform t;
    t.scale(m_scale.x(), m_scale.y());
    return t.mapRect(QRectF{m_layout->localBounds()}).adjusted(-3,-3 ,3 ,3);
}

void PixelSourceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillRect(boundingRect(), Qt::cyan);

    QPointF s = m_layout->scale();

    for(auto pt : m_layout->source()->positions())
    {
        //qDebug() << pt;
        painter->fillRect(QRectF{QPointF{(pt.x()* m_scale.x() * s.x()) - 2.0,(pt.y() * m_scale.y() * s.y())-2.0}, QSizeF{4,4}}, Qt::black);
    }
}

PixelLayoutEditorSidePanel::PixelLayoutEditorSidePanel(PixelLayout *t_layout) : QWidget(),pixelLayout(t_layout)
{
    scene = new PixelLayoutScene(t_layout);

    view = new PixelLayoutView();
    view->setScene(scene);
    view->setFrameRect(QRect(0,0,1,1));
    //scene->setScale(500);
    //view->setTransform(QTransform::fromScale(500,500));
    scene->setSceneRect(QRect(0,0,1,1));

    addButton = new QPushButton("Add");
    layoutList = new QListWidget;
    propertyEditor = new PixelLayoutPropertyEditor;

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(addButton);
    vLayout->addWidget(layoutList);
    vLayout->addWidget(propertyEditor);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(view);
    hLayout->addLayout(vLayout);


    setLayout(hLayout);

    connect(layoutList, &QListWidget::currentRowChanged, this, &PixelLayoutEditorSidePanel::selectedRow);
    connect(addButton, &QPushButton::clicked, this, &PixelLayoutEditorSidePanel::addClicked);

    auto sources = pixelLayout->sources();
    for(auto src : sources)
    {

        auto sourceObj = dynamic_cast<SceneObject*>(src);
        if(dynamic_cast<FixturePixelSource*>(src))
        {
            layoutList->addItem(static_cast<FixturePixelSource*>(src)->fixture()->name());
        }
        else if(sourceObj)
            layoutList->addItem(src ? sourceObj->name() : "Unnamed");
    }

}

void PixelLayoutEditorSidePanel::selectedRow(int t_row)
{
    if(t_row < pixelLayout->sourceLayouts().length())
        propertyEditor->selectLayout(pixelLayout->sourceLayouts()[t_row]);
    else
        propertyEditor->selectLayout(nullptr);
}
void PixelLayoutEditorSidePanel::addClicked()
{
    auto sources = SceneIterator::FindMany(photonApp->project()->sceneRoot(),[](SceneObject *obj, bool *keepGoing){
            *keepGoing = true;
        auto fixture = dynamic_cast<Fixture*>(obj);

        if(fixture)
        {
            if(!fixture->findCapability(Capability_Color).isEmpty()){
                    return dynamic_cast<SceneObject*>(obj);
                }
        }
        else
            {
            if(dynamic_cast<PixelSource*>(obj))
                return dynamic_cast<SceneObject*>(obj);
        }
        return static_cast<SceneObject*>(nullptr);

    });


    auto currentSources = pixelLayout->sources();

    QMenu menu;
    for(auto src : sources)
    {
        auto action = menu.addAction(src->name(),[src, this](){
            auto fixture = dynamic_cast<Fixture*>(src);

            if(fixture)
            {
                if(!fixture->findCapability(Capability_Color).isEmpty()){
                    auto capabilities = fixture->findCapability(Capability_Color);
                    addSource(new FixturePixelSource(capabilities));
                }
            }
            else
                addSource(dynamic_cast<PixelSource*>(src));
        });
        action->setEnabled(!currentSources.contains(dynamic_cast<PixelSource*>(src)));
    }

    menu.exec(addButton->mapToGlobal(QPoint{}));

}

void PixelLayoutEditorSidePanel::addSource(photon::PixelSource *t_source)
{
    auto sourceLayout = new PixelSourceLayout(t_source);
    pixelLayout->addSource(sourceLayout);

    auto sourceObj = dynamic_cast<SceneObject*>(t_source);

    if(dynamic_cast<FixturePixelSource*>(t_source))
    {
        layoutList->addItem(static_cast<FixturePixelSource*>(t_source)->fixture()->name());
    }
    else
        layoutList->addItem(sourceObj ? sourceObj->name() : "Unnamed");
}

PixelLayoutEditor::PixelLayoutEditor(PixelLayout *t_layout, QWidget *t_parent) : QWidget(t_parent),m_impl(new Impl)
{
    m_impl->sidePanel = new PixelLayoutEditorSidePanel(t_layout);


    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(m_impl->sidePanel);

    setLayout(hLayout);
}

PixelLayoutEditor::~PixelLayoutEditor()
{
    delete m_impl;
}

PixelLayout *PixelLayoutEditor::pixelLayout() const
{
    return m_impl->pixelLayout;
}

} // namespace photon
