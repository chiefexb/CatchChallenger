#include "MapVisibilityAlgorithm_Simple_StoreOnSender.h"
#include "MapVisibilityAlgorithm_WithoutSender.h"
#include "../GlobalServerData.h"
#include "../../VariableServer.h"

using namespace CatchChallenger;

//temp variable for purge buffer
bool MapVisibilityAlgorithm_Simple_StoreOnSender::mapHaveChanged;

MapVisibilityAlgorithm_Simple_StoreOnSender::MapVisibilityAlgorithm_Simple_StoreOnSender(
        #ifdef EPOLLCATCHCHALLENGERSERVER
            #ifdef SERVERSSL
                const int &infd, SSL_CTX *ctx
            #else
                const int &infd
            #endif
        #else
        ConnectedSocket *socket
        #endif
        ) :
    Client(
        #ifdef EPOLLCATCHCHALLENGERSERVER
            #ifdef SERVERSSL
                infd,ctx
            #else
                infd
            #endif
        #else
        socket
        #endif
        ),
    to_send_insert(false),
    haveNewMove(false)
{
    #ifdef CATCHCHALLENGER_SERVER_MAP_DROP_BLOCKED_MOVE
    previousMovedUnitBlocked=0;
    #endif
}

MapVisibilityAlgorithm_Simple_StoreOnSender::~MapVisibilityAlgorithm_Simple_StoreOnSender()
{
    extraStop();
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::insertClient()
{
    Map_server_MapVisibility_Simple_StoreOnSender *temp_map=static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(map);
    if(Q_LIKELY(temp_map->show))
    {
        const int loop_size=temp_map->clients.size();
        if(Q_UNLIKELY(loop_size>=GlobalServerData::serverSettings.mapVisibility.simple.max))
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            normalOutput(QStringLiteral("insertClient() too many client, hide now, into: %1").arg(map->map_file));
            #endif
            temp_map->show=false;
            //drop all show client because it have excess the limit
            //drop on all client
            if(temp_map->send_reinsert_all==false)
                temp_map->send_drop_all=true;
            else
                temp_map->send_reinsert_all=false;
        }
        else//why else dropped?
        {
            #ifdef CATCHCHALLENGER_EXTRA_CHECK
            if(this->x>=this->map->width)
            {
                qDebug() << QStringLiteral("x to out of map: %1 > %2 (%3)").arg(this->x).arg(this->map->width).arg(this->map->map_file);
                abort();
                return;
            }
            if(this->y>=this->map->height)
            {
                qDebug() << QStringLiteral("y to out of map: %1 > %2 (%3)").arg(this->y).arg(this->map->height).arg(this->map->map_file);
                abort();
                return;
            }
            #endif
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            //normalOutput(QStringLiteral("insertClient() insert the client, into: %1 (%2,%3)").arg(map->map_file).arg(x).arg(y));
            #endif
            //insert the new client
            to_send_insert=true;
            haveNewMove=false;
            temp_map->to_send_remove.removeOne(public_and_private_informations.public_informations.simplifiedId);
            temp_map->to_send_insert=true;
        }
    }
    else
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        normalOutput(QStringLiteral("insertClient() already too many client, into: %1").arg(map->map_file));
        #endif
    }
    //auto insert to know where it have spawn, now in charge of ClientLocalCalcule
    //insertAnotherClient(player_id,current_map,x,y,last_direction,speed);
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::moveClient(const quint8 &movedUnit,const Direction &direction)
{
    Q_UNUSED(movedUnit);
    Q_UNUSED(direction);
    Map_server_MapVisibility_Simple_StoreOnSender *temp_map=static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(map);
    if(Q_UNLIKELY(mapHaveChanged))
    {
        #ifdef DEBUG_MESSAGE_CLIENT_MOVE
        normalOutput(QStringLiteral("map have change, direction: %4: (%1,%2): %3, send at %5 player(s)").arg(x).arg(y).arg(public_and_private_informations.public_informations.simplifiedId).arg(MoveOnTheMap::directionToString(direction)).arg(loop_size-1));
        #endif
        if(Q_LIKELY(temp_map->show))
        {
            //insert the new client, do into insertClient(), call by singleMove()
        }
        else
        {
            //drop all show client because it have excess the limit, do into removeClient(), call by singleMove()
        }
    }
    else
    {
        if(to_send_insert)
            return;
        #ifdef CATCHCHALLENGER_SERVER_MAP_DROP_OVER_MOVE
        //already into over move
        if(haveNewMove)
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
            normalOutput(QStringLiteral("moveAnotherClientWithMap(%1,%2,%3) to the player: %4, already into over move").arg(player_id).arg(movedUnit).arg(MoveOnTheMap::directionToString(direction)).arg(public_and_private_informations.public_informations.simplifiedId));
            #endif
            //to_send_map_management_remove.remove(player_id); -> what?
            return;//quit now
        }
        #endif
        //here to know how player is affected
        #ifdef DEBUG_MESSAGE_CLIENT_MOVE
        normalOutput(QStringLiteral("after %4: (%1,%2): %3, send at %5 player(s)").arg(x).arg(y).arg(public_and_private_informations.public_informations.simplifiedId).arg(MoveOnTheMap::directionToString(direction)).arg(loop_size-1));
        #endif

        //normal operation
        if(Q_LIKELY(temp_map->show))
        {
            haveNewMove=true;
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_SQUARE
            normalOutput(QStringLiteral("moveAnotherClientWithMap(%1,%2,%3) to the player: %4, normal move").arg(player_id).arg(movedUnit).arg(MoveOnTheMap::directionToString(direction)).arg(public_and_private_informations.public_informations.simplifiedId));
            #endif
        }
        else //all client is dropped due to over load on the map
        {
        }
    }
}

//drop all clients
void MapVisibilityAlgorithm_Simple_StoreOnSender::dropAllClients()
{
    to_send_insert=false;
    haveNewMove=false;

    Client::dropAllClients();
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::purgeBuffer()
{
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::removeClient()
{
    Map_server_MapVisibility_Simple_StoreOnSender *temp_map=static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(map);
    int loop_size=temp_map->clients.size();
    if(Q_UNLIKELY(temp_map->show==false))
    {
        if(Q_UNLIKELY(loop_size<=(GlobalServerData::serverSettings.mapVisibility.simple.reshow)))
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            normalOutput(QStringLiteral("removeClient() client of the map is now under the limit, reinsert all into: %1").arg(map->map_file));
            #endif
            temp_map->show=true;
            //insert all the client because it start to be visible
            if(temp_map->send_drop_all==false)
                temp_map->send_reinsert_all=true;
            else
                temp_map->send_drop_all=false;
        }
        //nothing removed because all clients are already hide
        else
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            normalOutput(QStringLiteral("removeClient() do nothing because client hiden, into: %1").arg(map->map_file));
            #endif
        }
    }
    else //normal working
    {
        #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
        //normalOutput(QStringLiteral("removeClient() normal work, just remove from client on: %1").arg(map->map_file));
        #endif
        //on client side to remove the other visible client for later reinsert
        Client::dropAllClients();
        //on rest side
        if(to_send_insert)
            to_send_insert=false;
        else
        {
            haveNewMove=false;
            temp_map->to_send_remove << public_and_private_informations.public_informations.simplifiedId;
        }
    }
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::mapVisiblity_unloadFromTheMap()
{
    removeClient();
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::extraStop()
{
    if(!character_loaded)
        return;
    unloadFromTheMap();//product remove on the map

    to_send_insert=false;
    haveNewMove=false;
}

bool MapVisibilityAlgorithm_Simple_StoreOnSender::singleMove(const Direction &direction)
{
    CommonMap *old_map=map;
    if(!Client::singleMove(direction))//check of colision disabled because do into LocalClientHandler
        return false;
    if(old_map!=map)
    {
        if(old_map==NULL)
            normalOutput(QStringLiteral("singleMove() old map is null"));
        else
        {
            #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            normalOutput(QStringLiteral("singleMove() have change from old map: %1").arg(old_map->map_file));
            normalOutput(QStringLiteral("singleMove() to the new map: %1").arg(map->map_file));
            #endif
            mapHaveChanged=true;
            CommonMap *new_map=map;
            map=old_map;
            unloadFromTheMap();
            map=static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(new_map);
            if(map==NULL)
                normalOutput(QStringLiteral("singleMove() new map is null"));
            else
            {
                #ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
                normalOutput(QStringLiteral("singleMove() to the new map: %1").arg(map->map_file));
                #endif
                loadOnTheMap();
            }
        }
    }
    return true;
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::loadOnTheMap()
{
    insertClient();
    #ifdef CATCHCHALLENGER_SERVER_EXTRA_CHECK
    if(static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(map)->clients.contains(this))
    {
        normalOutput("loadOnTheMap() try dual insert into the player list");
        return;
    }
    #endif
    static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(map)->clients << this;
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::unloadFromTheMap()
{
    #ifdef CATCHCHALLENGER_SERVER_EXTRA_CHECK
    if(!static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(map)->clients.contains(this))
    {
        normalOutput("unloadFromTheMap() try remove of the player list, but not found");
        return;
    }
    #endif
    static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(map)->clients.removeOne(this);
    mapVisiblity_unloadFromTheMap();
}

//map slots, transmited by the current ClientNetworkRead
void MapVisibilityAlgorithm_Simple_StoreOnSender::put_on_the_map(CommonMap *map,const /*COORD_TYPE*/quint8 &x,const /*COORD_TYPE*/quint8 &y,const Orientation &orientation)
{
    Client::put_on_the_map(map,x,y,orientation);
    loadOnTheMap();
}

bool MapVisibilityAlgorithm_Simple_StoreOnSender::moveThePlayer(const quint8 &previousMovedUnit,const Direction &direction)
{
    mapHaveChanged=false;
    //move the player on the server map
    if(!Client::moveThePlayer(previousMovedUnit,direction))
        return false;
    //send the move to the other client
    moveClient(previousMovedUnit,direction);
    return true;
}

void MapVisibilityAlgorithm_Simple_StoreOnSender::teleportValidatedTo(CommonMap *map,const COORD_TYPE &x,const COORD_TYPE &y,const Orientation &orientation)
{
    bool mapChange=(this->map!=map);
    normalOutput(QStringLiteral("MapVisibilityAlgorithm_Simple_StoreOnSender::teleportValidatedTo() with mapChange: %1").arg(mapChange));
    if(mapChange)
        unloadFromTheMap();
    Client::teleportValidatedTo(map,x,y,orientation);//apply the change into it
    if(mapChange)
    {
        if(this->map->map_file!=map->map_file)
        {
            errorOutput(QStringLiteral("Warning: Map pointer != but map_file is same: %1 && %2, need be done into Client::teleportValidatedTo()").arg(this->map->map_file).arg(map->map_file));
            /*#ifdef DEBUG_MESSAGE_CLIENT_COMPLEXITY_LINEARE
            normalOutput(QStringLiteral("have changed of map for teleportation, old map: %1, new map: %2").arg(this->map->map_file).arg(map->map_file));
            #endif
            this->map=static_cast<Map_server_MapVisibility_Simple_StoreOnSender*>(map);
            loadOnTheMap();*/
        }
    }
    else
        haveNewMove=true;
}
