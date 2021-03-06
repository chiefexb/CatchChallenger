#include "TimerDdos.h"
#include "../Epoll.h"

#include "../../base/LocalClientHandlerWithoutSender.h"
#include "../../base/BroadCastWithoutSender.h"
#include "../../base/ClientNetworkReadWithoutSender.h"
#include "../../base/GlobalServerData.h"
#include "../../VariableServer.h"
#include "../../../general/base/GeneralVariable.h"

#include <iostream>

TimerDdos::TimerDdos()
{
}

void TimerDdos::exec()
{
    #ifdef CATCHCHALLENGER_EXTRA_CHECK
    if(CatchChallenger::GlobalServerData::serverSettings.ddos.computeAverageValueNumberOfValue<0 || CatchChallenger::GlobalServerData::serverSettings.ddos.computeAverageValueNumberOfValue>CATCHCHALLENGER_SERVER_DDOS_MAX_VALUE)
    {
        qDebug() << "GlobalServerData::serverSettings.ddos.computeAverageValueNumberOfValue out of range:" << CatchChallenger::GlobalServerData::serverSettings.ddos.computeAverageValueNumberOfValue;
        return;
    }
    #endif
    CatchChallenger::LocalClientHandlerWithoutSender::localClientHandlerWithoutSender.doDDOSAction();
    CatchChallenger::BroadCastWithoutSender::broadCastWithoutSender.doDDOSAction();
    CatchChallenger::ClientNetworkReadWithoutSender::clientNetworkReadWithoutSender.doDDOSAction();
}
