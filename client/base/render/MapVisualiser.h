#ifndef MAP_VISUALISER_H
#define MAP_VISUALISER_H

#include "MapItem.h"
#include "MapObjectItem.h"
#include "ObjectGroupItem.h"
#include "MapMark.h"

#include "../../../general/base/GeneralStructures.h"
#include "../../../general/base/CommonMap.h"
#include "../../../client/base/Map_client.h"
#include "../../../client/base/DisplayStructures.h"
#include "../../../general/base/Map_loader.h"

#include <QGraphicsView>
#include <QGraphicsSimpleTextItem>
#include <QTimer>
#include <QList>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QRectF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QSet>
#include <QString>
#include <QMultiMap>
#include <QHash>
#include <QTime>
#include <QDateTime>
//#include <QGLWidget>

#include "MapVisualiserThread.h"

class MapVisualiser : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MapVisualiser(const bool &debugTags=false,const bool &useCache=true,const bool &OpenGL=false);
    ~MapVisualiser();

    bool showFPS();
    void setShowFPS(const bool &showFPS);
    void setTargetFPS(int targetFPS);
    void setOpenGl(const bool &OpenGL);
    virtual void eventOnMap(CatchChallenger::MapEvent event,MapVisualiserThread::Map_full * tempMapObject,quint8 x,quint8 y);

    MapVisualiserThread::Map_full * getMap(QString map);

    QString current_map;
    QHash<QString,MapVisualiserThread::Map_full *> all_map,old_all_map;
    QHash<QString,QDateTime> old_all_map_time;
protected:
    Tiled::MapReader reader;
    QGraphicsScene *mScene;
    MapItem* mapItem;

    Tiled::Tileset * markPathFinding;
    int tagTilesetIndex;

    bool debugTags;
    QString mLastError;

    quint8 waitRenderTime;
    QTimer timerRender;
    QTime timeRender;
    quint16 frameCounter;
    QTimer timerUpdateFPS;
    QTime timeUpdateFPS;
    QGraphicsSimpleTextItem *FPSText;
    bool mShowFPS;

    Tiled::Layer *grass;
    Tiled::Layer *grassOver;
    MapMark *mark;

    MapVisualiserThread mapVisualiserThread;
    QStringList asyncMap;
    QHash<quint8/*intervale*/,QTimer *> animationTimer;
    QHash<quint8/*intervale*/,QHash<quint8/*frame total*/,quint8/*actual frame*/> > animationFrame;

    virtual void destroyMap(MapVisualiserThread::Map_full *map);
protected slots:
    virtual void resetAll();
public slots:
    void loadOtherMap(const QString &resolvedFileName);
    virtual void loadBotOnTheMap(MapVisualiserThread::Map_full *parsedMap, const quint32 &botId, const quint8 &x, const quint8 &y, const QString &lookAt, const QString &skin);
    //virtual QSet<QString> loadMap(MapVisualiserThread::Map_full *map, const bool &display);
    virtual void removeUnusedMap();
    virtual void hideNotloadedMap();
private slots:
    void loadTeleporter(MapVisualiserThread::Map_full *map);
    void paintEvent(QPaintEvent * event);
    void updateFPS();
    void asyncDetectBorder(MapVisualiserThread::Map_full * tempMapObject);
    void applyTheAnimationTimer();
protected slots:
    void render();
    virtual bool asyncMapLoaded(const QString &fileName,MapVisualiserThread::Map_full * tempMapObject);
    void passMapIntoOld();
signals:
    void loadOtherMapAsync(const QString &fileName);
    void mapDisplayed(const QString &fileName);
};

#endif
