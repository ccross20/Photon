#include <QImage>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QListWidget>
#include <QMenu>
#include <QOffscreenSurface>
#include "canvaslayergroup.h"
#include "pixel/canvas.h"
#include "sequence.h"
#include "project/project.h"
#include "pixel/canvascollection.h"
#include "pixel/pixelsource.h"
#include "pixel/pixellayout.h"
#include "scene/sceneiterator.h"
#include "scene/sceneobject.h"
#include "pixel/pixellayoutcollection.h"
#include "photoncore.h"
#include "opengl/openglframebuffer.h"
#include "cliplayer.h"
#include "canvasclip.h"
#include "canvaseffect.h"
#include "opengl/openglresources.h"

namespace photon {

class CanvasLayerGroupEditor::Impl
{
public:
    CanvasLayerGroup *canvasGroup;
    QListWidget *pixelLayoutList;
    QPushButton *addLayoutButton;
    QPushButton *removeLayoutButton;
    QComboBox *canvasCombo;
};

CanvasLayerGroupEditor::CanvasLayerGroupEditor(CanvasLayerGroup *t_group):QWidget(),m_impl(new Impl)
{
    m_impl->canvasGroup = t_group;

    connect(t_group, &CanvasLayerGroup::pixelLayoutAdded, this, &CanvasLayerGroupEditor::pixelLayoutAdded);
    connect(t_group, &CanvasLayerGroup::pixelLayoutRemoved, this, &CanvasLayerGroupEditor::pixelLayoutRemoved);


    QFormLayout *formLayout = new QFormLayout;

    int currentIndex = 0;
    int counter = 0;
    m_impl->canvasCombo = new QComboBox;
    for(auto canvas : photonApp->project()->canvases()->canvases())
    {
        m_impl->canvasCombo->addItem(canvas->name());
        if(t_group->canvas() == canvas)
            currentIndex = counter;
        counter++;
    }
    m_impl->canvasCombo->setCurrentIndex(currentIndex);

    formLayout->addRow("Canvas", m_impl->canvasCombo);


    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);

    m_impl->pixelLayoutList = new QListWidget;

    for(auto layout : t_group->pixelLayouts())
    {
        m_impl->pixelLayoutList->addItem(layout->name());
    }


    vLayout->addWidget(m_impl->pixelLayoutList);

    m_impl->addLayoutButton = new QPushButton("Add");
    connect(m_impl->addLayoutButton, &QPushButton::clicked,this, &CanvasLayerGroupEditor::openAddPixelLayout);

    m_impl->removeLayoutButton = new QPushButton("Remove");
    connect(m_impl->removeLayoutButton, &QPushButton::clicked,this, &CanvasLayerGroupEditor::removeSelectedLayout);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(m_impl->addLayoutButton);
    hLayout->addWidget(m_impl->removeLayoutButton);

    vLayout->addLayout(hLayout);
    formLayout->addRow("Pixel Layouts", vLayout);

    setLayout(formLayout);
}

void CanvasLayerGroupEditor::canvasChanged(int t_index)
{
    m_impl->canvasGroup->setCanvas(photonApp->project()->canvases()->canvases()[t_index]);
}

void CanvasLayerGroupEditor::openAddPixelLayout()
{
    QMenu menu;
    auto pixelLayouts = photonApp->project()->pixelLayouts()->layouts();
    for(auto layout : pixelLayouts)
    {
        if(!m_impl->canvasGroup->pixelLayouts().contains(layout))
            menu.addAction(layout->name(),[this, layout](){m_impl->canvasGroup->addPixelLayout(layout);});
    }
    menu.exec(m_impl->addLayoutButton->mapToGlobal(QPoint(0,m_impl->addLayoutButton->height())));
}

void CanvasLayerGroupEditor::removeSelectedLayout()
{
    QByteArrayList ids;
    for(auto item : m_impl->pixelLayoutList->selectedItems())
    {
        ids << item->data(Qt::ItemDataRole::UserRole).toByteArray();
    }

    for(const auto &id : ids)
    {
        for(auto layout : m_impl->canvasGroup->pixelLayouts())
        {
            if(layout->uniqueId() == id)
                m_impl->canvasGroup->removePixelLayout(layout);
        }
    }

}

void CanvasLayerGroupEditor::pixelLayoutAdded(photon::PixelLayout *t_layout)
{
    auto item = new QListWidgetItem(t_layout->name());
    item->setData(Qt::ItemDataRole::UserRole, t_layout->uniqueId());
    m_impl->pixelLayoutList->addItem(item);
}

void CanvasLayerGroupEditor::pixelLayoutRemoved(photon::PixelLayout *t_layout)
{
    for(int i = 0; i < m_impl->pixelLayoutList->count(); ++i)
    {
        auto item = m_impl->pixelLayoutList->item(i);
        if(item->data(Qt::ItemDataRole::UserRole).toByteArray() == t_layout->uniqueId())
        {
            delete item;
        }
    }
}




class CanvasLayerGroup::Impl
{
public:
    QVector<PixelLayout*> pixelLayouts;
    Canvas *canvas;
    QOpenGLContext *context = nullptr;
    QImage tempImage;
    int canvasIndex = -1;
};

CanvasLayerGroup::CanvasLayerGroup(QObject *t_parent):LayerGroup("CanvasGroup", t_parent),m_impl(new Impl)
{
    m_impl->canvas = nullptr;

    QOffscreenSurface *surface = photonApp->surface();

    m_impl->context = new QOpenGLContext();

    m_impl->context->setShareContext(QOpenGLContext::globalShareContext());
    m_impl->context->create();
    m_impl->context->makeCurrent(surface);
}

CanvasLayerGroup::CanvasLayerGroup(Canvas *t_canvas, const QString &t_name):LayerGroup(t_name,"CanvasGroup"),m_impl(new Impl)
{
    m_impl->canvas = t_canvas;

    QOffscreenSurface *surface = photonApp->surface();

    m_impl->context = new QOpenGLContext();

    m_impl->context->setShareContext(QOpenGLContext::globalShareContext());
    m_impl->context->create();
    m_impl->context->makeCurrent(surface);
}

CanvasLayerGroup::~CanvasLayerGroup()
{
    delete m_impl;
}

QOpenGLContext *CanvasLayerGroup::openGLContext() const
{
    return m_impl->context;
}

void CanvasLayerGroup::setCanvas(Canvas *t_canvas)
{
    if(m_impl->canvas)
        disconnect(m_impl->canvas, &Canvas::sizeUpdated,this, &CanvasLayerGroup::canvasSizeUpdated);
    m_impl->canvas = t_canvas;
    if(t_canvas)
    {
        connect(t_canvas, &Canvas::sizeUpdated,this, &CanvasLayerGroup::canvasSizeUpdated);

        m_impl->context->makeCurrent(photonApp->surface());

        for(auto layer : layers())
        {
            auto clipLayer = dynamic_cast<ClipLayer*>(layer);

            if(clipLayer)
            {
                for(auto clip : clipLayer->clips())
                {
                    auto canvasClip = dynamic_cast<CanvasClip*>(clip);
                    if(canvasClip)
                        canvasClip->initializeContext(m_impl->context, m_impl->canvas);
                }
            }
        }
    }
}

Canvas *CanvasLayerGroup::canvas() const
{
    return m_impl->canvas;
}

void CanvasLayerGroup::canvasSizeUpdated(QSize t_size)
{
    m_impl->context->makeCurrent(photonApp->surface());

    for(auto layer : layers())
    {
        auto clipLayer = dynamic_cast<ClipLayer*>(layer);

        if(clipLayer)
        {
            for(auto clip : clipLayer->clips())
            {
                auto canvasClip = dynamic_cast<CanvasClip*>(clip);
                if(canvasClip)
                    canvasClip->canvasResized(m_impl->context, m_impl->canvas);
            }
        }
    }
}

void CanvasLayerGroup::processChannels(ProcessContext &t_context)
{
    if(!m_impl->canvas->texture())
    {
        qDebug() << "No texture";
        return;
    }


    t_context.canvas = m_impl->canvas;
    m_impl->context->makeCurrent(photonApp->surface());
    t_context.openglContext = m_impl->context;
    OpenGLFrameBuffer buffer(m_impl->canvas->texture(), m_impl->context);
    t_context.frameBuffer = &buffer;
    t_context.resources = photonApp->openGLResources();
    t_context.resources->bind(t_context.openglContext);

    auto f = m_impl->context->functions();

    f->glViewport(0,0,m_impl->canvas->width(), m_impl->canvas->height());
    f->glClearColor(0.0f,0.0f,0.0f,0.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);
    f->glEnable(GL_BLEND);
    f->glBlendEquation (GL_FUNC_ADD);
    //f->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    /*
    QImage image(m_impl->canvas->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    t_context.previousCanvasImage = &m_impl->tempImage;
    t_context.canvasImage = &image;
*/
    LayerGroup::processChannels(t_context);

    //context.doneCurrent();

    if(!m_impl->pixelLayouts.isEmpty())
    {
        QImage image(m_impl->canvas->width(), m_impl->canvas->height(),QImage::Format_ARGB32_Premultiplied);
        buffer.writeToRaster(&image);

        t_context.image = &image;

        for(auto pixelLayout : m_impl->pixelLayouts)
            pixelLayout->process(t_context);
    }

    buffer.destroy();
    m_impl->canvas->updateTexture();
    m_impl->context->doneCurrent();

}

void CanvasLayerGroup::addPixelLayout(PixelLayout *t_layout)
{
    if(m_impl->pixelLayouts.contains(t_layout))
        return;
    m_impl->pixelLayouts << t_layout;
    emit pixelLayoutAdded(t_layout);
}

void CanvasLayerGroup::removePixelLayout(PixelLayout *t_layout)
{
    if(m_impl->pixelLayouts.removeOne(t_layout))
        emit pixelLayoutRemoved(t_layout);
}

PixelLayout *CanvasLayerGroup::pixelLayoutAtIndex(int t_index) const
{
    return m_impl->pixelLayouts[t_index];
}

int CanvasLayerGroup::pixelLayoutCount() const
{
    return m_impl->pixelLayouts.length();
}

const QVector<PixelLayout*> &CanvasLayerGroup::pixelLayouts() const
{
    return m_impl->pixelLayouts;
}

QWidget *CanvasLayerGroup::createEditor()
{
    return new CanvasLayerGroupEditor(this);
}

QVector<PixelSource*> CanvasLayerGroup::sources() const
{
    QVector<PixelSource*> results;

    if(m_impl->pixelLayouts.isEmpty())
    {
        auto sources = SceneIterator::FindMany(photonApp->project()->sceneRoot(),[](SceneObject *obj, bool *keepGoing){
            *keepGoing = true;
            return dynamic_cast<PixelSource*>(obj);
        });

        for(auto src : sources)
            results << dynamic_cast<PixelSource*>(src);
    }
    else
    {
        for(auto pixelLayout : m_impl->pixelLayouts)
            results << pixelLayout->sources();
    }

    return results;
}

void CanvasLayerGroup::restore(Project &t_project)
{
    if(m_impl->canvasIndex >= 0)
        setCanvas(t_project.canvases()->canvasAtIndex(m_impl->canvasIndex));
    LayerGroup::restore(t_project);
}

void CanvasLayerGroup::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    LayerGroup::readFromJson(t_json, t_context);

    m_impl->canvasIndex = t_json.value("canvasIndex").toInt(-1);


    if(t_json.contains("pixelLayouts"))
    {
        auto pixelLayoutArray = t_json.value("pixelLayouts").toArray();
        for(auto layoutObj : pixelLayoutArray)
        {
            auto layout = t_context.project->pixelLayouts()->findLayoutWithId(layoutObj.toString().toLatin1());

            if(layout)
                m_impl->pixelLayouts.append(layout);
        }
    }
}

void CanvasLayerGroup::writeToJson(QJsonObject &t_json) const
{
    LayerGroup::writeToJson(t_json);

    if(m_impl->canvas)
        t_json.insert("canvasIndex", 0);
    else
        t_json.insert("canvasIndex", -1);

    QJsonArray pixelLayoutArray;
    for(auto pl : m_impl->pixelLayouts)
    {
        pixelLayoutArray.append(QString{pl->uniqueId()});
    }

    t_json.insert("pixelLayouts", pixelLayoutArray);
}

} // namespace photon
