#include "BroadCastWithoutSender.h"
#include "Client.h"
#include "GlobalServerData.h"
#include "../../general/base/ProtocolParsing.h"

using namespace CatchChallenger;

BroadCastWithoutSender BroadCastWithoutSender::broadCastWithoutSender;

BroadCastWithoutSender::BroadCastWithoutSender()
{
}

#ifndef EPOLLCATCHCHALLENGERSERVER
void BroadCastWithoutSender::emit_serverCommand(const QString &command,const QString &extraText)
{
    /*emit */serverCommand(command,extraText);
}

void BroadCastWithoutSender::emit_new_player_is_connected(const Player_private_and_public_informations &newPlayer)
{
    /*emit */new_player_is_connected(newPlayer);
}

void BroadCastWithoutSender::emit_player_is_disconnected(const QString &oldPlayer)
{
    /*emit */player_is_disconnected(oldPlayer);
}

void BroadCastWithoutSender::emit_new_chat_message(const QString &pseudo,const Chat_type &type,const QString &text)
{
    /*emit */new_chat_message(pseudo,type,text);
}
#endif

void BroadCastWithoutSender::receive_instant_player_number(const qint16 &connected_players)
{
    if(GlobalServerData::serverSettings.sendPlayerNumber)
    {
        if(Client::clientBroadCastList.isEmpty())
            return;

        QByteArray finalData;
        {
            QByteArray outputData;
            QDataStream out(&outputData, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_4);
            if(GlobalServerData::serverSettings.max_players<=255)
                out << (qint8)connected_players;
            else
                out << (qint16)connected_players;
            finalData.resize(16+outputData.size());
            finalData.resize(ProtocolParsingBase::computeOutcommingData(
            #ifndef CATCHCHALLENGERSERVERDROPIFCLENT
            false,
            #endif
                    finalData.data(),0xC3,outputData.constData(),outputData.size()));
        }

        int index=0;
        const int &list_size=Client::clientBroadCastList.size();
        while(index<list_size)
        {
            Client::clientBroadCastList.at(index)->receive_instant_player_number(connected_players,finalData);
            index++;
        }
    }
}

void BroadCastWithoutSender::doDDOSAction()
{
    if(Client::generalChatDrop.size()==CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE)
    {
        Client::generalChatDropTotalCache=0;
        int index=CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-GlobalServerData::serverSettings.ddos.computeAverageValueNumberOfValue;
        while(index<(CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-1))
        {
            Client::generalChatDrop[index]=Client::generalChatDrop[index+1];
            Client::generalChatDropTotalCache+=Client::generalChatDrop[index];
            index++;
        }
        Client::generalChatDrop[CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-1]=Client::generalChatDropNewValue;
        Client::generalChatDropTotalCache+=Client::generalChatDropNewValue;
        Client::generalChatDropNewValue=0;
    }
    if(Client::clanChatDrop.size()==CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE)
    {
        Client::clanChatDropTotalCache=0;
        int index=CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-GlobalServerData::serverSettings.ddos.computeAverageValueNumberOfValue;
        while(index<(CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-1))
        {
            Client::clanChatDrop[index]=Client::clanChatDrop[index+1];
            Client::clanChatDropTotalCache+=Client::clanChatDrop[index];
            index++;
        }
        Client::clanChatDrop[CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-1]=Client::clanChatDropNewValue;
        Client::clanChatDropTotalCache+=Client::clanChatDropNewValue;
        Client::clanChatDropNewValue=0;
    }
    if(Client::privateChatDrop.size()==CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE)
    {
        Client::privateChatDropTotalCache=0;
        int index=CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-GlobalServerData::serverSettings.ddos.computeAverageValueNumberOfValue;
        while(index<(CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-1))
        {
            Client::privateChatDrop[index]=Client::privateChatDrop[index+1];
            Client::privateChatDropTotalCache+=Client::privateChatDrop[index];
            index++;
        }
        Client::privateChatDrop[CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE-1]=Client::privateChatDropNewValue;
        Client::privateChatDropTotalCache+=Client::privateChatDropNewValue;
        Client::privateChatDropNewValue=0;
    }
}

