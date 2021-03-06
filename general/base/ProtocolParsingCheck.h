#ifndef CATCHCHALLENGER_PROTOCOLPARSINGCHECK_H
#define CATCHCHALLENGER_PROTOCOLPARSINGCHECK_H

#include "ProtocolParsing.h"
#include "GeneralVariable.h"

#ifdef CATCHCHALLENGER_EXTRA_CHECK

namespace CatchChallenger {

class ProtocolParsingCheck : public ProtocolParsingBase
{
    public:
        ProtocolParsingCheck(const PacketModeTransmission &packetModeTransmission);
        friend class Client;
        friend class ProtocolParsingBase;
        friend class ProtocolParsingInputOutput;
        bool valid;
        bool parseIncommingDataRaw(const char *commonBuffer, const quint32 &size,quint32 &cursor);
    private:
        void parseMessage(const quint8 &mainCodeType,const char *data,const int &size);
        void parseFullMessage(const quint8 &mainCodeType,const quint16 &subCodeType,const char *data,const int &size);
        //have query with reply
        void parseQuery(const quint8 &mainCodeType,const quint8 &queryNumber,const char *data,const int &size);
        void parseFullQuery(const quint8 &mainCodeType,const quint16 &subCodeType,const quint8 &queryNumber,const char *data,const int &size);
        //send reply
        void parseReplyData(const quint8 &mainCodeType,const quint8 &queryNumber,const char *data,const int &size);
        void parseFullReplyData(const quint8 &mainCodeType,const quint16 &subCodeType,const quint8 &queryNumber,const char *data,const int &size);
        //message
        void errorParsingLayer(const QString &error);
        void messageParsingLayer(const QString &message) const;

        void disconnectClient();

        ssize_t read(char * data, const int &size);
        ssize_t write(const char * data, const int &size);
};

}

#endif

#endif // PROTOCOLPARSING_H
