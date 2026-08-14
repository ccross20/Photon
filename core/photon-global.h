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
#include "data/types.h"

namespace photon
{

enum Axis {
    Axis_X = 0x1,
    Axis_Y = 0x2,
    Axis_Z = 0x4
};

enum SurfaceMode{
    Mode_Performance,
    Mode_Edit
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
using ColorPalette = QVector<QColor>;

class AudioProcessor;
class BusEvaluator;
class CueLayer;
class SongData;
class FeatureTrack;
class BeatGrid;
class BusGraph;
class DMXMatrix;
class DMXTimeMachine;
class Canvas;
class CanvasClip;
class CanvasEffect;
class CanvasLayerGroup;
class Channel;
class ChannelEffect;
class ChannelEffectEditor;
class ChannelParameterContainer;
class ChannelParameter;
class ChannelParameterView;
class Clip;
class ClipLayer;
class Fixture;
class FixtureChannel;
class FixtureCapability;
class FixtureClip;
class FixtureCollection;
class FixtureGroup;
class FixtureGroupCollection;
class FixtureQuery;
class ProjectFolder;
class ProjectModel;
class ProjectResource;
class ProjectResourceNotifier;
class FixtureMask;
class FixtureWheel;
class FixtureWheelSlot;
class FixtureListParameter;
class GuiManager;
class Layer;
class LayerGroup;
class MasterLayer;
class OpenGLShader;
class OpenGLPlane;
class OpenGLTexture;
class OpenGLResources;
class RhiContext;
class Panel;
class PatchBay;
class PhotonCore;
class PixelLayout;
class PixelLayoutCollection;
class PixelListParameter;
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
class Surface;
class SurfaceCollection;
class SurfaceGizmo;
class SurfaceGizmoContainer;
class ContainerGizmo;
class SurfaceGraph;
class SurfaceView;
class State;
class SongData;
class SongLibrary;
class StateCapability;
class Timekeeper;
class Timeline;
class VirtualDJConnector;

struct LoadContext
{
    Project *project;

};


}

#define photonApp (static_cast<PhotonCore *>(QCoreApplication::instance()))

#endif // PHOTONGLOBAL_H
