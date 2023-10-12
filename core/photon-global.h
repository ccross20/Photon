#ifndef PHOTONGLOBAL_H
#define PHOTONGLOBAL_H

#include <QtCore/qglobal.h>
#include <QDebug>
#include <QVariant>
#include <QPointF>
#include <QVector3D>
#include <QUuid>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>

namespace photon
{

enum Axis {
    Axis_X = 0x1,
    Axis_Y = 0x2,
    Axis_Z = 0x4
};



#if defined(PHOTONCORE_LIBRARY)
#  define PHOTONCORE_EXPORT Q_DECL_EXPORT
#else
#  define PHOTONCORE_EXPORT Q_DECL_IMPORT
#endif

using PanelId = QByteArray;
using PluginId = QByteArray;
using SettingsPageId = QByteArray;
using CategoryList = QVector<QByteArray>;

class AudioProcessor;
class BeatLayer;
class BusEvaluator;
class BusGraph;
class DMXMatrix;
class Canvas;
class CanvasClip;
class CanvasCollection;
class CanvasEffect;
class CanvasLayerGroup;
class Channel;
class ChannelEffect;
class ChannelEffectEditor;
class ChannelParameter;
class ChannelParameterView;
class Clip;
class ClipEffect;
class ClipLayer;
class FalloffEffect;
class Fixture;
class FixtureChannel;
class FixtureCapability;
class FixtureClip;
class FixtureCollection;
class FixtureFalloffMap;
class FixtureMask;
class FixtureWheel;
class FixtureWheelSlot;
class GuiManager;
class Layer;
class LayerGroup;
class MaskEffect;
class MasterLayer;
class Panel;
class PatchBay;
class PhotonCore;
class PixelLayout;
class PixelLayoutCollection;
class PixelSource;
class PixelSourceLayout;
class PluginFactory;
class ProcessContext;
class Project;
class Resource;
class ResourceManager;
class Routine;
class RoutineCollection;
class SceneObject;
class Sequence;
class SequenceCollection;
class Settings;
class SettingsPage;
class State;
class StateCapability;
class Timekeeper;
class Timeline;

struct LoadContext
{
    Project *project;

};


}

#define photonApp (static_cast<PhotonCore *>(QCoreApplication::instance()))

#endif // PHOTONGLOBAL_H
