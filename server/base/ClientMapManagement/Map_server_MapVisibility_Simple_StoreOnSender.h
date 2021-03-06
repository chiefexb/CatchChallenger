#ifndef CATCHCHALLENGER_MAP_SERVER_MAPVISIBILITY_SIMPLE_STOREONSENDER_H
#define CATCHCHALLENGER_MAP_SERVER_MAPVISIBILITY_SIMPLE_STOREONSENDER_H

#include "../MapServer.h"
#include "MapVisibilityAlgorithm_Simple_StoreOnSender.h"

#include <QSet>
#include <QList>

#ifndef CATCHCHALLENGER_BIGBUFFERSIZE_FORTOPLAYER
#define CATCHCHALLENGER_BIGBUFFERSIZE_FORTOPLAYER 128*1024
#endif

namespace CatchChallenger {
class MapVisibilityAlgorithm_Simple_StoreOnSender;

class Map_server_MapVisibility_Simple_StoreOnSender : public MapServer
{
public:
    Map_server_MapVisibility_Simple_StoreOnSender();
    void purgeBuffer();
    QList<MapVisibilityAlgorithm_Simple_StoreOnSender *> clients;//manipulated by thread of ClientMapManagement()

    //mostly less remove than don't remove
    QList<quint16> to_send_remove;
    int to_send_remove_size;
    bool show;
    bool to_send_insert;
    bool send_drop_all;
    bool send_reinsert_all;

    static MapVisibilityAlgorithm_Simple_StoreOnSender * clientsToSendDataNewClients[65535];
    static MapVisibilityAlgorithm_Simple_StoreOnSender * clientsToSendDataOldClients[65535];
    static char buffer[CATCHCHALLENGER_BIGBUFFERSIZE_FORTOPLAYER];
};
}

#endif
