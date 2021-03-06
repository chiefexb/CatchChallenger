#include "BaseWindow.h"
#include "ui_BaseWindow.h"
#include "../../../general/base/FacilityLib.h"
#include "../../../general/base/CommonDatapack.h"
#include "../ClientVariable.h"
#include "DatapackClientLoader.h"
#include "Chat.h"
#include "../../fight/interface/ClientFightEngine.h"
#include "../Options.h"
#include "../Audio.h"

#include <QListWidgetItem>
#include <QBuffer>
#include <QInputDialog>
#include <QMessageBox>
#include <QQmlContext>
#include <vlc/vlc.h>
#include <iostream>
#include <QStandardPaths>

using namespace CatchChallenger;

void BaseWindow::resetAll()
{
    #ifndef CATCHCHALLENGER_VERSION_ULTIMATE
    ui->labelLastReplyTime->hide();
    ui->labelQueryList->hide();
    #else
    ui->labelLastReplyTime->setText(tr("Last reply time: ?"));
    ui->labelQueryList->setText(tr("Running query: ? Query with worse time: ?"));
    #endif
    ui->labelSlow->hide();
    ui->frame_main_display_interface_player->hide();
    ui->label_interface_number_of_player->setText("?/?");
    ui->stackedWidget->setCurrentWidget(ui->page_init);
    Chat::chat->resetAll();
    MapController::mapController->resetAll();
    waitedObjectType=ObjectType_All;
    lastReplyTimeValue=-1;
    monsterBeforeMoveForChangeInWaiting=false;
    worseQueryTime=0;
    progressingDatapackFileSize=0;
    haveDatapack=false;
    characterSelected=false;
    havePlayerInformations=false;
    DatapackClientLoader::datapackLoader.resetAll();
    haveInventory=false;
    isLogged=false;
    datapackIsParsed=false;
    characterEntryList.clear();
    ui->inventory->clear();
    ui->shopItemList->clear();
    items_graphical.clear();
    items_to_graphical.clear();
    shop_items_graphical.clear();
    shop_items_to_graphical.clear();
    plants_items_graphical.clear();
    plants_items_to_graphical.clear();
    crafting_recipes_items_graphical.clear();
    crafting_recipes_items_to_graphical.clear();
    itemOnMap.clear();
    quests.clear();
    ui->tip->setVisible(false);
    ui->persistant_tip->setVisible(false);
    ui->gain->setVisible(false);
    ui->IG_dialog->setVisible(false);
    inSelection=false;
    isInQuest=false;
    displayAttackProgression=0;
    mLastGivenXP=0;
    fightTimerFinish=false;
    queryList.clear();
    ui->inventoryInformation->setVisible(false);
    ui->inventoryUse->setVisible(false);
    ui->inventoryDestroy->setVisible(false);
    previousRXSize=0;
    previousTXSize=0;
    attack_quantity_changed=0;
    previousAnimationWidget=NULL;
    ui->plantUse->setVisible(false);
    ui->craftingUse->setVisible(false);
    waitToSell=false;
    fightTimerFinish=false;
    ui->tabWidgetTrainerCard->setCurrentWidget(ui->tabWidgetTrainerCardPage1);
    ui->selectMonster->setVisible(false);
    doNextActionStep=DoNextActionStep_Start;
    clan=0;
    clan_leader=false;
    allow.clear();
    actionClan.clear();
    clanName.clear();
    haveClanInformations=false;
    nextCityCatchTimer.stop();
    battleInformationsList.clear();
    botFightList.clear();
    buffToGraphicalItemTop.clear();
    buffToGraphicalItemBottom.clear();
    zonecatch=false;
    marketBuyInSuspend=false;
    marketBuyObjectList.clear();
    marketBuyCashInSuspend=0;
    marketPutMonsterList.clear();
    marketPutMonsterPlaceList.clear();
    marketPutInSuspend=false;
    marketPutCashInSuspend=0;
    marketWithdrawInSuspend=false;
    marketWithdrawObjectList.clear();
    marketWithdrawMonsterList.clear();
    datapackDownloadedCount=0;
    datapackDownloadedSize=0;
    escape=false;
    escapeSuccess=false;
    datapackFileNumber=0;
    datapackFileSize=0;
    baseMonsterEvolution=NULL;
    targetMonsterEvolution=NULL;
    idMonsterEvolution=0;
    evolutionControl=NULL;
    lastPlaceDisplayed.clear();
    events.clear();
    visualCategory.clear();
    add_to_inventoryGainList.clear();
    add_to_inventoryGainTime.clear();
    add_to_inventoryGainExtraList.clear();
    add_to_inventoryGainExtraTime.clear();
    while(!ambianceList.isEmpty())
    {
        libvlc_media_player_stop(ambianceList.first().player);
        libvlc_media_player_release(ambianceList.first().player);
        ambianceList.removeFirst();
    }
    industryStatus.products.clear();
    industryStatus.resources.clear();
    if(newProfile!=NULL)
    {
        delete newProfile;
        newProfile=NULL;
    }

    CatchChallenger::ClientFightEngine::fightEngine.resetAll();
}

void BaseWindow::serverIsLoading()
{
    ui->label_connecting_status->setText(tr("Preparing the game data"));
    resetAll();
}

void BaseWindow::serverIsReady()
{
    ui->label_connecting_status->setText(tr("Game data is ready"));
}

void BaseWindow::setMultiPlayer(bool multiplayer)
{
    emit sendsetMultiPlayer(multiplayer);
    this->multiplayer=multiplayer;
    /*if(!multiplayer)
        MapController::mapController->setOpenGl(true);*/
    //frame_main_display_right->setVisible(multiplayer);
    //ui->frame_main_display_interface_player->setVisible(multiplayer);//displayed when have the player connected (if have)
}

void BaseWindow::disconnected(QString reason)
{
    if(haveShowDisconnectionReason)
        return;
    haveShowDisconnectionReason=true;
    QMessageBox::information(this,tr("Disconnected"),tr("Disconnected by the reason: %1").arg(reason));
    newError("Disconnected",QStringLiteral("Disconnected by the reason: %1").arg(reason));
    resetAll();
}

void BaseWindow::notLogged(QString reason)
{
    QMessageBox::information(this,tr("Unable to login"),tr("Unable to login: %1").arg(reason));
    newError("Unable to login",QStringLiteral("Unable to login: %1").arg(reason));
    resetAll();
}

void BaseWindow::logged(const QList<CharacterEntry> &characterEntryList)
{
    CatchChallenger::Api_client_real::client->sendDatapackContent();
    this->characterEntryList=characterEntryList;
    isLogged=true;
    updateConnectingStatus();
    ui->character_add->setEnabled(characterEntryList.size()<CommonSettings::commonSettings.max_character);
    ui->character_remove->setEnabled(characterEntryList.size()>CommonSettings::commonSettings.min_character);
    if(characterEntryList.isEmpty())
    {
        if(CommonSettings::commonSettings.max_character==0)
            emit message("Can't create character but the list is empty");
    }
}

void BaseWindow::protocol_is_good()
{
    ui->label_connecting_status->setText(tr("Try login..."));
}

void BaseWindow::stateChanged(QAbstractSocket::SocketState socketState)
{
    if(this->socketState==socketState)
        return;
    this->socketState=socketState;

    if(socketState!=QAbstractSocket::UnconnectedState)
    {
        if(socketState==QAbstractSocket::ConnectedState)
        {
            haveShowDisconnectionReason=false;
            ui->label_connecting_status->setText(tr("Try initialise the protocol..."));
        }
        else
            ui->label_connecting_status->setText(tr("Connecting to the server..."));
    }
}

void BaseWindow::have_current_player_info()
{
    #ifdef DEBUG_BASEWINDOWS
    qDebug() << "BaseWindow::have_current_player_info()";
    #endif
    if(havePlayerInformations)
        return;
    havePlayerInformations=true;
    Player_private_and_public_informations informations=CatchChallenger::Api_client_real::client->get_player_informations();
    MapController::mapController->have_current_player_info(informations);
    CatchChallenger::ClientFightEngine::fightEngine.public_and_private_informations.playerMonster=CatchChallenger::Api_client_real::client->player_informations.playerMonster;
    clan=informations.clan;
    allow=informations.allow;
    clan_leader=informations.clan_leader;
    cash=informations.cash;
    warehouse_cash=informations.warehouse_cash;
    quests=informations.quests;
    ui->player_informations_pseudo->setText(informations.public_informations.pseudo);
    ui->tradePlayerPseudo->setText(informations.public_informations.pseudo);
    ui->warehousePlayerPseudo->setText(informations.public_informations.pseudo);
    ui->player_informations_cash->setText(QStringLiteral("%1$").arg(informations.cash));
    ui->shopCash->setText(tr("Cash: %1$").arg(informations.cash));
    CatchChallenger::ClientFightEngine::fightEngine.setVariableContent(CatchChallenger::Api_client_real::client->get_player_informations());
    DebugClass::debugConsole(QStringLiteral("%1 is logged with id: %2, cash: %3").arg(informations.public_informations.pseudo).arg(informations.public_informations.simplifiedId).arg(informations.cash));
    updateConnectingStatus();
    updateClanDisplay();
}

void BaseWindow::insert_player(const CatchChallenger::Player_public_informations &player,const quint32 &mapId,const quint8 &x,const quint8 &y,const CatchChallenger::Direction &direction)
{
    Q_UNUSED(player);
    Q_UNUSED(mapId);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(direction);
    updatePlayerImage();
}

void BaseWindow::haveTheDatapack()
{
    #ifdef DEBUG_BASEWINDOWS
    qDebug() << "BaseWindow::haveTheDatapack()";
    #endif
    if(haveDatapack)
        return;
    haveDatapack=true;

    if(CatchChallenger::Api_client_real::client!=NULL)
        emit parseDatapack(CatchChallenger::Api_client_real::client->datapackPath());
}

void BaseWindow::datapackSize(const quint32 &datapackFileNumber,const quint32 &datapackFileSize)
{
    this->datapackFileNumber=datapackFileNumber;
    this->datapackFileSize=datapackFileSize;
    updateConnectingStatus();
}

void BaseWindow::newDatapackFile(const quint32 &size)
{
    progressingDatapackFileSize=0;
    datapackDownloadedCount++;
    datapackDownloadedSize+=size;
    updateConnectingStatus();
}

void BaseWindow::progressingDatapackFile(const quint32 &size)
{
    progressingDatapackFileSize=size;
    updateConnectingStatus();
}

void BaseWindow::have_inventory(const QHash<quint16,quint32> &items, const QHash<quint16, quint32> &warehouse_items)
{
    #ifdef DEBUG_BASEWINDOWS
    qDebug() << "BaseWindow::have_inventory()";
    #endif
    this->items=items;
    this->warehouse_items=warehouse_items;
    haveInventory=true;
    updateConnectingStatus();
}

void BaseWindow::load_inventory()
{
    #ifdef DEBUG_BASEWINDOWS
    qDebug() << "BaseWindow::load_inventory()";
    #endif
    if(!haveInventory || !datapackIsParsed)
        return;
    ui->inventoryInformation->setVisible(false);
    ui->inventoryUse->setVisible(false);
    ui->inventoryDestroy->setVisible(false);
    ui->inventory->clear();
    items_graphical.clear();
    items_to_graphical.clear();
    QHashIterator<quint16,quint32> i(items);
    while (i.hasNext()) {
        i.next();

        bool show=!inSelection;
        if(inSelection)
        {
            switch(waitedObjectType)
            {
                case ObjectType_Seed:
                    if(DatapackClientLoader::datapackLoader.itemToPlants.contains(i.key()))
                        show=true;
                break;
                case ObjectType_UseInFight:
                    if(CatchChallenger::ClientFightEngine::fightEngine.isInFightWithWild() && CommonDatapack::commonDatapack.items.trap.contains(i.key()))
                        show=true;
                    else if(CommonDatapack::commonDatapack.items.monsterItemEffect.contains(i.key()))
                        show=true;
                    else
                        show=false;
                break;
                default:
                qDebug() << "waitedObjectType is unknow into load_inventory()";
                break;
            }
        }
        if(show)
        {
            QListWidgetItem *item=new QListWidgetItem();
            items_to_graphical[i.key()]=item;
            items_graphical[item]=i.key();
            if(DatapackClientLoader::datapackLoader.itemsExtra.contains(i.key()))
            {
                item->setIcon(DatapackClientLoader::datapackLoader.itemsExtra.value(i.key()).image);
                if(i.value()>1)
                    item->setText(QString::number(i.value()));
                item->setToolTip(DatapackClientLoader::datapackLoader.itemsExtra.value(i.key()).name);
            }
            else
            {
                item->setIcon(DatapackClientLoader::datapackLoader.defaultInventoryImage());
                if(i.value()>1)
                    item->setText(QStringLiteral("id: %1 (x%2)").arg(i.key()).arg(i.value()));
                else
                    item->setText(QStringLiteral("id: %1").arg(i.key()));
            }
            ui->inventory->addItem(item);
        }
    }
}

void BaseWindow::datapackParsed()
{
    #ifdef DEBUG_BASEWINDOWS
    qDebug() << "BaseWindow::datapackParsed()";
    #endif
    datapackIsParsed=true;
    updateConnectingStatus();
    loadSettingsWithDatapack();
    {
        if(QFile(CatchChallenger::Api_client_real::client->datapackPath()+QStringLiteral("/images/interface/fight/labelBottom.png")).exists())
            ui->frameFightBottom->setStyleSheet(QStringLiteral("#frameFightBottom{background-image: url('")+CatchChallenger::Api_client_real::client->datapackPath()+QStringLiteral("/images/interface/fight/labelBottom.png');padding:6px 6px 6px 14px;}"));
        else
            ui->frameFightBottom->setStyleSheet(QStringLiteral("#frameFightBottom{background-image: url(:/images/interface/fight/labelBottom.png);padding:6px 6px 6px 14px;}"));
        if(QFile(CatchChallenger::Api_client_real::client->datapackPath()+QStringLiteral("/images/interface/fight/labelTop.png")).exists())
            ui->frameFightTop->setStyleSheet(QStringLiteral("#frameFightTop{background-image: url('")+CatchChallenger::Api_client_real::client->datapackPath()+QStringLiteral("/images/interface/fight/labelTop.png');padding:6px 14px 6px 6px;}"));
        else
            ui->frameFightTop->setStyleSheet(QStringLiteral("#frameFightTop{background-image: url(:/images/interface/fight/labelTop.png);padding:6px 14px 6px 6px;}"));
    }
    //updatePlayerImage();
}

void BaseWindow::datapackChecksumError()
{
    #ifdef DEBUG_BASEWINDOWS
    qDebug() << "BaseWindow::datapackChecksumError()";
    #endif
    datapackIsParsed=false;
    emit newError(tr("Datapack on mirror is corrupted"),QStringLiteral("The checksum sended by the server is not the same than have on the mirror"));
}

void BaseWindow::loadSoundSettings()
{
    const QStringList &outputDeviceNames=Audio::audio.output_list();
    Options::options.setAudioDeviceList(outputDeviceNames);
    ui->audiodevice->clear();
    if(outputDeviceNames.isEmpty())
        {}//soundEngine.setOutputDeviceName(QString());
    else
    {
        const int &indexDevice=Options::options.getIndexDevice();
        ui->audiodevice->addItems(outputDeviceNames);
        if(indexDevice!=-1)
        {
            ui->audiodevice->setCurrentIndex(indexDevice);
            {}//soundEngine.setOutputDeviceName(outputDeviceNames.at(indexDevice));
        }
        connect(ui->audiodevice,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&BaseWindow::changeDeviceIndex);
    }
}

void BaseWindow::setEvents(const QList<QPair<quint8,quint8> > &events)
{
    this->events.clear();
    int index=0;
    while(index<events.size())
    {
        const QPair<quint8,quint8> event=events.at(index);
        if(event.first>=CommonDatapack::commonDatapack.events.size())
        {
            emit error("BaseWindow::setEvents() event out of range");
            break;
        }
        if(event.second>=CommonDatapack::commonDatapack.events.at(event.first).values.size())
        {
            emit error("BaseWindow::setEvents() event value out of range");
            break;
        }
        while(this->events.size()<=event.first)
            this->events << 0;
        this->events[event.first]=event.second;
        index++;
    }
    load_event();
}

void BaseWindow::load_event()
{
    if(isLogged && datapackIsParsed && havePlayerInformations)
    {
        while(events.size()<CatchChallenger::CommonDatapack::commonDatapack.events.size())
            events << 0;
    }
    if(events.size()>CatchChallenger::CommonDatapack::commonDatapack.events.size())
    {
        while(events.size()>CatchChallenger::CommonDatapack::commonDatapack.events.size())
            events.removeLast();
        emit error("BaseWindow::load_event() event list biger than it should");
    }
}

void BaseWindow::updateConnectingStatus()
{
    if(isLogged && datapackIsParsed && !havePlayerInformations)
    {
        if(ui->stackedWidget->currentWidget()!=ui->page_character)
        {
            ui->stackedWidget->setCurrentWidget(ui->page_character);
            updateCharacterList();
            if(characterEntryList.isEmpty() && CommonSettings::commonSettings.max_character>0)
            {
                if(CommonSettings::commonSettings.min_character>0)
                {
                    ui->stackedWidget->setCurrentWidget(ui->page_init);
                    ui->label_connecting_status->setText(QString());
                }
                on_character_add_clicked();
            }
            if(characterEntryList.size()==1 && CommonSettings::commonSettings.min_character>=characterEntryList.size() && CommonSettings::commonSettings.max_character<=characterEntryList.size())
            {
                if(!characterSelected && characterEntryList.first().mapId!=-1)
                {
                    qDebug() << characterEntryList.first().mapId;
                    characterSelected=true;
                    ui->characterEntryList->item(ui->characterEntryList->count()-1)->setSelected(true);
                    on_character_select_clicked();
                }
            }
        }
        return;
    }
    QStringList waitedData;
    if(haveDatapack && (!haveInventory || !havePlayerInformations))
    {
        if(!havePlayerInformations)
            waitedData << tr("Loading of the player informations");
        else
            waitedData << tr("Loading of the inventory");
    }
    if(!haveDatapack)
    {
        if(datapackFileSize==0)
            waitedData << tr("Loading of the datapack");
        else if(datapackFileSize<0)
            waitedData << tr("Loaded datapack size: %1KB").arg((datapackDownloadedSize+progressingDatapackFileSize)/1000);//when the http server don't send the size
        else if((datapackDownloadedSize+progressingDatapackFileSize)>=(quint32)datapackFileSize)
            waitedData << tr("Loaded datapack file: 100%");
        else
            waitedData << tr("Loaded datapack file: %1%").arg(((datapackDownloadedSize+progressingDatapackFileSize)*100)/datapackFileSize);
    }
    else if(!datapackIsParsed)
        waitedData << tr("Opening the datapack");
    if(waitedData.isEmpty())
    {
        Player_private_and_public_informations player_private_and_public_informations=CatchChallenger::Api_client_real::client->get_player_informations();
        itemOnMap=player_private_and_public_informations.itemOnMap;
        warehouse_playerMonster=player_private_and_public_informations.warehouse_playerMonster;
        MapController::mapController->setBotsAlreadyBeaten(player_private_and_public_informations.bot_already_beaten);
        MapController::mapController->setInformations(&items,&quests,&events,&itemOnMap);
        load_inventory();
        load_plant_inventory();
        load_crafting_inventory();
        updateDisplayedQuests();
        if(!check_senddata())
            return;
        load_monsters();
        show_reputation();
        load_event();
        emit gameIsLoaded();
        this->setWindowTitle(QStringLiteral("CatchChallenger - %1").arg(CatchChallenger::Api_client_real::client->getPseudo()));
        ui->stackedWidget->setCurrentWidget(ui->page_map);
        showTip(tr("Welcome <b><i>%1</i></b> on <i>CatchChallenger</i>").arg(CatchChallenger::Api_client_real::client->getPseudo()));
        return;
    }
    ui->label_connecting_status->setText(tr("Waiting: %1").arg(waitedData.join(", ")));
}

//with the datapack content
bool BaseWindow::check_senddata()
{
    if(!check_monsters())
        return false;
    //check the reputation here
    QMapIterator<quint8,PlayerReputation> i(CatchChallenger::Api_client_real::client->player_informations.reputation);
    while(i.hasNext())
    {
        i.next();
        if(i.value().level<-100 || i.value().level>100)
        {
            error(QStringLiteral("level is <100 or >100, skip"));
            return false;
        }
        if(i.key()>=CatchChallenger::CommonDatapack::commonDatapack.reputation.size())
        {
            error(QStringLiteral("The reputation: %1 don't exist").arg(i.key()));
            return false;
        }
        if(i.value().level>=0)
        {
            if(i.value().level>=CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_positive.size())
            {
                error(QStringLiteral("The reputation level %1 is wrong because is out of range (reputation level: %2 > max level: %3)").arg(i.key()).arg(i.value().level).arg(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_positive.size()));
                return false;
            }
        }
        else
        {
            if((-i.value().level)>CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_negative.size())
            {
                error(QStringLiteral("The reputation level %1 is wrong because is out of range (reputation level: %2 < max level: %3)").arg(i.key()).arg(i.value().level).arg(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_negative.size()));
                return false;
            }
        }
        if(i.value().point>0)
        {
            if(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_positive.size()==i.value().level)//start at level 0 in positive
            {
                emit message(QStringLiteral("The reputation level is already at max, drop point"));
                return false;
            }
            if(i.value().point>=CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_positive.at(i.value().level+1))//start at level 0 in positive
            {
                error(QStringLiteral("The reputation point %1 is greater than max %2").arg(i.value().point).arg(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_positive.at(i.value().level)));
                return false;
            }
        }
        else if(i.value().point<0)
        {
            if(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_negative.size()==-i.value().level)//start at level -1 in negative
            {
                error(QStringLiteral("The reputation level is already at min, drop point"));
                return false;
            }
            if(i.value().point<CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_negative.at(-i.value().level))//start at level -1 in negative
            {
                error(QStringLiteral("The reputation point %1 is greater than max %2").arg(i.value().point).arg(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_negative.at(i.value().level)));
                return false;
            }
        }
    }
    return true;
}

void BaseWindow::show_reputation()
{
    QString html="<ul>";
    QMapIterator<quint8,PlayerReputation> i(CatchChallenger::Api_client_real::client->player_informations.reputation);
    while(i.hasNext())
    {
        i.next();
        if(i.value().level>=0)
        {
            if((i.value().level+1)==CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_positive.size())
                html+=QStringLiteral("<li>100% %1</li>")
                    .arg(DatapackClientLoader::datapackLoader.reputationExtra.value(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).name).reputation_positive.last());
            else
            {
                qint32 next_level_xp=CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_positive.at(i.value().level+1);
                if(next_level_xp==0)
                {
                    error("Next level can't be need 0 xp");
                    return;
                }
                QString text=DatapackClientLoader::datapackLoader.reputationExtra.value(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).name).reputation_positive.at(i.value().level);
                html+=QStringLiteral("<li>%1% %2</li>").arg((i.value().point*100)/next_level_xp).arg(text);
            }
        }
        else
        {
            if((-i.value().level)==CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_negative.size())
                html+=QStringLiteral("<li>100% %1</li>")
                    .arg(DatapackClientLoader::datapackLoader.reputationExtra.value(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).name).reputation_negative.last());
            else
            {
                qint32 next_level_xp=CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).reputation_negative.at(-i.value().level);
                if(next_level_xp==0)
                {
                    error("Next level can't be need 0 xp");
                    return;
                }
                QString text=DatapackClientLoader::datapackLoader.reputationExtra.value(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(i.key()).name).reputation_negative.at(-i.value().level-1);
                html+=QStringLiteral("<li>%1% %2</li>")
                    .arg((i.value().point*100)/next_level_xp)
                    .arg(text);
            }
        }
    }
    html+="</ul>";
    ui->labelReputation->setText(html);
}

QPixmap BaseWindow::getFrontSkin(const QString &skinName) const
{
    const QPixmap skin(getFrontSkinPath(skinName));
    if(!skin.isNull())
        return skin;
    return QPixmap(":/images/player_default/front.png");
}

QPixmap BaseWindow::getFrontSkin(const quint32 &skinId) const
{
    const QPixmap skin(getFrontSkinPath(skinId));
    if(!skin.isNull())
        return skin;
    return QPixmap(":/images/player_default/front.png");
}

QPixmap BaseWindow::getBackSkin(const quint32 &skinId) const
{
    const QPixmap skin(getBackSkinPath(skinId));
    if(!skin.isNull())
        return skin;
    return QPixmap(":/images/player_default/back.png");
}

QString BaseWindow::getSkinPath(const QString &skinName,const QString &type) const
{
    {
        QFileInfo pngFile(CatchChallenger::Api_client_real::client->datapackPath()+DATAPACK_BASE_PATH_SKIN+skinName+QStringLiteral("/%1.png").arg(type));
        if(pngFile.exists())
            return pngFile.absoluteFilePath();
    }
    {
        QFileInfo gifFile(CatchChallenger::Api_client_real::client->datapackPath()+DATAPACK_BASE_PATH_SKIN+skinName+QStringLiteral("/%1.gif").arg(type));
        if(gifFile.exists())
            return gifFile.absoluteFilePath();
    }
    QDir folderList(CatchChallenger::Api_client_real::client->datapackPath()+DATAPACK_BASE_PATH_SKINBASE);
    const QStringList &entryList=folderList.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
    int entryListIndex=0;
    while(entryListIndex<entryList.size())
    {
        {
            QFileInfo pngFile(QStringLiteral("%1/skin/%2/%3/%4.png").arg(CatchChallenger::Api_client_real::client->datapackPath()).arg(entryList.at(entryListIndex)).arg(skinName).arg(type));
            if(pngFile.exists())
                return pngFile.absoluteFilePath();
        }
        {
            QFileInfo gifFile(QStringLiteral("%1/skin/%2/%3/%4.gif").arg(CatchChallenger::Api_client_real::client->datapackPath()).arg(entryList.at(entryListIndex)).arg(skinName).arg(type));
            if(gifFile.exists())
                return gifFile.absoluteFilePath();
        }
        entryListIndex++;
    }
    return QString();
}

QString BaseWindow::getFrontSkinPath(const quint32 &skinId) const
{
    const QStringList &skinFolderList=DatapackClientLoader::datapackLoader.skins;
    //front image
    if(skinId<(quint32)skinFolderList.size())
        return getSkinPath(skinFolderList.at(skinId),"front");
    else
        qDebug() << "The skin id: "+QString::number(skinId)+", into a list of: "+QString::number(skinFolderList.size())+" item(s) into BaseWindow::updatePlayerImage()";
    return ":/images/player_default/front.png";
}

QString BaseWindow::getFrontSkinPath(const QString &skin) const
{
    return getSkinPath(skin,"front");
}

QString BaseWindow::getBackSkinPath(const quint32 &skinId) const
{
    const QStringList &skinFolderList=DatapackClientLoader::datapackLoader.skins;
    //front image
    if(skinId<(quint32)skinFolderList.size())
        return getSkinPath(skinFolderList.at(skinId),"back");
    else
        qDebug() << "The skin id: "+QString::number(skinId)+", into a list of: "+QString::number(skinFolderList.size())+" item(s) into BaseWindow::updatePlayerImage()";
    return ":/images/player_default/back.png";
}

void BaseWindow::updatePlayerImage()
{
    if(havePlayerInformations && haveDatapack)
    {
        Player_public_informations informations=CatchChallenger::Api_client_real::client->get_player_informations().public_informations;
        playerFrontImage=getFrontSkin(informations.skinId);
        playerBackImage=getBackSkin(informations.skinId);
        playerFrontImagePath=getFrontSkinPath(informations.skinId);
        playerBackImagePath=getBackSkinPath(informations.skinId);

        //load into the UI
        ui->player_informations_front->setPixmap(playerFrontImage);
        ui->warehousePlayerImage->setPixmap(playerFrontImage);
        ui->tradePlayerImage->setPixmap(playerFrontImage);
    }
}

void BaseWindow::updateDisplayedQuests()
{
    QString html=QStringLiteral("<ul>");
    ui->questsList->clear();
    quests_to_id_graphical.clear();
    QHashIterator<quint16, PlayerQuest> i(quests);
    while (i.hasNext()) {
        i.next();
        if(DatapackClientLoader::datapackLoader.questsExtra.contains(i.key()))
        {
            if(i.value().step>0)
            {
                QListWidgetItem * item=new QListWidgetItem(DatapackClientLoader::datapackLoader.questsExtra.value(i.key()).name);
                quests_to_id_graphical[item]=i.key();
                ui->questsList->addItem(item);
            }
            if(i.value().step==0 || i.value().finish_one_time)
            {
                #ifdef CATCHCHALLENGER_VERSION_ULTIMATE
                html+=QStringLiteral("<li>");
                if(CommonDatapack::commonDatapack.quests.value(i.key()).repeatable)
                    html+=imagesInterfaceRepeatableString;
                if(i.value().step>0)
                    html+=imagesInterfaceInProgressString;
                html+=DatapackClientLoader::datapackLoader.questsExtra.value(i.key()).name+QStringLiteral("</li>");
                #else
                html+=QStringLiteral("<li>")+DatapackClientLoader::datapackLoader.questsExtra.value(i.key()).name+QStringLiteral("</li>");
                #endif
            }
        }
    }
    html+=QStringLiteral("</ul>");
    if(html==QStringLiteral("<ul></ul>"))
        html=QStringLiteral("<i>None</i>");
    ui->finishedQuests->setHtml(html);
    on_questsList_itemSelectionChanged();
}

void BaseWindow::on_questsList_itemSelectionChanged()
{
    QList<QListWidgetItem *> items=ui->questsList->selectedItems();
    if(items.size()!=1)
    {
        ui->questDetails->setText(tr("Select a quest"));
        return;
    }
    if(!quests_to_id_graphical.contains(items.first()))
    {
        qDebug() << "Selected quest have not id";
        ui->questDetails->setText(tr("Select a quest"));
        return;
    }
    quint32 questId=quests_to_id_graphical.value(items.first());
    if(!quests.contains(questId))
    {
        qDebug() << "Selected quest is not into the player list";
        ui->questDetails->setText(tr("Select a quest"));
        return;
    }
    if(quests.value(questId).step==0 || quests.value(questId).step>DatapackClientLoader::datapackLoader.questsExtra.value(questId).steps.size())
    {
        qDebug() << "Selected quest step is out of range";
        ui->questDetails->setText(tr("Select a quest"));
        return;
    }
    const QString &stepDescription=DatapackClientLoader::datapackLoader.questsExtra.value(questId).steps.value(quests.value(questId).step-1)+"<br />";
    QString stepRequirements;
    {
        QList<Quest::Item> items=CommonDatapack::commonDatapack.quests.value(questId).steps.value(quests.value(questId).step-1).requirements.items;
        QStringList objects;
        int index=0;
        while(index<items.size())
        {
            QPixmap image;
            QString name;
            if(DatapackClientLoader::datapackLoader.itemsExtra.contains(items.at(index).item))
            {
                image=DatapackClientLoader::datapackLoader.itemsExtra.value(items.at(index).item).image;
                name=DatapackClientLoader::datapackLoader.itemsExtra.value(items.at(index).item).name;
            }
            else
            {
                image=DatapackClientLoader::datapackLoader.defaultInventoryImage();
                name=QStringLiteral("id: %1").arg(items.at(index).item);
            }

            image=image.scaled(24,24);
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            image.save(&buffer, "PNG");
            if(objects.size()<16)
            {
                if(items.at(index).quantity>1)
                    objects << QStringLiteral("<b>%2x</b> %3 <img src=\"data:image/png;base64,%1\" />").arg(QString(byteArray.toBase64())).arg(items.at(index).quantity).arg(name);
                else
                    objects << QStringLiteral("%2 <img src=\"data:image/png;base64,%1\" />").arg(QString(byteArray.toBase64())).arg(name);
            }
            index++;
        }
        if(objects.size()==16)
            objects << "...";
        stepRequirements+=tr("Step requirements: ")+QStringLiteral("%1<br />").arg(objects.join(", "));
    }
    QString finalRewards;
    if(DatapackClientLoader::datapackLoader.questsExtra.value(questId).showRewards
        #ifdef CATCHCHALLENGER_VERSION_ULTIMATE
            || true
        #endif
            )
    {
        finalRewards+=tr("Final rewards: ");
        {
            QList<Quest::Item> items=CommonDatapack::commonDatapack.quests.value(questId).rewards.items;
            QStringList objects;
            int index=0;
            while(index<items.size())
            {
                QPixmap image;
                QString name;
                if(DatapackClientLoader::datapackLoader.itemsExtra.contains(items.at(index).item))
                {
                    image=DatapackClientLoader::datapackLoader.itemsExtra.value(items.at(index).item).image;
                    name=DatapackClientLoader::datapackLoader.itemsExtra.value(items.at(index).item).name;
                }
                else
                {
                    image=DatapackClientLoader::datapackLoader.defaultInventoryImage();
                    name=QStringLiteral("id: %1").arg(items.at(index).item);
                }
                image=image.scaled(24,24);
                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                image.save(&buffer, "PNG");
                if(objects.size()<16)
                {
                    if(items.at(index).quantity>1)
                        objects << QStringLiteral("<b>%2x</b> %3 <img src=\"data:image/png;base64,%1\" />").arg(QString(byteArray.toBase64())).arg(items.at(index).quantity).arg(name);
                    else
                        objects << QStringLiteral("%2 <img src=\"data:image/png;base64,%1\" />").arg(QString(byteArray.toBase64())).arg(name);
                }
                index++;
            }
            if(objects.size()==16)
                objects << "...";
            finalRewards+=objects.join(", ")+"<br />";
        }
        {
            QList<ReputationRewards> reputationRewards=CommonDatapack::commonDatapack.quests.value(questId).rewards.reputation;
            QStringList reputations;
            int index=0;
            while(index<reputationRewards.size())
            {
                if(DatapackClientLoader::datapackLoader.reputationExtra.contains(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(reputationRewards.at(index).reputationId).name))
                {
                    if(reputationRewards.at(index).point<0)
                        reputations << tr("Less reputation for %1").arg(DatapackClientLoader::datapackLoader.reputationExtra.value(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(reputationRewards.at(index).reputationId).name).name);
                    if(reputationRewards.at(index).point>0)
                        reputations << tr("More reputation for %1").arg(DatapackClientLoader::datapackLoader.reputationExtra.value(CatchChallenger::CommonDatapack::commonDatapack.reputation.at(reputationRewards.at(index).reputationId).name).name);
                }
                index++;
            }
            if(reputations.size()>16)
            {
                while(reputations.size()>15)
                    reputations.removeLast();
                reputations << "...";
            }
            finalRewards+=reputations.join(", ")+"<br />";
        }
        {
            QList<ActionAllow> allowRewards=CommonDatapack::commonDatapack.quests.value(questId).rewards.allow;
            QStringList allows;
            int index=0;
            while(index<allowRewards.size())
            {
                if(allowRewards.contains(ActionAllow_Clan))
                    allows << tr("Add permission to create clan");
                index++;
            }
            if(allows.size()>16)
            {
                while(allows.size()>15)
                    allows.removeLast();
                allows << "...";
            }
            finalRewards+=allows.join(", ")+"<br />";
        }
    }

    ui->questDetails->setText(stepDescription+stepRequirements+finalRewards);
}

QListWidgetItem * BaseWindow::itemToGraphic(const quint32 &id,const quint32 &quantity)
{
    QListWidgetItem *item=new QListWidgetItem();
    item->setData(99,id);
    if(DatapackClientLoader::datapackLoader.itemsExtra.contains(id))
    {
        item->setIcon(DatapackClientLoader::datapackLoader.itemsExtra.value(id).image);
        if(quantity>1)
            item->setText(QString::number(quantity));
        item->setToolTip(DatapackClientLoader::datapackLoader.itemsExtra.value(id).name);
    }
    else
    {
        item->setIcon(DatapackClientLoader::datapackLoader.defaultInventoryImage());
        if(quantity>1)
            item->setText(QStringLiteral("id: %1 (x%2)").arg(id).arg(quantity));
        else
            item->setText(QStringLiteral("id: %1").arg(id));
    }
    return item;
}

void BaseWindow::updateTheWareHouseContent()
{
    if(!haveInventory || !datapackIsParsed)
        return;

    //inventory
    {
        ui->warehousePlayerInventory->clear();
        QHashIterator<quint16,quint32> i(items);
        while (i.hasNext()) {
            i.next();
            qint64 quantity=i.value();
            if(change_warehouse_items.contains(i.key()))
                quantity+=change_warehouse_items.value(i.key());
            if(quantity>0)
                ui->warehousePlayerInventory->addItem(itemToGraphic(i.key(),quantity));
        }
        QHashIterator<quint16,qint32> j(change_warehouse_items);
        while (j.hasNext()) {
            j.next();
            if(!items.contains(j.key()) && j.value()>0)
                ui->warehousePlayerInventory->addItem(itemToGraphic(j.key(),j.value()));
        }
    }

    qDebug() << QStringLiteral("ui->warehousePlayerInventory icon size: %1x%2").arg(ui->warehousePlayerInventory->iconSize().width()).arg(ui->warehousePlayerInventory->iconSize().height());

    //inventory warehouse
    {
        ui->warehousePlayerStoredInventory->clear();
        QHashIterator<quint16,quint32> i(warehouse_items);
        while (i.hasNext()) {
            i.next();
            qint64 quantity=i.value();
            if(change_warehouse_items.contains(i.key()))
                quantity-=change_warehouse_items.value(i.key());
            if(quantity>0)
                ui->warehousePlayerStoredInventory->addItem(itemToGraphic(i.key(),quantity));
        }
        QHashIterator<quint16,qint32> j(change_warehouse_items);
        while (j.hasNext()) {
            j.next();
            if(!warehouse_items.contains(j.key()) && j.value()<0)
                ui->warehousePlayerStoredInventory->addItem(itemToGraphic(j.key(),-j.value()));
        }
    }

    //cash
    ui->warehousePlayerCash->setText(tr("Cash: %1").arg(cash+temp_warehouse_cash));
    ui->warehousePlayerStoredCash->setText(tr("Cash: %1").arg(warehouse_cash-temp_warehouse_cash));

    //do before because the dispatch put into random of it
    ui->warehousePlayerStoredMonster->clear();
    ui->warehousePlayerMonster->clear();

    //monster
    {
        const QList<PlayerMonster> &playerMonster=CatchChallenger::ClientFightEngine::fightEngine.getPlayerMonster();
        int index=0;
        int size=playerMonster.size();
        while(index<size)
        {
            const PlayerMonster &monster=playerMonster.at(index);
            if(CatchChallenger::CommonDatapack::commonDatapack.monsters.contains(monster.monster))
            {
                QListWidgetItem *item=new QListWidgetItem();
                item->setText(tr("%1, level: %2")
                        .arg(DatapackClientLoader::datapackLoader.monsterExtra.value(monster.monster).name)
                        .arg(monster.level)
                        );
                item->setToolTip(DatapackClientLoader::datapackLoader.monsterExtra.value(monster.monster).description);
                item->setIcon(DatapackClientLoader::datapackLoader.monsterExtra.value(monster.monster).front);
                item->setData(99,monster.id);
                if(!monster_to_deposit.contains(monster.id) || monster_to_withdraw.contains(monster.id))
                    ui->warehousePlayerMonster->addItem(item);
                else
                    ui->warehousePlayerStoredMonster->addItem(item);
            }
            index++;
        }
    }

    //monster warehouse
    {
        int index=0;
        int size=warehouse_playerMonster.size();
        while(index<size)
        {
            const PlayerMonster &monster=warehouse_playerMonster.at(index);
            if(CatchChallenger::CommonDatapack::commonDatapack.monsters.contains(monster.monster))
            {
                QListWidgetItem *item=new QListWidgetItem();
                item->setText(tr("%1, level: %2")
                        .arg(DatapackClientLoader::datapackLoader.monsterExtra.value(monster.monster).name)
                        .arg(monster.level)
                        );
                item->setToolTip(DatapackClientLoader::datapackLoader.monsterExtra.value(monster.monster).description);
                item->setIcon(DatapackClientLoader::datapackLoader.monsterExtra.value(monster.monster).front);
                item->setData(99,monster.id);
                if(!monster_to_withdraw.contains(monster.id) || monster_to_deposit.contains(monster.id))
                    ui->warehousePlayerStoredMonster->addItem(item);
                else
                    ui->warehousePlayerMonster->addItem(item);
            }
            index++;
        }
    }

    //set the button enabled
    ui->warehouseWithdrawCash->setEnabled((warehouse_cash-temp_warehouse_cash)>0);
    ui->warehouseDepositCash->setEnabled((cash+temp_warehouse_cash)>0);
    ui->warehouseDepositItem->setEnabled(ui->warehousePlayerInventory->count()>0);
    ui->warehouseWithdrawItem->setEnabled(ui->warehousePlayerStoredInventory->count()>0);
    ui->warehouseDepositMonster->setEnabled(ui->warehousePlayerMonster->count()>1);
    ui->warehouseWithdrawMonster->setEnabled(ui->warehousePlayerStoredMonster->count()>0);
}

void BaseWindow::cityCapture(const quint32 &remainingTime,const quint8 &type)
{
    if(remainingTime==0)
    {
        nextCityCatchTimer.stop();
        return;//disabled
    }
    nextCityCatchTimer.start(remainingTime*1000);
    nextCatch=QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()+remainingTime*1000);
    city.capture.frenquency=(City::Capture::Frequency)type;
    city.capture.day=(City::Capture::Day)QDateTime::currentDateTime().addSecs(remainingTime).date().dayOfWeek();
    city.capture.hour=QDateTime::currentDateTime().addSecs(remainingTime).time().hour();
    city.capture.minute=QDateTime::currentDateTime().addSecs(remainingTime).time().minute();
}

void BaseWindow::animationFinished()
{
    if(animationWidget!=NULL)
    {
        delete animationWidget;
        animationWidget=NULL;
    }
    if(qQuickViewContainer!=NULL)
    {
        delete qQuickViewContainer;
        qQuickViewContainer=NULL;
    }
    if(previousAnimationWidget==ui->page_crafting)
    {
        ui->stackedWidget->setCurrentWidget(previousAnimationWidget);
        craftingAnimationObject->deleteLater();
        craftingAnimationObject=NULL;
    }
    else if(previousAnimationWidget==ui->page_map)
    {
        ui->stackedWidget->setCurrentWidget(previousAnimationWidget);
        if(baseMonsterEvolution!=NULL)
        {
            delete baseMonsterEvolution;
            baseMonsterEvolution=NULL;
        }
        if(targetMonsterEvolution!=NULL)
        {
            delete targetMonsterEvolution;
            targetMonsterEvolution=NULL;
        }
        if(idMonsterEvolution!=0)
        {
            CatchChallenger::ClientFightEngine::fightEngine.confirmEvolution(idMonsterEvolution);
            idMonsterEvolution=0;
            load_monsters();
        }
    }
    else
        qDebug() << "Unknown animation quit";
}

void BaseWindow::evolutionCanceled()
{
    if(animationWidget!=NULL)
    {
        delete animationWidget;
        animationWidget=NULL;
    }
    if(qQuickViewContainer!=NULL)
    {
        delete qQuickViewContainer;
        qQuickViewContainer=NULL;
    }
    ui->stackedWidget->setCurrentWidget(previousAnimationWidget);
    if(baseMonsterEvolution!=NULL)
    {
        delete baseMonsterEvolution;
        baseMonsterEvolution=NULL;
    }
    if(targetMonsterEvolution!=NULL)
    {
        delete targetMonsterEvolution;
        targetMonsterEvolution=NULL;
    }
    if(evolutionControl!=NULL)
    {
        delete evolutionControl;
        evolutionControl=NULL;
    }
    idMonsterEvolution=0;
    checkEvolution();
}

void BaseWindow::customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    //QString dt = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    //QString txt = QString("[%1] ").arg(dt);
    QString txt;

    switch (type)
    {
        default:
        case QtDebugMsg:
            txt = QStringLiteral("%1\n").arg(msg);
        break;
        case QtWarningMsg:
            txt = QStringLiteral("[Warning] %1\n").arg(msg);
        break;
        case QtCriticalMsg:
            txt = QStringLiteral("[Critical] %1\n").arg(msg);
        break;
        case QtFatalMsg:
            txt = QStringLiteral("[Fatal] %1\n").arg(msg);
            std::cout << txt.toUtf8().data();
        break;
    }
    std::cout << msg.toUtf8().data() << std::endl;

    if(BaseWindow::debugFileStatus==0)
    {
        if(!debugFile.isOpen())
        {
            QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
            debugFile.setFileName(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/LogFile.log");
            if(!debugFile.open(QIODevice::WriteOnly))
            {
                qDebug() << debugFile.errorString();
                BaseWindow::debugFileStatus=2;
                std::cout << static_cast<const char *>(msg.toLocal8Bit().constData()) << std::endl;
                return;
            }
            debugFile.resize(0);
        }
        BaseWindow::debugFileStatus=1;
    }
    if(debugFile.isOpen())
    {
        debugFile.write(txt.toUtf8());
        debugFile.flush();
    }
}
