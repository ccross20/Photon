#include "canvasclip.h"
#include "scene/sceneiterator.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/pixelsource.h"
#include "pixel/pixellayout.h"
#include "scene/sceneobject.h"
#include "pixel/pixellayoutcollection.h"

namespace photon {

class CanvasClip::Impl
{
public:
    QVector<PixelLayout*> pixelLayouts;
};

CanvasClip::CanvasClip(): Clip(),m_impl(new Impl)
{
    setId("canvasclip");
}

CanvasClip::CanvasClip(double t_start, double t_duration, QObject *t_parent) : Clip(t_start, t_duration, t_parent),m_impl(new Impl)
{    
    setId("canvasclip");
}

CanvasClip::~CanvasClip()
{
    delete m_impl;
}

void CanvasClip::processChannels(ProcessContext &t_context)
{
    Clip::processChannels(t_context);

    for(auto pixelLayout : m_impl->pixelLayouts)
        pixelLayout->process(t_context);
}

void CanvasClip::addPixelLayout(PixelLayout *t_layout)
{
    if(m_impl->pixelLayouts.contains(t_layout))
        return;
    m_impl->pixelLayouts << t_layout;
    emit pixelLayoutAdded(t_layout);
}

void CanvasClip::removePixelLayout(PixelLayout *t_layout)
{
    if(m_impl->pixelLayouts.removeOne(t_layout))
        emit pixelLayoutRemoved(t_layout);
}

PixelLayout *CanvasClip::pixelLayoutAtIndex(int t_index) const
{
    return m_impl->pixelLayouts[t_index];
}

int CanvasClip::pixelLayoutCount() const
{
    return m_impl->pixelLayouts.length();
}

QVector<PixelSource*> CanvasClip::sources() const
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

void CanvasClip::restore(Project &t_project)
{
    Clip::restore(t_project);
}

void CanvasClip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Clip::readFromJson(t_json, t_context);

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

void CanvasClip::writeToJson(QJsonObject &t_json) const
{
    Clip::writeToJson(t_json);

    QJsonArray pixelLayoutArray;
    for(auto pl : m_impl->pixelLayouts)
    {
        pixelLayoutArray.append(QString{pl->uniqueId()});
    }

    t_json.insert("pixelLayouts", pixelLayoutArray);
}


ClipInformation CanvasClip::info()
{
    ClipInformation toReturn([](){return new CanvasClip;});
    toReturn.name = "CanvasClip";
    toReturn.id = "canvasclip";
    //toReturn.categories.append("Generator");

    return toReturn;
}

} // namespace photon
