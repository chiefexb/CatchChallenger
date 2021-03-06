#ifndef MAP_VISUALISER_PLAYER_WITH_FIGHT_H
#define MAP_VISUALISER_PLAYER_WITH_FIGHT_H

#include "../../base/render/MapVisualiserPlayer.h"

#include <QSet>

class MapVisualiserPlayerWithFight : public MapVisualiserPlayer
{
    Q_OBJECT
public:
    explicit MapVisualiserPlayerWithFight(const bool &centerOnPlayer=true, const bool &debugTags=false, const bool &useCache=true, const bool &OpenGL=false);
    ~MapVisualiserPlayerWithFight();
    void setBotsAlreadyBeaten(const QSet<quint16> &botAlreadyBeaten);
    void addBeatenBotFight(const quint16 &botFightId);
    bool haveBeatBot(const quint16 &botFightId) const;
    void addRepelStep(const quint32 &repel_step);
protected slots:
    virtual void keyPressParse();
    virtual bool haveStopTileAction();
    virtual bool canGoTo(const CatchChallenger::Direction &direction,CatchChallenger::CommonMap map,COORD_TYPE x,COORD_TYPE y,const bool &checkCollision);
    virtual void resetAll();
protected:
    QSet<quint16> botAlreadyBeaten;
    quint32 repel_step;
    Tiled::Tileset *fightCollisionBot;
signals:
    void repelEffectIsOver() const;
    void teleportConditionNotRespected(const QString &text) const;
};

#endif
