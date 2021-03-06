#ifndef CATCHCHALLENGER_MAP_SERVER_MAPVISIBILITY_WITHBORDER_STOREONSENDER_H
#define CATCHCHALLENGER_MAP_SERVER_MAPVISIBILITY_WITHBORDER_STOREONSENDER_H

#include "../MapServer.h"
#include "MapVisibilityAlgorithm_WithBorder_StoreOnSender.h"

#include <QSet>
#include <QList>

namespace CatchChallenger {
class MapVisibilityAlgorithm_WithBorder_StoreOnSender;
class Map_server_MapVisibility_WithBorder_StoreOnSender : public MapServer
{
public:
    Map_server_MapVisibility_WithBorder_StoreOnSender();
    void purgeBuffer();
    QList<MapVisibilityAlgorithm_WithBorder_StoreOnSender *> clients;//manipulated by thread of ClientMapManagement()

    QSet<SIMPLIFIED_PLAYER_ID_TYPE>     to_send_remove;
    quint16 clientsOnBorder;
    bool showWithBorder;
    bool show;
};
}

#endif
