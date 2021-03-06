#include "NormalServerGlobal.h"
#include "VariableServer.h"

#include <QDebug>

NormalServerGlobal::NormalServerGlobal()
{
}

void NormalServerGlobal::checkSettingsFile(QSettings *settings)
{
    #if defined(Q_CC_GNU)
        qDebug() << QStringLiteral("GCC %1.%2.%3 build: ").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__)+__DATE__+" "+__TIME__;
    #else
        #if defined(__DATE__) && defined(__TIME__)
            qDebug() << QStringLiteral("Unknow compiler: ")+__DATE__+" "+__TIME__;
        #else
            qDebug() << QStringLiteral("Unknow compiler");
        #endif
    #endif
    qDebug() << QStringLiteral("Qt version: %1 (%2)").arg(qVersion()).arg(QT_VERSION);

    if(!settings->contains(QLatin1Literal("max-players")))
        settings->setValue(QLatin1Literal("max-players"),200);
    if(!settings->contains(QLatin1Literal("server-ip")))
        settings->setValue(QLatin1Literal("server-ip"),QString());
    if(!settings->contains(QLatin1Literal("pvp")))
        settings->setValue(QLatin1Literal("pvp"),true);
    if(!settings->contains(QLatin1Literal("useSP")))
        settings->setValue(QLatin1Literal("useSP"),true);
    if(!settings->contains(QLatin1Literal("autoLearn")))
        settings->setValue(QLatin1Literal("autoLearn"),false);
    if(!settings->contains(QLatin1Literal("server-port")))
        settings->setValue(QLatin1Literal("server-port"),10000+rand()%(65535-10000));
    if(!settings->contains(QLatin1Literal("sendPlayerNumber")))
        settings->setValue(QLatin1Literal("sendPlayerNumber"),false);
    if(!settings->contains(QLatin1Literal("tolerantMode")))
        settings->setValue(QLatin1Literal("tolerantMode"),false);
    if(!settings->contains(QLatin1Literal("compression")))
        settings->setValue(QLatin1Literal("compression"),QLatin1Literal("zlib"));
    if(!settings->contains(QLatin1Literal("character_delete_time")))
        settings->setValue(QLatin1Literal("character_delete_time"),604800);
    if(!settings->contains(QLatin1Literal("max_pseudo_size")))
        settings->setValue(QLatin1Literal("max_pseudo_size"),20);
    if(!settings->contains(QLatin1Literal("max_character")))
        settings->setValue(QLatin1Literal("max_character"),3);
    if(!settings->contains(QLatin1Literal("min_character")))
        settings->setValue(QLatin1Literal("min_character"),1);
    if(!settings->contains(QLatin1Literal("automatic_account_creation")))
        settings->setValue(QLatin1Literal("automatic_account_creation"),false);
    if(!settings->contains(QLatin1Literal("anonymous")))
        settings->setValue(QLatin1Literal("anonymous"),false);
    if(!settings->contains(QLatin1Literal("server_message")))
        settings->setValue(QLatin1Literal("server_message"),QString());
    if(!settings->contains(QLatin1Literal("proxy")))
        settings->setValue(QLatin1Literal("proxy"),QString());
    if(!settings->contains(QLatin1Literal("proxy_port")))
        settings->setValue(QLatin1Literal("proxy_port"),9050);
    if(!settings->contains(QLatin1Literal("forcedSpeed")))
        settings->setValue(QLatin1Literal("forcedSpeed"),CATCHCHALLENGER_SERVER_NORMAL_SPEED);
    if(!settings->contains(QLatin1Literal("dontSendPseudo")))
        settings->setValue(QLatin1Literal("dontSendPseudo"),false);
    if(!settings->contains(QLatin1Literal("dontSendPlayerType")))
        settings->setValue(QLatin1Literal("dontSendPlayerType"),false);
    if(!settings->contains(QLatin1Literal("forceClientToSendAtMapChange")))
        settings->setValue(QLatin1Literal("forceClientToSendAtMapChange"),true);
    if(!settings->contains(QLatin1Literal("httpDatapackMirror")))
        settings->setValue(QLatin1Literal("httpDatapackMirror"),QString());
    if(!settings->contains(QLatin1Literal("datapackCache")))
        settings->setValue(QLatin1Literal("datapackCache"),-1);
    if(!settings->contains(QLatin1Literal("useSsl")))
    #ifdef Q_OS_LINUX
        settings->setValue(QLatin1Literal("useSsl"),false);
    #else
        settings->setValue(QLatin1Literal("useSsl"),false);
    #endif
    if(!settings->contains(QLatin1Literal("maxPlayerMonsters")))
        settings->setValue(QLatin1Literal("maxPlayerMonsters"),8);
    if(!settings->contains(QLatin1Literal("maxWarehousePlayerMonsters")))
        settings->setValue(QLatin1Literal("maxWarehousePlayerMonsters"),30);
    if(!settings->contains(QLatin1Literal("maxPlayerItems")))
        settings->setValue(QLatin1Literal("maxPlayerItems"),30);
    if(!settings->contains(QLatin1Literal("maxWarehousePlayerItems")))
        settings->setValue(QLatin1Literal("maxWarehousePlayerItems"),150);

    #ifdef Q_OS_LINUX
    settings->beginGroup(QLatin1Literal("Linux"));
    if(!settings->contains(QLatin1Literal("tcpCork")))
        settings->setValue(QLatin1Literal("tcpCork"),true);
    if(!settings->contains(QLatin1Literal("tcpNodelay")))
        settings->setValue(QLatin1Literal("tcpNodelay"),false);
    settings->endGroup();
    #endif

    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm"));
    if(!settings->contains(QLatin1Literal("MapVisibilityAlgorithm")))
        settings->setValue(QLatin1Literal("MapVisibilityAlgorithm"),0);
    settings->endGroup();

    settings->beginGroup(QLatin1Literal("DDOS"));
    if(!settings->contains(QLatin1Literal("waitBeforeConnectAfterKick")))
        settings->setValue(QLatin1Literal("waitBeforeConnectAfterKick"),30);
    if(!settings->contains(QLatin1Literal("computeAverageValueNumberOfValue")))
        settings->setValue(QLatin1Literal("computeAverageValueNumberOfValue"),3);
    if(!settings->contains(QLatin1Literal("computeAverageValueTimeInterval")))
        settings->setValue(QLatin1Literal("computeAverageValueTimeInterval"),5);
    if(!settings->contains(QLatin1Literal("kickLimitMove")))
        settings->setValue(QLatin1Literal("kickLimitMove"),60);
    if(!settings->contains(QLatin1Literal("kickLimitChat")))
        settings->setValue(QLatin1Literal("kickLimitChat"),5);
    if(!settings->contains(QLatin1Literal("kickLimitOther")))
        settings->setValue(QLatin1Literal("kickLimitOther"),30);
    if(!settings->contains(QLatin1Literal("dropGlobalChatMessageGeneral")))
        settings->setValue(QLatin1Literal("dropGlobalChatMessageGeneral"),20);
    if(!settings->contains(QLatin1Literal("dropGlobalChatMessageLocalClan")))
        settings->setValue(QLatin1Literal("dropGlobalChatMessageLocalClan"),20);
    if(!settings->contains(QLatin1Literal("dropGlobalChatMessagePrivate")))
        settings->setValue(QLatin1Literal("dropGlobalChatMessagePrivate"),20);
    settings->endGroup();

    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-Simple"));
    if(!settings->contains(QLatin1Literal("Max")))
        settings->setValue(QLatin1Literal("Max"),50);
    if(!settings->contains(QLatin1Literal("Reshow")))
        settings->setValue(QLatin1Literal("Reshow"),30);
    if(!settings->contains(QLatin1Literal("Reemit")))
        settings->setValue(QLatin1Literal("Reemit"),false);
    settings->endGroup();

    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-WithBorder"));
    if(!settings->contains(QLatin1Literal("MaxWithBorder")))
        settings->setValue(QLatin1Literal("MaxWithBorder"),20);
    if(!settings->contains(QLatin1Literal("ReshowWithBorder")))
        settings->setValue(QLatin1Literal("ReshowWithBorder"),10);
    if(!settings->contains(QLatin1Literal("Max")))
        settings->setValue(QLatin1Literal("Max"),50);
    if(!settings->contains(QLatin1Literal("Reshow")))
        settings->setValue(QLatin1Literal("Reshow"),30);
    settings->endGroup();

    settings->beginGroup(QLatin1Literal("rates"));
    if(!settings->contains(QLatin1Literal("xp_normal")))
        settings->setValue(QLatin1Literal("xp_normal"),1.0);
    if(!settings->contains(QLatin1Literal("gold_normal")))
        settings->setValue(QLatin1Literal("gold_normal"),1.0);
    if(!settings->contains(QLatin1Literal("drop_normal")))
        settings->setValue(QLatin1Literal("drop_normal"),1.0);
    if(!settings->contains(QLatin1Literal("xp_pow_normal")))
        settings->setValue(QLatin1Literal("xp_pow_normal"),1.0);
    settings->endGroup();

    settings->beginGroup(QLatin1Literal("chat"));
    if(!settings->contains(QLatin1Literal("allow-all")))
        settings->setValue(QLatin1Literal("allow-all"),true);
    if(!settings->contains(QLatin1Literal("allow-local")))
        settings->setValue(QLatin1Literal("allow-local"),true);
    if(!settings->contains(QLatin1Literal("allow-private")))
        settings->setValue(QLatin1Literal("allow-private"),true);
    if(!settings->contains(QLatin1Literal("allow-clan")))
        settings->setValue(QLatin1Literal("allow-clan"),true);
    settings->endGroup();

    if(!settings->contains(QLatin1Literal("programmedEventFirstInit")))
    {
        settings->setValue(QLatin1Literal("programmedEventFirstInit"),true);
        settings->beginGroup(QLatin1Literal("programmedEvent"));
            settings->beginGroup(QLatin1Literal("day"));
                settings->beginGroup(QLatin1Literal("day"));
                if(!settings->contains(QLatin1Literal("value")))
                    settings->setValue(QLatin1Literal("value"),"day");
                if(!settings->contains(QLatin1Literal("cycle")))
                    settings->setValue(QLatin1Literal("cycle"),"60");
                if(!settings->contains(QLatin1Literal("offset")))
                    settings->setValue(QLatin1Literal("offset"),"0");
                settings->endGroup();
                settings->beginGroup(QLatin1Literal("night"));
                if(!settings->contains(QLatin1Literal("value")))
                    settings->setValue(QLatin1Literal("value"),"night");
                if(!settings->contains(QLatin1Literal("cycle")))
                    settings->setValue(QLatin1Literal("cycle"),"60");
                if(!settings->contains(QLatin1Literal("offset")))
                    settings->setValue(QLatin1Literal("offset"),"30");
                settings->endGroup();
            settings->endGroup();
        settings->endGroup();
    }

    settings->beginGroup(QLatin1Literal("db"));
    if(!settings->contains(QLatin1Literal("type")))
        settings->setValue(QLatin1Literal("type"),QLatin1Literal("sqlite"));
    if(!settings->contains(QLatin1Literal("mysql_host")))
        settings->setValue(QLatin1Literal("mysql_host"),QLatin1Literal("localhost"));
    if(!settings->contains(QLatin1Literal("mysql_login")))
        settings->setValue(QLatin1Literal("mysql_login"),QLatin1Literal("catchchallenger-login"));
    if(!settings->contains(QLatin1Literal("mysql_pass")))
        settings->setValue(QLatin1Literal("mysql_pass"),QLatin1Literal("catchchallenger-pass"));
    if(!settings->contains(QLatin1Literal("mysql_db")))
        settings->setValue(QLatin1Literal("mysql_db"),QLatin1Literal("catchchallenger"));
    if(!settings->contains(QLatin1Literal("db_fight_sync")))
        settings->setValue(QLatin1Literal("db_fight_sync"),QLatin1Literal("FightSync_AtTheEndOfBattle"));
    if(!settings->contains(QLatin1Literal("secondToPositionSync")))
        settings->setValue(QLatin1Literal("secondToPositionSync"),0);
    if(!settings->contains(QLatin1Literal("positionTeleportSync")))
        settings->setValue(QLatin1Literal("positionTeleportSync"),true);
    if(!settings->contains(QLatin1Literal("considerDownAfterNumberOfTry")))
        settings->setValue(QLatin1Literal("considerDownAfterNumberOfTry"),3);
    if(!settings->contains(QLatin1Literal("tryInterval")))
        settings->setValue(QLatin1Literal("tryInterval"),5);
    settings->endGroup();


    settings->beginGroup(QLatin1Literal("city"));
    if(!settings->contains(QLatin1Literal("capture_frequency")))
        settings->setValue(QLatin1Literal("capture_frequency"),QLatin1Literal("month"));
    if(!settings->contains(QLatin1Literal("capture_day")))
        settings->setValue(QLatin1Literal("capture_day"),QLatin1Literal("monday"));
    if(!settings->contains(QLatin1Literal("capture_time")))
        settings->setValue(QLatin1Literal("capture_time"),QLatin1Literal("0:0"));
    settings->endGroup();

    settings->sync();
}
