#ifndef MAP_VISUALISER_PLAYER_H
#define MAP_VISUALISER_PLAYER_H

#include "MapVisualiser.h"

class MapVisualiserPlayer : public MapVisualiser
{
    Q_OBJECT

public:
    explicit MapVisualiserPlayer(QWidget *parent = 0,const bool &centerOnPlayer=true,const bool &debugTags=false,const bool &useCache=true,const bool &OpenGL=false);
    ~MapVisualiserPlayer();
    virtual bool viewMap(const QString &fileName);
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent *event);
protected:
    Tiled::MapObject * playerMapObject;
    Tiled::Tileset * playerTileset;
    int moveStep;
    Pokecraft::Direction direction;
    quint8 xPerso,yPerso;
    bool inMove;

    QTimer timer;
    QTimer moveTimer;
    QTimer lookToMove;
    QSet<int> keyPressed;
private slots:
    void keyPressParse();

    void moveStepSlot();
    //have look into another direction, if the key remain pressed, apply like move
    void transformLookToMove();
protected:
    //call after enter on new map
    virtual void loadPlayerFromCurrentMap();
    //call before leave the old map (and before loadPlayerFromCurrentMap())
    virtual void unloadPlayerFromCurrentMap();
};

#endif