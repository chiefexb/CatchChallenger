#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "../general/base/DatapackGeneralLoader.h"

#include <QFileDialog>
#include <QInputDialog>

using namespace CatchChallenger;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    settings=new QSettings(QCoreApplication::applicationDirPath()+QLatin1Literal("/server.properties"),QSettings::IniFormat);
    NormalServer::checkSettingsFile(settings);
    ui->setupUi(this);
    updateActionButton();
    qRegisterMetaType<Chat_type>("Chat_type");
    connect(&server,&NormalServer::is_started,          this,&MainWindow::server_is_started);
    connect(&server,&NormalServer::need_be_stopped,     this,&MainWindow::server_need_be_stopped);
    connect(&server,&NormalServer::need_be_restarted,   this,&MainWindow::server_need_be_restarted);
    connect(&server,&NormalServer::new_player_is_connected,this,&MainWindow::new_player_is_connected);
    connect(&server,&NormalServer::player_is_disconnected,this,&MainWindow::player_is_disconnected);
    connect(&server,&NormalServer::new_chat_message,    this,&MainWindow::new_chat_message);
    connect(&server,&NormalServer::error,               this,&MainWindow::server_error);
    connect(&server,&NormalServer::haveQuitForCriticalDatabaseQueryFailed,               this,&MainWindow::haveQuitForCriticalDatabaseQueryFailed);
    connect(&timer_update_the_info,&QTimer::timeout,    this,&MainWindow::update_the_info);
    connect(&check_latency,&QTimer::timeout,            this,&MainWindow::start_calculate_latency);
    connect(this,&MainWindow::record_latency,           this,&MainWindow::stop_calculate_latency,Qt::QueuedConnection);
    timer_update_the_info.start(200);
    check_latency.setSingleShot(true);
    check_latency.start(1000);
    need_be_restarted=false;
    need_be_closed=false;
    ui->tabWidget->setCurrentIndex(0);
    internal_currentLatency=0;
    load_settings();
    updateDbGroupbox();
    {
        events=DatapackGeneralLoader::loadEvents(QCoreApplication::applicationDirPath()+QLatin1Literal("/datapack/player/event.xml"));
        int index=0;
        while(index<events.size())
        {
            ui->programmedEventType->addItem(events.at(index).name);
            index++;
        }
    }
}

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
    need_be_closed=true;
    if(!server.isStopped())
        server_need_be_stopped();
    else
        QCoreApplication::exit();
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
    break;
    default:
    break;
    }
}

void MainWindow::on_lineEdit_returnPressed()
{
    ui->lineEdit->setText("");
}

void MainWindow::updateActionButton()
{
    ui->pushButton_server_start->setEnabled(server.isStopped());
    ui->pushButton_server_restart->setEnabled(server.isListen());
    ui->pushButton_server_stop->setEnabled(server.isListen());
}

void MainWindow::on_pushButton_server_start_clicked()
{
    ui->pushButton_server_start->setEnabled(false);
    send_settings();
    server.start_server();
}

void MainWindow::on_pushButton_server_stop_clicked()
{
    ui->pushButton_server_restart->setEnabled(false);
    ui->pushButton_server_stop->setEnabled(false);
    server.stop_server();
}

void MainWindow::on_pushButton_server_restart_clicked()
{
    ui->pushButton_server_restart->setEnabled(false);
    ui->pushButton_server_stop->setEnabled(false);
    need_be_restarted=true;
    server.stop_server();
}

void MainWindow::server_is_started(bool is_started)
{
    updateActionButton();
    if(need_be_closed || !isVisible())
    {
        QCoreApplication::exit();
        return;
    }
    if(!is_started)
    {
        clean_updated_info();
        if(need_be_restarted)
        {
            need_be_restarted=false;
            send_settings();
            server.start_server();
        }
        ui->displayPort->setText(QString());
    }
    else
        ui->displayPort->setText(QString::number(server.getNormalSettings().server_port));
}

void MainWindow::server_need_be_stopped()
{
    server.stop_server();
}

void MainWindow::server_need_be_restarted()
{
    need_be_restarted=true;
    server.stop_server();
}

void MainWindow::new_player_is_connected(Player_private_and_public_informations player)
{
    QIcon icon;
    switch(player.public_informations.type)
    {
        case Player_type_premium:
            icon=QIcon(QLatin1Literal(":/images/chat/premium.png"));
        break;
        case Player_type_gm:
            icon=QIcon(QLatin1Literal(":/images/chat/admin.png"));
        break;
        case Player_type_dev:
            icon=QIcon(QLatin1Literal(":/images/chat/developer.png"));
        break;
        default:
        break;
    }

    ui->listPlayer->addItem(new QListWidgetItem(icon,player.public_informations.pseudo));
    players << player;
}

void MainWindow::player_is_disconnected(QString pseudo)
{
    QList<QListWidgetItem *> tempList=ui->listPlayer->findItems(pseudo,Qt::MatchExactly);
    int index=0;
    while(index<tempList.size())
    {
        delete tempList.at(index);
        index++;
    }
    index=0;
    while(index<players.size())
    {
        if(players.at(index).public_informations.pseudo==pseudo)
        {
            players.removeAt(index);
            break;
        }
        index++;
    }
}

void MainWindow::new_chat_message(QString pseudo,Chat_type type,QString text)
{
    int index=0;
    while(index<players.size())
    {
        if(players.at(index).public_informations.pseudo==pseudo)
        {
            QString html=ui->textBrowserChat->toHtml();
            html+=ChatParsing::new_chat_message(players.at(index).public_informations.pseudo,players.at(index).public_informations.type,type,text);
            if(html.size()>1024*1024)
                html=html.mid(html.size()-1024*1024,1024*1024);
            ui->textBrowserChat->setHtml(html);
            return;
        }
        index++;
    }
    QMessageBox::information(this,"Warning","unable to locate the player");
}

void MainWindow::server_error(QString error)
{
    QMessageBox::information(this,"Warning",error);
}

void MainWindow::haveQuitForCriticalDatabaseQueryFailed()
{
    QMessageBox::information(this,"Warning","Unable to do critical database query to initialise the server");
}

void MainWindow::update_the_info()
{
    if(ui->listLatency->count()<=0)
        ui->listLatency->addItem(tr("%1ms").arg(internal_currentLatency));
    else
        ui->listLatency->item(0)->setText(tr("%1ms").arg(internal_currentLatency));
    if(server.isListen())
    {
        quint16 player_current,player_max;
        player_current=server.player_current();
        player_max=server.player_max();
        ui->label_player->setText(QStringLiteral("%1/%2").arg(player_current).arg(player_max));
        ui->progressBar_player->setMaximum(player_max);
        ui->progressBar_player->setValue(player_current);
    }
    else
    {
        ui->progressBar_player->setValue(0);
    }
}

QString MainWindow::sizeToString(double size)
{
    if(size<1024)
        return QString::number(size)+tr("B");
    if((size=size/1024)<1024)
        return adaptString(size)+tr("KB");
    if((size=size/1024)<1024)
        return adaptString(size)+tr("MB");
    if((size=size/1024)<1024)
        return adaptString(size)+tr("GB");
    if((size=size/1024)<1024)
        return adaptString(size)+tr("TB");
    if((size=size/1024)<1024)
        return adaptString(size)+tr("PB");
    if((size=size/1024)<1024)
        return adaptString(size)+tr("EB");
    if((size=size/1024)<1024)
        return adaptString(size)+tr("ZB");
    if((size=size/1024)<1024)
        return adaptString(size)+tr("YB");
    return tr("Too big");
}

QString MainWindow::adaptString(float size)
{
    if(size>=100)
        return QString::number(size,'f',0);
    else
        return QString::number(size,'g',3);
}


void MainWindow::start_calculate_latency()
{
    time_latency.restart();
    emit record_latency();
}

void MainWindow::stop_calculate_latency()
{
    internal_currentLatency=time_latency.elapsed();
    check_latency.start();
}

void MainWindow::benchmark_result(int latency,double TX_speed,double RX_speed,double TX_size,double RX_size,double second)
{
    updateActionButton();
    clean_updated_info();
    QMessageBox::information(this,"benchmark_map",tr("The latency of the benchmark: %1\nTX_speed: %2/s, RX_speed %3/s\nTX_size: %4, RX_size: %5, in %6s\nThis latency is cumulated latency of different point. That's not show the database performance.")
                 .arg(latency)
                 .arg(sizeToString(TX_speed))
                 .arg(sizeToString(RX_speed))
                 .arg(sizeToString(TX_size))
                 .arg(sizeToString(RX_size))
                 .arg(second)
                 );
}

void MainWindow::clean_updated_info()
{
    ui->label_player->setText("?/?");
    ui->listLatency->clear();
}

void MainWindow::load_settings()
{
    // --------------------------------------------------
    ui->max_player->setValue(settings->value(QLatin1Literal("max-players")).toUInt());
    ui->server_ip->setText(settings->value(QLatin1Literal("server-ip")).toString());
    ui->pvp->setChecked(settings->value(QLatin1Literal("pvp")).toBool());
    ui->sendPlayerNumber->setChecked(settings->value(QLatin1Literal("sendPlayerNumber")).toBool());
    ui->server_port->setValue(settings->value(QLatin1Literal("server-port")).toUInt());
    ui->tolerantMode->setChecked(settings->value(QLatin1Literal("tolerantMode")).toBool());
    ui->forceClientToSendAtMapChange->setChecked(settings->value(QLatin1Literal("forceClientToSendAtMapChange")).toBool());
    ui->useSsl->setChecked(settings->value(QLatin1Literal("useSsl")).toBool());
    ui->autoLearn->setChecked(settings->value(QLatin1Literal("autoLearn")).toBool());
    ui->useSP->setChecked(settings->value(QLatin1Literal("useSP")).toBool());
    if(settings->value(QLatin1Literal("compression")).toString()==QLatin1Literal("none"))
        ui->compression->setCurrentIndex(0);
    else if(settings->value(QLatin1Literal("compression")).toString()==QLatin1Literal("xz"))
        ui->compression->setCurrentIndex(2);
    else
        ui->compression->setCurrentIndex(1);
    ui->maxPlayerMonsters->setValue(settings->value(QLatin1Literal("maxPlayerMonsters")).toUInt());
    ui->maxWarehousePlayerMonsters->setValue(settings->value(QLatin1Literal("maxWarehousePlayerMonsters")).toUInt());
    ui->maxPlayerItems->setValue(settings->value(QLatin1Literal("maxPlayerItems")).toUInt());
    ui->maxWarehousePlayerItems->setValue(settings->value(QLatin1Literal("maxWarehousePlayerItems")).toUInt());
    ui->min_character->setValue(settings->value(QLatin1Literal("min_character")).toUInt());
    ui->max_character->setValue(settings->value(QLatin1Literal("max_character")).toUInt());
    ui->max_pseudo_size->setValue(settings->value(QLatin1Literal("max_pseudo_size")).toUInt());
    ui->character_delete_time->setValue(settings->value(QLatin1Literal("character_delete_time")).toUInt()/3600);
    ui->automatic_account_creation->setChecked(settings->value(QLatin1Literal("automatic_account_creation")).toBool());
    ui->anonymous->setChecked(settings->value(QLatin1Literal("anonymous")).toBool());
    ui->min_character->setMaximum(ui->max_character->value());
    ui->max_character->setMinimum(ui->min_character->value());
    ui->server_message->setPlainText(settings->value(QLatin1Literal("server_message")).toString());
    ui->proxy->setText(settings->value(QLatin1Literal("proxy")).toString());
    ui->proxy_port->setValue(settings->value(QLatin1Literal("proxy_port")).toUInt());
    ui->httpDatapackMirror->setText(settings->value(QLatin1Literal("httpDatapackMirror")).toString());
    {
        const qint32 &datapackCache=settings->value(QLatin1Literal("datapackCache")).toInt();
        if(datapackCache<0)
        {
            ui->datapack_cache->setChecked(false);
            ui->datapack_cache_timeout_checkbox->setChecked(false);
            ui->datapack_cache_timeout->setValue(30);
            ui->datapack_cache_timeout_checkbox->setEnabled(ui->datapack_cache->isChecked());
            ui->datapack_cache_timeout->setEnabled(ui->datapack_cache->isChecked() && ui->datapack_cache_timeout_checkbox->isChecked());
        }
        else if(datapackCache==0)
        {
            ui->datapack_cache->setChecked(true);
            ui->datapack_cache_timeout_checkbox->setChecked(false);
            ui->datapack_cache_timeout->setValue(30);
            ui->datapack_cache_timeout_checkbox->setEnabled(ui->datapack_cache->isChecked());
            ui->datapack_cache_timeout->setEnabled(ui->datapack_cache->isChecked() && ui->datapack_cache_timeout_checkbox->isChecked());
        }
        else
        {
            ui->datapack_cache->setChecked(true);
            ui->datapack_cache_timeout_checkbox->setChecked(true);
            ui->datapack_cache_timeout->setValue(datapackCache);
            ui->datapack_cache_timeout_checkbox->setEnabled(ui->datapack_cache->isChecked());
            ui->datapack_cache_timeout->setEnabled(ui->datapack_cache->isChecked() && ui->datapack_cache_timeout_checkbox->isChecked());
        }
    }
    {
        settings->beginGroup(QLatin1Literal("programmedEvent"));
            const QStringList &tempListType=settings->childGroups();
            int indexType=0;
            while(indexType<tempListType.size())
            {
                const QString &type=tempListType.at(indexType);
                settings->beginGroup(type);
                    const QStringList &tempList=settings->childGroups();
                    int index=0;
                    while(index<tempList.size())
                    {
                        const QString &groupName=tempList.at(index);
                        settings->beginGroup(groupName);
                        if(settings->contains(QLatin1Literal("value")) && settings->contains(QLatin1Literal("cycle")) && settings->contains(QLatin1Literal("offset")))
                        {
                            ServerSettings::ProgrammedEvent event;
                            event.value=settings->value(QLatin1Literal("value")).toString();
                            bool ok;
                            event.cycle=settings->value(QLatin1Literal("cycle")).toUInt(&ok);
                            if(!ok)
                                event.cycle=0;
                            event.offset=settings->value(QLatin1Literal("offset")).toUInt(&ok);
                            if(!ok)
                                event.offset=0;
                            if(event.cycle>0)
                                programmedEventList[type][groupName]=event;
                        }
                        settings->endGroup();
                        index++;
                    }
                settings->endGroup();
                indexType++;
            }
        settings->endGroup();
        if(ui->programmedEventType->count()>0)
            on_programmedEventType_currentIndexChanged(0);
    }
    {
        #ifdef Q_OS_LINUX
        bool tcpNodelay=false;
        bool tcpCork=true;
        settings->beginGroup(QLatin1Literal("Linux"));
        tcpCork=settings->value(QLatin1Literal("tcpCork")).toBool();
        tcpNodelay=settings->value(QLatin1Literal("tcpNodelay")).toBool();
        settings->endGroup();
        ui->linux_socket_cork->setChecked(tcpCork);
        ui->tcpNodelay->setChecked(tcpNodelay);
        #else
        ui->linux_socket_cork->setEnabled(false);
        ui->tcpNodelay->setEnabled(false);
        #endif
    }
    if(settings->value(QLatin1Literal("forcedSpeed")).toUInt()==0)
    {
        ui->forceSpeed->setChecked(true);
        ui->speed->setEnabled(false);
    }
    else
    {
        ui->forceSpeed->setChecked(false);
        ui->speed->setValue(settings->value(QLatin1Literal("forcedSpeed")).toUInt());
        ui->speed->setEnabled(true);
    }
    ui->dontSendPseudo->setChecked(settings->value(QLatin1Literal("dontSendPseudo")).toBool());
    ui->dontSendPlayerType->setChecked(settings->value(QLatin1Literal("dontSendPlayerType")).toBool());

    quint32 tempValue=0;
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm"));
    tempValue=settings->value(QLatin1Literal("MapVisibilityAlgorithm")).toUInt();
    settings->endGroup();
    if(tempValue<(quint32)ui->MapVisibilityAlgorithm->count())
        ui->MapVisibilityAlgorithm->setCurrentIndex(tempValue);
    ui->groupBoxMapVisibilityAlgorithmSimple->setEnabled(tempValue==0);
    ui->groupBoxMapVisibilityAlgorithmWithBorder->setEnabled(tempValue==2);

    {
        quint32 reshow=0;
        settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-Simple"));
        tempValue=settings->value(QLatin1Literal("Max")).toUInt();
        reshow=settings->value(QLatin1Literal("Reshow")).toUInt();
        if(reshow>tempValue)
        {
            DebugClass::debugConsole("Reshow number corrected");
            reshow=tempValue;
            settings->setValue(QLatin1Literal("Reshow"),reshow);
        }
        settings->endGroup();
        ui->MapVisibilityAlgorithmSimpleMax->setValue(tempValue);
        ui->MapVisibilityAlgorithmSimpleReshow->setValue(reshow);
        ui->MapVisibilityAlgorithmSimpleReshow->setMaximum(ui->MapVisibilityAlgorithmSimpleMax->value());
        ui->MapVisibilityAlgorithmSimpleReemit->setChecked(settings->value(QLatin1Literal("Reemit")).toBool());
    }
    {
        quint32 tempValueWithBorder=0;
        quint32 reshowWithBorder=0;
        quint32 reshow=0;
        settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-WithBorder"));
        tempValueWithBorder=settings->value(QLatin1Literal("MaxWithBorder")).toUInt();
        reshowWithBorder=settings->value(QLatin1Literal("ReshowWithBorder")).toUInt();
        tempValue=settings->value(QLatin1Literal("Max")).toUInt();
        reshow=settings->value(QLatin1Literal("Reshow")).toUInt();
        if(reshow>tempValue)
        {
            DebugClass::debugConsole("Reshow number corrected");
            reshow=tempValue;
            settings->setValue(QLatin1Literal("Reshow"),reshow);
        }
        if(reshowWithBorder>tempValueWithBorder)
        {
            DebugClass::debugConsole("ReshowWithBorder number corrected");
            reshowWithBorder=tempValueWithBorder;
            settings->setValue(QLatin1Literal("ReshowWithBorder"),reshow);
        }
        if(tempValueWithBorder>tempValue)
        {
            DebugClass::debugConsole("MaxWithBorder number corrected");
            tempValueWithBorder=tempValue;
            settings->setValue(QLatin1Literal("MaxWithBorder"),reshow);
        }
        if(reshowWithBorder>reshow)
        {
            DebugClass::debugConsole("ReshowWithBorder number corrected");
            reshowWithBorder=reshow;
            settings->setValue(QLatin1Literal("ReshowWithBorder"),reshow);
        }
        settings->endGroup();
        ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->setValue(tempValueWithBorder);
        ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setValue(reshowWithBorder);
        ui->MapVisibilityAlgorithmWithBorderMax->setValue(tempValue);
        ui->MapVisibilityAlgorithmWithBorderReshow->setValue(reshow);
        ui->MapVisibilityAlgorithmWithBorderReshow->setMaximum(ui->MapVisibilityAlgorithmWithBorderMax->value());
        ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderMax->value());
        if(ui->MapVisibilityAlgorithmWithBorderReshow->value()>ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value())
            ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value());
        else
            ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderReshow->value());
    }

    {
        settings->beginGroup(QLatin1Literal("rates"));
        double rates_xp_normal=settings->value(QLatin1Literal("xp_normal")).toFloat();
        double rates_gold_normal=settings->value(QLatin1Literal("gold_normal")).toFloat();
        double rates_xp_pow_normal=settings->value(QLatin1Literal("xp_pow_normal")).toFloat();
        double rates_drop_normal=settings->value(QLatin1Literal("drop_normal")).toFloat();
        settings->endGroup();

        ui->rates_xp_normal->setValue(rates_xp_normal);
        ui->rates_gold_normal->setValue(rates_gold_normal);
        ui->rates_xp_pow_normal->setValue(rates_xp_pow_normal);
        ui->rates_drop_normal->setValue(rates_drop_normal);
    }

    {
        settings->beginGroup(QLatin1Literal("chat"));
        bool chat_allow_all=settings->value(QLatin1Literal("allow-all")).toBool();
        bool chat_allow_local=settings->value(QLatin1Literal("allow-local")).toBool();
        bool chat_allow_private=settings->value(QLatin1Literal("allow-private")).toBool();
        bool chat_allow_clan=settings->value(QLatin1Literal("allow-clan")).toBool();
        settings->endGroup();

        ui->chat_allow_all->setChecked(chat_allow_all);
        ui->chat_allow_local->setChecked(chat_allow_local);
        ui->chat_allow_private->setChecked(chat_allow_private);
        ui->chat_allow_clan->setChecked(chat_allow_clan);
    }

    settings->beginGroup(QLatin1Literal("db"));
    QString db_type=settings->value(QLatin1Literal("type")).toString();
    QString db_mysql_host=settings->value(QLatin1Literal("mysql_host")).toString();
    QString db_mysql_login=settings->value(QLatin1Literal("mysql_login")).toString();
    QString db_mysql_pass=settings->value(QLatin1Literal("mysql_pass")).toString();
    QString db_mysql_base=settings->value(QLatin1Literal("mysql_db")).toString();
    QString db_fight_sync=settings->value(QLatin1Literal("db_fight_sync")).toString();
    bool positionTeleportSync=settings->value(QLatin1Literal("positionTeleportSync")).toBool();
    quint32 secondToPositionSync=settings->value(QLatin1Literal("secondToPositionSync")).toUInt();

    if(!settings->contains(QLatin1Literal("db_fight_sync")))
        settings->setValue(QLatin1Literal("db_fight_sync"),"FightSync_AtTheEndOfBattle");
    quint32 tryInterval;
    quint32 considerDownAfterNumberOfTry;
    tryInterval=settings->value(QLatin1Literal("tryInterval")).toUInt();
    considerDownAfterNumberOfTry=settings->value(QLatin1Literal("considerDownAfterNumberOfTry")).toUInt();
    settings->endGroup();
    ui->tryInterval->setValue(tryInterval);
    ui->considerDownAfterNumberOfTry->setValue(considerDownAfterNumberOfTry);

    settings->beginGroup(QLatin1Literal("DDOS"));
    ui->DDOSwaitBeforeConnectAfterKick->setValue(settings->value(QLatin1Literal("waitBeforeConnectAfterKick")).toUInt());
    ui->DDOScomputeAverageValueNumberOfValue->setValue(settings->value(QLatin1Literal("computeAverageValueNumberOfValue")).toUInt());
    ui->DDOScomputeAverageValueTimeInterval->setValue(settings->value(QLatin1Literal("computeAverageValueTimeInterval")).toUInt());
    ui->DDOSkickLimitMove->setValue(settings->value(QLatin1Literal("kickLimitMove")).toUInt());
    ui->DDOSkickLimitChat->setValue(settings->value(QLatin1Literal("kickLimitChat")).toUInt());
    ui->DDOSkickLimitOther->setValue(settings->value(QLatin1Literal("kickLimitOther")).toUInt());
    ui->DDOSdropGlobalChatMessageGeneral->setValue(settings->value(QLatin1Literal("dropGlobalChatMessageGeneral")).toUInt());
    ui->DDOSdropGlobalChatMessageLocalClan->setValue(settings->value(QLatin1Literal("dropGlobalChatMessageLocalClan")).toUInt());
    ui->DDOSdropGlobalChatMessagePrivate->setValue(settings->value(QLatin1Literal("dropGlobalChatMessagePrivate")).toUInt());
    settings->endGroup();

    if(db_type==QLatin1Literal("mysql"))
        ui->db_type->setCurrentIndex(0);
    else if(db_type==QLatin1Literal("sqlite"))
        ui->db_type->setCurrentIndex(1);
    else if(db_type==QLatin1Literal("postgresql"))
        ui->db_type->setCurrentIndex(2);
    else
        ui->db_type->setCurrentIndex(1);
    ui->db_mysql_host->setText(db_mysql_host);
    ui->db_mysql_login->setText(db_mysql_login);
    ui->db_mysql_pass->setText(db_mysql_pass);
    ui->db_mysql_base->setText(db_mysql_base);
    if(db_fight_sync==QLatin1Literal("FightSync_AtEachTurn"))
        ui->db_fight_sync->setCurrentIndex(0);
    else if(db_fight_sync==QLatin1Literal("FightSync_AtTheEndOfBattle"))
        ui->db_fight_sync->setCurrentIndex(1);
    else if(db_fight_sync==QLatin1Literal("FightSync_AtTheDisconnexion"))
        ui->db_fight_sync->setCurrentIndex(2);
    else
        ui->db_fight_sync->setCurrentIndex(0);
    ui->positionTeleportSync->setChecked(positionTeleportSync);
    ui->secondToPositionSync->setValue(secondToPositionSync);

    ui->db_sqlite_file->setText(QCoreApplication::applicationDirPath()+QLatin1Literal("/catchchallenger.db.sqlite"));

    {
        settings->beginGroup(QLatin1Literal("city"));
        if(!settings->contains(QLatin1Literal("capture_frequency")))
            settings->setValue(QLatin1Literal("capture_frequency"),QLatin1Literal("day"));
        int capture_frequency_int=0;
        if(settings->value(QLatin1Literal("capture_frequency")).toString()==QLatin1Literal("week"))
            capture_frequency_int=0;
        else if(settings->value(QLatin1Literal("capture_frequency")).toString()==QLatin1Literal("month"))
            capture_frequency_int=1;
        update_capture();
        int capture_day_int=0;
        if(settings->value(QLatin1Literal("capture_day")).toString()==QLatin1Literal("monday"))
            capture_day_int=0;
        else if(settings->value(QLatin1Literal("capture_day")).toString()==QLatin1Literal("tuesday"))
            capture_day_int=1;
        else if(settings->value(QLatin1Literal("capture_day")).toString()==QLatin1Literal("wednesday"))
            capture_day_int=2;
        else if(settings->value(QLatin1Literal("capture_day")).toString()==QLatin1Literal("thursday"))
            capture_day_int=3;
        else if(settings->value(QLatin1Literal("capture_day")).toString()==QLatin1Literal("friday"))
            capture_day_int=4;
        else if(settings->value(QLatin1Literal("capture_day")).toString()==QLatin1Literal("saturday"))
            capture_day_int=5;
        else if(settings->value(QLatin1Literal("capture_day")).toString()==QLatin1Literal("sunday"))
            capture_day_int=6;
        int capture_time_hours=0,capture_time_minutes=0;
        QStringList capture_time_string_list=settings->value(QLatin1Literal("capture_time")).toString().split(QLatin1Literal(":"));
        if(capture_time_string_list.size()!=2)
            settings->setValue(QLatin1Literal("capture_time"),QLatin1Literal("0:0"));
        else
        {
            bool ok;
            capture_time_hours=capture_time_string_list.first().toUInt(&ok);
            if(!ok)
                settings->setValue(QLatin1Literal("capture_time"),QLatin1Literal("0:0"));
            else
            {
                capture_time_minutes=capture_time_string_list.last().toUInt(&ok);
                if(!ok)
                    settings->setValue(QLatin1Literal("capture_time"),QLatin1Literal("0:0"));
            }
        }
        settings->endGroup();
        ui->comboBox_city_capture_frequency->setCurrentIndex(capture_frequency_int);
        ui->comboBox_city_capture_day->setCurrentIndex(capture_day_int);
        ui->timeEdit_city_capture_time->setTime(QTime(capture_time_hours,capture_time_minutes));
    }

    send_settings();
}

void MainWindow::send_settings()
{
    ServerSettings formatedServerSettings=server.getSettings();
    NormalServerSettings formatedServerNormalSettings=server.getNormalSettings();

    //common var
    CommonSettings::commonSettings.min_character					= ui->min_character->value();
    CommonSettings::commonSettings.max_character					= ui->max_character->value();
    CommonSettings::commonSettings.max_pseudo_size					= ui->max_pseudo_size->value();
    CommonSettings::commonSettings.character_delete_time			= ui->character_delete_time->value()*3600;

    if(!ui->forceSpeed->isChecked())
        CommonSettings::commonSettings.forcedSpeed					= 0;
    else
        CommonSettings::commonSettings.forcedSpeed					= ui->speed->value();
    formatedServerSettings.dontSendPlayerType                       = ui->dontSendPlayerType->isChecked();
    CommonSettings::commonSettings.dontSendPseudo					= ui->dontSendPseudo->isChecked();
    CommonSettings::commonSettings.forceClientToSendAtMapChange		= ui->forceClientToSendAtMapChange->isChecked();
    CommonSettings::commonSettings.useSP                            = ui->useSP->isChecked();
    CommonSettings::commonSettings.autoLearn                        = ui->autoLearn->isChecked() && !ui->useSP->isChecked();
    CommonSettings::commonSettings.maxPlayerMonsters                = ui->maxPlayerMonsters->value();
    CommonSettings::commonSettings.maxWarehousePlayerMonsters       = ui->maxWarehousePlayerMonsters->value();
    CommonSettings::commonSettings.maxPlayerItems                   = ui->maxPlayerItems->value();
    CommonSettings::commonSettings.maxWarehousePlayerItems          = ui->maxWarehousePlayerItems->value();

    //the listen
    formatedServerNormalSettings.server_port			= ui->server_port->value();
    formatedServerNormalSettings.server_ip				= ui->server_ip->text();
    formatedServerNormalSettings.proxy    				= ui->proxy->text();
    formatedServerNormalSettings.proxy_port				= ui->proxy_port->value();
    formatedServerNormalSettings.useSsl					= ui->useSsl->isChecked();
    formatedServerSettings.anonymous					= ui->anonymous->isChecked();
    formatedServerSettings.server_message				= ui->server_message->toPlainText();
    CommonSettings::commonSettings.httpDatapackMirror    		= ui->httpDatapackMirror->text();
    if(!ui->datapack_cache->isChecked())
        formatedServerSettings.datapackCache			= -1;
    else if(!ui->datapack_cache_timeout_checkbox->isChecked())
        formatedServerSettings.datapackCache			= 0;
    else
        formatedServerSettings.datapackCache			= ui->datapack_cache_timeout->value();
    #ifdef Q_OS_LINUX
    CommonSettings::commonSettings.tcpCork  = ui->linux_socket_cork->isChecked();
    formatedServerNormalSettings.tcpNodelay  = ui->tcpNodelay->isChecked();
    #endif

    //ddos
    CommonSettings::commonSettings.waitBeforeConnectAfterKick=ui->DDOSwaitBeforeConnectAfterKick->value();
    formatedServerSettings.ddos.computeAverageValueNumberOfValue=ui->DDOScomputeAverageValueNumberOfValue->value();
    formatedServerSettings.ddos.computeAverageValueTimeInterval=ui->DDOScomputeAverageValueTimeInterval->value();
    formatedServerSettings.ddos.kickLimitMove=ui->DDOSkickLimitMove->value();
    formatedServerSettings.ddos.kickLimitChat=ui->DDOSkickLimitChat->value();
    formatedServerSettings.ddos.kickLimitOther=ui->DDOSkickLimitOther->value();
    formatedServerSettings.ddos.dropGlobalChatMessageGeneral=ui->DDOSdropGlobalChatMessageGeneral->value();
    formatedServerSettings.ddos.dropGlobalChatMessageLocalClan=ui->DDOSdropGlobalChatMessageLocalClan->value();
    formatedServerSettings.ddos.dropGlobalChatMessagePrivate=ui->DDOSdropGlobalChatMessagePrivate->value();
    formatedServerSettings.programmedEventList=programmedEventList;

    //fight
    formatedServerSettings.pvp			= ui->pvp->isChecked();
    formatedServerSettings.sendPlayerNumber		= ui->sendPlayerNumber->isChecked();

    //compression
    switch(ui->compression->currentIndex())
    {
        case 0:
        formatedServerSettings.compressionType=CatchChallenger::CompressionType_None;
        break;
        default:
        case 1:
        formatedServerSettings.compressionType=CatchChallenger::CompressionType_Zlib;
        break;
        case 2:
        formatedServerSettings.compressionType=CatchChallenger::CompressionType_Xz;
        break;
    }

    //rates
    CommonSettings::commonSettings.rates_xp			= ui->rates_xp_normal->value();
    CommonSettings::commonSettings.rates_gold		= ui->rates_gold_normal->value();
    CommonSettings::commonSettings.rates_xp_pow     = ui->rates_xp_pow_normal->value();
    CommonSettings::commonSettings.rates_drop		= ui->rates_drop_normal->value();

    //chat allowed
    CommonSettings::commonSettings.chat_allow_all		= ui->chat_allow_all->isChecked();
    CommonSettings::commonSettings.chat_allow_local		= ui->chat_allow_local->isChecked();
    CommonSettings::commonSettings.chat_allow_private		= ui->chat_allow_private->isChecked();
    CommonSettings::commonSettings.chat_allow_clan		= ui->chat_allow_clan->isChecked();

    switch(ui->db_type->currentIndex())
    {
        default:
        case 0:
            formatedServerSettings.database.type					= ServerSettings::Database::DatabaseType_Mysql;
        break;
        case 1:
            formatedServerSettings.database.type					= ServerSettings::Database::DatabaseType_SQLite;
        break;
        case 2:
            formatedServerSettings.database.type					= ServerSettings::Database::DatabaseType_PostgreSQL;
        break;
    }
    switch(formatedServerSettings.database.type)
    {
        default:
        case ServerSettings::Database::DatabaseType_Mysql:
            formatedServerSettings.database.mysql.host				= ui->db_mysql_host->text();
            formatedServerSettings.database.mysql.db				= ui->db_mysql_base->text();
            formatedServerSettings.database.mysql.login				= ui->db_mysql_login->text();
            formatedServerSettings.database.mysql.pass				= ui->db_mysql_pass->text();
        break;
        case ServerSettings::Database::DatabaseType_SQLite:
            formatedServerSettings.database.sqlite.file				= ui->db_sqlite_file->text();
        break;
        case ServerSettings::Database::DatabaseType_PostgreSQL:
            formatedServerSettings.database.mysql.host				= ui->db_mysql_host->text();
            formatedServerSettings.database.mysql.db				= ui->db_mysql_base->text();
            formatedServerSettings.database.mysql.login				= ui->db_mysql_login->text();
            formatedServerSettings.database.mysql.pass				= ui->db_mysql_pass->text();
        break;
    }
    formatedServerSettings.database.fightSync                       = (ServerSettings::Database::FightSync)ui->db_fight_sync->currentIndex();
    formatedServerSettings.database.positionTeleportSync=ui->positionTeleportSync->isChecked();
    formatedServerSettings.database.secondToPositionSync=ui->secondToPositionSync->value();
    formatedServerSettings.database.tryInterval=ui->tryInterval->value();
    formatedServerSettings.database.considerDownAfterNumberOfTry=ui->considerDownAfterNumberOfTry->value();

    //connection
    formatedServerSettings.automatic_account_creation   = ui->automatic_account_creation->isChecked();
    formatedServerSettings.max_players					= ui->max_player->value();
    formatedServerSettings.tolerantMode                 = ui->tolerantMode->isChecked();

    //visibility algorithm
    switch(ui->MapVisibilityAlgorithm->currentIndex())
    {
        case 0:
            formatedServerSettings.mapVisibility.mapVisibilityAlgorithm		= MapVisibilityAlgorithmSelection_Simple;
        break;
        case 1:
            formatedServerSettings.mapVisibility.mapVisibilityAlgorithm		= MapVisibilityAlgorithmSelection_None;
        break;
        case 2:
            formatedServerSettings.mapVisibility.mapVisibilityAlgorithm		= MapVisibilityAlgorithmSelection_WithBorder;
        break;
        default:
            formatedServerSettings.mapVisibility.mapVisibilityAlgorithm		= MapVisibilityAlgorithmSelection_Simple;
        break;
    }

    formatedServerSettings.mapVisibility.simple.max                 = ui->MapVisibilityAlgorithmSimpleMax->value();
    formatedServerSettings.mapVisibility.simple.reshow              = ui->MapVisibilityAlgorithmSimpleReshow->value();
    formatedServerSettings.mapVisibility.simple.reemit              = ui->MapVisibilityAlgorithmSimpleReemit->isChecked();
    formatedServerSettings.mapVisibility.withBorder.maxWithBorder	= ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value();
    formatedServerSettings.mapVisibility.withBorder.reshowWithBorder= ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->value();
    formatedServerSettings.mapVisibility.withBorder.max				= ui->MapVisibilityAlgorithmWithBorderMax->value();
    formatedServerSettings.mapVisibility.withBorder.reshow			= ui->MapVisibilityAlgorithmWithBorderReshow->value();

    switch(ui->comboBox_city_capture_frequency->currentIndex())
    {
        default:
        case 0:
            formatedServerSettings.city.capture.frenquency=City::Capture::Frequency_week;
        break;
        case 1:
            formatedServerSettings.city.capture.frenquency=City::Capture::Frequency_month;
        break;
    }
    switch(ui->comboBox_city_capture_day->currentIndex())
    {
        default:
        case 0:
            formatedServerSettings.city.capture.day=City::Capture::Monday;
        break;
        case 1:
            formatedServerSettings.city.capture.day=City::Capture::Tuesday;
        break;
        case 2:
            formatedServerSettings.city.capture.day=City::Capture::Wednesday;
        break;
        case 3:
            formatedServerSettings.city.capture.day=City::Capture::Thursday;
        break;
        case 4:
            formatedServerSettings.city.capture.day=City::Capture::Friday;
        break;
        case 5:
            formatedServerSettings.city.capture.day=City::Capture::Saturday;
        break;
        case 6:
            formatedServerSettings.city.capture.day=City::Capture::Sunday;
        break;
    }
    QTime time=ui->timeEdit_city_capture_time->time();
    formatedServerSettings.city.capture.hour=time.hour();
    formatedServerSettings.city.capture.minute=time.minute();

    server.setSettings(formatedServerSettings);
    server.setNormalSettings(formatedServerNormalSettings);
}

void MainWindow::on_max_player_valueChanged(int arg1)
{
    settings->setValue(QLatin1Literal("max-players"),arg1);
}

void MainWindow::on_server_ip_editingFinished()
{
    settings->setValue(QLatin1Literal("server-ip"),ui->server_ip->text());
}

void MainWindow::on_pvp_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    settings->setValue(QLatin1Literal("pvp"),ui->pvp->isChecked());
}

void MainWindow::on_server_port_valueChanged(int arg1)
{
    settings->setValue(QLatin1Literal("server-port"),arg1);
}

void MainWindow::on_rates_xp_normal_valueChanged(double arg1)
{
    settings->beginGroup(QLatin1Literal("rates"));
    settings->setValue(QLatin1Literal("xp_normal"),arg1);
    settings->endGroup();
}

void MainWindow::on_rates_gold_normal_valueChanged(double arg1)
{
    settings->beginGroup(QLatin1Literal("rates"));
    settings->setValue(QLatin1Literal("gold_normal"),arg1);
    settings->endGroup();
}

void MainWindow::on_chat_allow_all_toggled(bool checked)
{
    settings->beginGroup(QLatin1Literal("chat"));
    settings->setValue(QLatin1Literal("allow-all"),checked);
    settings->endGroup();
}

void MainWindow::on_chat_allow_local_toggled(bool checked)
{
    settings->beginGroup(QLatin1Literal("chat"));
    settings->setValue(QLatin1Literal("allow-local"),checked);
    settings->endGroup();
}

void MainWindow::on_chat_allow_private_toggled(bool checked)
{
    settings->beginGroup(QLatin1Literal("chat"));
    settings->setValue(QLatin1Literal("allow-private"),checked);
    settings->endGroup();
}

void MainWindow::on_chat_allow_clan_toggled(bool checked)
{
    settings->beginGroup(QLatin1Literal("chat"));
    settings->setValue(QLatin1Literal("allow-clan"),checked);
    settings->endGroup();
}

void MainWindow::on_db_mysql_host_editingFinished()
{
    settings->beginGroup(QLatin1Literal("db"));
    settings->setValue(QLatin1Literal("mysql_host"),ui->db_mysql_host->text());
    settings->endGroup();
}

void MainWindow::on_db_mysql_login_editingFinished()
{
    settings->beginGroup(QLatin1Literal("db"));
    settings->setValue(QLatin1Literal("mysql_login"),ui->db_mysql_login->text());
    settings->endGroup();
}

void MainWindow::on_db_mysql_pass_editingFinished()
{
    settings->beginGroup(QLatin1Literal("db"));
    settings->setValue(QLatin1Literal("mysql_pass"),ui->db_mysql_pass->text());
    settings->endGroup();
}

void MainWindow::on_db_mysql_base_editingFinished()
{
    settings->beginGroup(QLatin1Literal("db"));
    settings->setValue(QLatin1Literal("mysql_db"),ui->db_mysql_base->text());
    settings->endGroup();
}

void MainWindow::on_MapVisibilityAlgorithm_currentIndexChanged(int index)
{
    ui->groupBoxMapVisibilityAlgorithmSimple->setEnabled(index==0);
    ui->groupBoxMapVisibilityAlgorithmWithBorder->setEnabled(index==2);
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm"));
    settings->setValue(QLatin1Literal("MapVisibilityAlgorithm"),index);
    settings->endGroup();
}

void MainWindow::on_MapVisibilityAlgorithmSimpleMax_valueChanged(int arg1)
{
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-Simple"));
    settings->setValue(QLatin1Literal("Max"),arg1);
    settings->endGroup();
    ui->MapVisibilityAlgorithmSimpleReshow->setMaximum(arg1);
}


void MainWindow::on_MapVisibilityAlgorithmSimpleReshow_editingFinished()
{
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-Simple"));
    settings->setValue(QLatin1Literal("Reshow"),ui->MapVisibilityAlgorithmSimpleReshow->value());
    settings->endGroup();
}

void MainWindow::on_db_type_currentIndexChanged(int index)
{
    settings->beginGroup(QLatin1Literal("db"));
    switch(index)
    {
        case 0:
        default:
            settings->setValue(QLatin1Literal("type"),QLatin1Literal("mysql"));
        break;
        case 1:
            settings->setValue(QLatin1Literal("type"),QLatin1Literal("sqlite"));
        break;
        case 2:
            settings->setValue(QLatin1Literal("type"),QLatin1Literal("postgresql"));
        break;
    }
    settings->endGroup();
    updateDbGroupbox();
}

void MainWindow::updateDbGroupbox()
{
    int index=ui->db_type->currentIndex();
    ui->groupBoxDbMysql->setEnabled(index==0 || index==2);
    ui->groupBoxDbSQLite->setEnabled(index==1);
}

void MainWindow::on_sendPlayerNumber_toggled(bool checked)
{
    Q_UNUSED(checked);
    settings->setValue(QLatin1Literal("sendPlayerNumber"),ui->sendPlayerNumber->isChecked());
}

void MainWindow::on_db_sqlite_browse_clicked()
{
    QString file=QFileDialog::getOpenFileName(this,tr("Select the SQLite database"));
    if(file.isEmpty())
        return;
    ui->db_sqlite_file->setText(file);
}

void MainWindow::on_tolerantMode_toggled(bool checked)
{
    settings->setValue(QLatin1Literal("tolerantMode"),checked);
}

void MainWindow::on_db_fight_sync_currentIndexChanged(int index)
{
    settings->beginGroup(QLatin1Literal("db"));
    switch(index)
    {
        case 0:
            settings->setValue(QLatin1Literal("db_fight_sync"),QLatin1Literal("FightSync_AtEachTurn"));
        break;
        case 1:
        default:
            settings->setValue(QLatin1Literal("db_fight_sync"),QLatin1Literal("FightSync_AtTheEndOfBattle"));
        break;
    }
    settings->endGroup();
}

void MainWindow::on_comboBox_city_capture_frequency_currentIndexChanged(int index)
{
    settings->beginGroup(QLatin1Literal("city"));
    switch(index)
    {
        default:
        case 0:
            settings->setValue(QLatin1Literal("capture_frequency"),QLatin1Literal("week"));
        break;
        case 1:
            settings->setValue(QLatin1Literal("capture_frequency"),QLatin1Literal("month"));
        break;
    }
    settings->endGroup();
    update_capture();
}

void MainWindow::on_comboBox_city_capture_day_currentIndexChanged(int index)
{
    settings->beginGroup(QLatin1Literal("city"));
    switch(index)
    {
        default:
        case 0:
            settings->setValue(QLatin1Literal("capture_day"),QLatin1Literal("monday"));
        break;
        case 1:
            settings->setValue(QLatin1Literal("capture_day"),QLatin1Literal("tuesday"));
        break;
        case 2:
            settings->setValue(QLatin1Literal("capture_day"),QLatin1Literal("wednesday"));
        break;
        case 3:
            settings->setValue(QLatin1Literal("capture_day"),QLatin1Literal("thursday"));
        break;
        case 4:
            settings->setValue(QLatin1Literal("capture_day"),QLatin1Literal("friday"));
        break;
        case 5:
            settings->setValue(QLatin1Literal("capture_day"),QLatin1Literal("saturday"));
        break;
        case 6:
            settings->setValue(QLatin1Literal("capture_day"),QLatin1Literal("sunday"));
        break;
    }
    settings->endGroup();
}

void MainWindow::on_timeEdit_city_capture_time_editingFinished()
{
    settings->beginGroup(QLatin1Literal("city"));
    QTime time=ui->timeEdit_city_capture_time->time();
    settings->setValue(QLatin1Literal("capture_time"),QStringLiteral("%1:%2").arg(time.hour()).arg(time.minute()));
    settings->endGroup();
}

void MainWindow::update_capture()
{
    switch(ui->comboBox_city_capture_frequency->currentIndex())
    {
        case 0:
            ui->label_city_capture_day->setVisible(true);
            ui->label_city_capture_time->setVisible(true);
            ui->comboBox_city_capture_day->setVisible(true);
            ui->timeEdit_city_capture_time->setVisible(true);
        break;
        case 1:
            ui->label_city_capture_day->setVisible(false);
            ui->label_city_capture_time->setVisible(true);
            ui->comboBox_city_capture_day->setVisible(false);
            ui->timeEdit_city_capture_time->setVisible(true);
        break;
        default:
        case 2:
            ui->label_city_capture_day->setVisible(false);
            ui->label_city_capture_time->setVisible(false);
            ui->comboBox_city_capture_day->setVisible(false);
            ui->timeEdit_city_capture_time->setVisible(false);
        break;
    }
}

void MainWindow::on_compression_currentIndexChanged(int index)
{
    if(index<0)
        return;
    switch(index)
    {
        case 0:
        settings->setValue(QLatin1Literal("compression"),QLatin1Literal("none"));
        break;
        default:
        case 1:
        settings->setValue(QLatin1Literal("compression"),QLatin1Literal("zlib"));
        break;
        case 2:
        settings->setValue(QLatin1Literal("compression"),QLatin1Literal("xz"));
        break;
    }
}

void MainWindow::on_min_character_editingFinished()
{
    settings->setValue(QLatin1Literal("min_character"),ui->min_character->value());
    ui->max_character->setMinimum(ui->min_character->value());
}

void MainWindow::on_max_character_editingFinished()
{
    settings->setValue(QLatin1Literal("max_character"),ui->max_character->value());
    ui->min_character->setMaximum(ui->max_character->value());
}

void MainWindow::on_max_pseudo_size_editingFinished()
{
    settings->setValue(QLatin1Literal("max_pseudo_size"),ui->max_pseudo_size->value());
}

void MainWindow::on_character_delete_time_editingFinished()
{
    settings->setValue(QLatin1Literal("character_delete_time"),ui->character_delete_time->value()*3600);
}

void MainWindow::on_automatic_account_creation_clicked()
{
    settings->setValue(QLatin1Literal("automatic_account_creation"),ui->automatic_account_creation->isChecked());
}

void MainWindow::on_anonymous_toggled(bool checked)
{
    Q_UNUSED(checked);
    settings->setValue(QLatin1Literal("anonymous"),ui->anonymous->isChecked());
}

void MainWindow::on_server_message_textChanged()
{
    settings->setValue(QLatin1Literal("server_message"),ui->server_message->toPlainText());
}

void MainWindow::on_proxy_editingFinished()
{
    settings->setValue(QLatin1Literal("proxy"),ui->proxy->text());
}

void MainWindow::on_proxy_port_editingFinished()
{
    settings->setValue(QLatin1Literal("proxy_port"),ui->proxy_port->value());
}

void MainWindow::on_forceSpeed_toggled(bool checked)
{
    Q_UNUSED(checked);
    ui->speed->setEnabled(ui->forceSpeed->isChecked());
    if(!ui->forceSpeed->isChecked())
        settings->setValue(QLatin1Literal("forcedSpeed"),0);
    else
        settings->setValue(QLatin1Literal("forcedSpeed"),ui->speed->value());
}

void MainWindow::on_speed_editingFinished()
{
    ui->speed->setEnabled(ui->forceSpeed->isChecked());
    if(!ui->forceSpeed->isChecked())
        settings->setValue(QLatin1Literal("forcedSpeed"),0);
    else
        settings->setValue(QLatin1Literal("forcedSpeed"),ui->speed->value());
}

void MainWindow::on_dontSendPseudo_toggled(bool checked)
{
    Q_UNUSED(checked);
    settings->setValue(QLatin1Literal("dontSendPseudo"),ui->dontSendPseudo->isChecked());
}

void MainWindow::on_dontSendPlayerType_toggled(bool checked)
{
    Q_UNUSED(checked);
    settings->setValue(QLatin1Literal("dontSendPlayerType"),ui->dontSendPlayerType->isChecked());
}

void MainWindow::on_rates_xp_pow_normal_valueChanged(double arg1)
{
    settings->beginGroup(QLatin1Literal("rates"));
    settings->setValue(QLatin1Literal("xp_pow_normal"),arg1);
    settings->endGroup();
}

void MainWindow::on_rates_drop_normal_valueChanged(double arg1)
{
    settings->beginGroup(QLatin1Literal("rates"));
    settings->setValue(QLatin1Literal("drop_normal"),arg1);
    settings->endGroup();
}

void MainWindow::on_forceClientToSendAtMapChange_toggled(bool checked)
{
    Q_UNUSED(checked);
    settings->setValue(QLatin1Literal("forceClientToSendAtMapChange"),ui->forceClientToSendAtMapChange->isChecked());
}

void MainWindow::on_MapVisibilityAlgorithmWithBorderMax_editingFinished()
{
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-WithBorder"));
    settings->setValue(QLatin1Literal("Max"),ui->MapVisibilityAlgorithmWithBorderMax->value());
    settings->endGroup();
    ui->MapVisibilityAlgorithmWithBorderReshow->setMaximum(ui->MapVisibilityAlgorithmWithBorderMax->value());
    ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderMax->value());
    if(ui->MapVisibilityAlgorithmWithBorderReshow->value()>ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value())
        ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value());
    else
        ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderReshow->value());
}

void MainWindow::on_MapVisibilityAlgorithmWithBorderReshow_editingFinished()
{
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-WithBorder"));
    settings->setValue(QLatin1Literal("Reshow"),ui->MapVisibilityAlgorithmSimpleReshow->value());
    settings->endGroup();
    if(ui->MapVisibilityAlgorithmWithBorderReshow->value()>ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value())
        ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value());
    else
        ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderReshow->value());
}

void MainWindow::on_MapVisibilityAlgorithmWithBorderMaxWithBorder_editingFinished()
{
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-WithBorder"));
    settings->setValue(QLatin1Literal("MaxWithBorder"),ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value());
    settings->endGroup();
    if(ui->MapVisibilityAlgorithmWithBorderReshow->value()>ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value())
        ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderMaxWithBorder->value());
    else
        ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->setMaximum(ui->MapVisibilityAlgorithmWithBorderReshow->value());
}

void MainWindow::on_MapVisibilityAlgorithmWithBorderReshowWithBorder_editingFinished()
{
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-WithBorder"));
    settings->setValue(QLatin1Literal("ReshowWithBorder"),ui->MapVisibilityAlgorithmWithBorderReshowWithBorder->value());
    settings->endGroup();
}

void MainWindow::on_httpDatapackMirror_editingFinished()
{
    settings->setValue(QLatin1Literal("httpDatapackMirror"),ui->httpDatapackMirror->text());
}

void MainWindow::on_datapack_cache_toggled(bool checked)
{
    Q_UNUSED(checked);
    ui->datapack_cache_timeout_checkbox->setEnabled(ui->datapack_cache->isChecked());
    ui->datapack_cache_timeout->setEnabled(ui->datapack_cache->isChecked() && ui->datapack_cache_timeout_checkbox->isChecked());
    datapack_cache_save();
}

void MainWindow::on_datapack_cache_timeout_checkbox_toggled(bool checked)
{
    Q_UNUSED(checked);
    ui->datapack_cache_timeout_checkbox->setEnabled(ui->datapack_cache->isChecked());
    ui->datapack_cache_timeout->setEnabled(ui->datapack_cache->isChecked() && ui->datapack_cache_timeout_checkbox->isChecked());
    datapack_cache_save();
}

void MainWindow::on_datapack_cache_timeout_editingFinished()
{
    datapack_cache_save();
}

void MainWindow::datapack_cache_save()
{
    if(!ui->datapack_cache->isChecked())
        settings->setValue(QLatin1Literal("datapackCache"),-1);
    else if(!ui->datapack_cache_timeout_checkbox->isChecked())
        settings->setValue(QLatin1Literal("datapackCache"),0);
    else
        settings->setValue(QLatin1Literal("datapackCache"),ui->datapack_cache_timeout->value());
}

void MainWindow::on_linux_socket_cork_toggled(bool checked)
{
    #ifdef Q_OS_LINUX
    settings->beginGroup(QLatin1Literal("Linux"));
    settings->setValue(QLatin1Literal("tcpCork"),checked);
    settings->endGroup();
    #endif
}

void CatchChallenger::MainWindow::on_MapVisibilityAlgorithmSimpleReemit_toggled(bool checked)
{
    settings->beginGroup(QLatin1Literal("MapVisibilityAlgorithm-Simple"));
    settings->setValue(QLatin1Literal("Reemit"),checked);
    settings->endGroup();
}

void CatchChallenger::MainWindow::on_useSsl_toggled(bool checked)
{
    settings->setValue(QLatin1Literal("useSsl"),checked);
}

void CatchChallenger::MainWindow::on_useSP_toggled(bool checked)
{
    settings->setValue(QLatin1Literal("useSP"),checked);
}

void CatchChallenger::MainWindow::on_autoLearn_toggled(bool checked)
{
    settings->setValue(QLatin1Literal("autoLearn"),checked);
}

void CatchChallenger::MainWindow::on_programmedEventType_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->programmedEventList->clear();
    const QString &selectedEvent=ui->programmedEventType->currentText();
    if(selectedEvent.isEmpty())
        return;
    if(programmedEventList.contains(selectedEvent))
    {
        const QHash<QString,ServerSettings::ProgrammedEvent> &list=programmedEventList.value(selectedEvent);
        QHashIterator<QString,ServerSettings::ProgrammedEvent> i(list);
        while (i.hasNext()) {
            i.next();
            QListWidgetItem *listWidgetItem=new QListWidgetItem(
                        tr("%1\nCycle: %2mins, offset: %3mins\nValue: %4")
                        .arg(i.key())
                        .arg(i.value().cycle)
                        .arg(i.value().offset)
                        .arg(i.value().value)
                        );
            listWidgetItem->setData(99,i.key());
            ui->programmedEventList->addItem(listWidgetItem);
        }
    }
}

void CatchChallenger::MainWindow::on_programmedEventList_itemActivated(QListWidgetItem *item)
{
    Q_UNUSED(item);
    on_programmedEventEdit_clicked();
}

void CatchChallenger::MainWindow::on_programmedEventAdd_clicked()
{
    const QString &selectedEvent=ui->programmedEventType->currentText();
    if(selectedEvent.isEmpty())
        return;
    ServerSettings::ProgrammedEvent programmedEvent;
    bool ok;
    const QString &name=QInputDialog::getText(this,tr("Name"),tr("Name:"),QLineEdit::Normal,QString(),&ok);
    if(!ok)
        return;
    if(programmedEventList.value(selectedEvent).contains(name))
    {
        QMessageBox::warning(this,tr("Error"),tr("Entry already name"));
        return;
    }
    programmedEvent.cycle=QInputDialog::getInt(this,tr("Name"),tr("Name:"),60,1,60*24,1,&ok);
    if(!ok)
        return;
    programmedEvent.offset=QInputDialog::getInt(this,tr("Name"),tr("Name:"),0,1,60*24,1,&ok);
    if(!ok)
        return;
    programmedEvent.value=QInputDialog::getText(this,tr("Value"),tr("Value:"),QLineEdit::Normal,QString(),&ok);
    if(!ok)
        return;
    programmedEventList[selectedEvent][name]=programmedEvent;
    settings->beginGroup(QLatin1Literal("programmedEvent"));
        settings->beginGroup(selectedEvent);
            settings->beginGroup(name);
                settings->setValue(QLatin1Literal("value"),programmedEvent.value);
                settings->setValue(QLatin1Literal("cycle"),programmedEvent.cycle);
                settings->setValue(QLatin1Literal("offset"),programmedEvent.offset);
            settings->endGroup();
        settings->endGroup();
    settings->endGroup();
    on_programmedEventType_currentIndexChanged(0);
}

void CatchChallenger::MainWindow::on_programmedEventEdit_clicked()
{
    const QList<QListWidgetItem*> &selectedItems=ui->programmedEventList->selectedItems();
    if(selectedItems.size()!=1)
        return;
    const QString &selectedEvent=ui->programmedEventType->currentText();
    if(selectedEvent.isEmpty())
        return;
    ServerSettings::ProgrammedEvent programmedEvent;
    bool ok;
    const QString &oldName=selectedItems.first()->data(99).toString();
    const QString &name=QInputDialog::getText(this,tr("Name"),tr("Name:"),QLineEdit::Normal,oldName,&ok);
    if(!ok)
        return;
    if(programmedEventList.value(selectedEvent).contains(name))
    {
        QMessageBox::warning(this,tr("Error"),tr("Entry already name"));
        return;
    }
    programmedEvent.cycle=QInputDialog::getInt(this,tr("Name"),tr("Name:"),60,1,60*24,1,&ok);
    if(!ok)
        return;
    programmedEvent.offset=QInputDialog::getInt(this,tr("Name"),tr("Name:"),0,1,60*24,1,&ok);
    if(!ok)
        return;
    programmedEvent.value=QInputDialog::getText(this,tr("Value"),tr("Value:"),QLineEdit::Normal,QString(),&ok);
    if(!ok)
        return;
    if(oldName!=name)
        programmedEventList[selectedEvent].remove(oldName);
    programmedEventList[selectedEvent][name]=programmedEvent;
    settings->beginGroup(QLatin1Literal("programmedEvent"));
        settings->beginGroup(selectedEvent);
            settings->beginGroup(oldName);
                settings->remove("");
            settings->endGroup();
            settings->beginGroup(name);
                settings->setValue(QLatin1Literal("value"),programmedEvent.value);
                settings->setValue(QLatin1Literal("cycle"),programmedEvent.cycle);
                settings->setValue(QLatin1Literal("offset"),programmedEvent.offset);
            settings->endGroup();
        settings->endGroup();
    settings->endGroup();
    on_programmedEventType_currentIndexChanged(0);
}

void CatchChallenger::MainWindow::on_programmedEventRemove_clicked()
{
    const QList<QListWidgetItem*> &selectedItems=ui->programmedEventList->selectedItems();
    if(selectedItems.size()!=1)
        return;
    const QString &selectedEvent=ui->programmedEventType->currentText();
    if(selectedEvent.isEmpty())
        return;
    const QString &name=selectedItems.first()->data(99).toString();
    programmedEventList[selectedEvent].remove(name);
    settings->beginGroup(QLatin1Literal("programmedEvent"));
        settings->beginGroup(selectedEvent);
            settings->beginGroup(name);
                settings->remove("");
            settings->endGroup();
        settings->endGroup();
    settings->endGroup();
    on_programmedEventType_currentIndexChanged(0);
}

void CatchChallenger::MainWindow::on_tcpNodelay_toggled(bool checked)
{
    #ifdef Q_OS_LINUX
    settings->beginGroup(QLatin1Literal("Linux"));
    settings->setValue(QLatin1Literal("tcpNodelay"),checked);
    settings->endGroup();
    #endif
}

void CatchChallenger::MainWindow::on_maxPlayerMonsters_editingFinished()
{
    settings->setValue(QLatin1Literal("maxPlayerMonsters"),ui->maxPlayerMonsters->value());
}

void CatchChallenger::MainWindow::on_maxWarehousePlayerMonsters_editingFinished()
{
    settings->setValue(QLatin1Literal("maxWarehousePlayerMonsters"),ui->maxWarehousePlayerMonsters->value());
}

void CatchChallenger::MainWindow::on_maxPlayerItems_editingFinished()
{
    settings->setValue(QLatin1Literal("maxPlayerItems"),ui->maxPlayerItems->value());
}

void CatchChallenger::MainWindow::on_maxWarehousePlayerItems_editingFinished()
{
    settings->setValue(QLatin1Literal("maxWarehousePlayerItems"),ui->maxWarehousePlayerItems->value());
}

void CatchChallenger::MainWindow::on_tryInterval_editingFinished()
{
    settings->beginGroup(QLatin1Literal("db"));
    settings->setValue(QLatin1Literal("tryInterval"),ui->tryInterval->value());
    settings->endGroup();
}

void CatchChallenger::MainWindow::on_considerDownAfterNumberOfTry_editingFinished()
{
    settings->beginGroup(QLatin1Literal("db"));
    settings->setValue(QLatin1Literal("considerDownAfterNumberOfTry"),ui->considerDownAfterNumberOfTry->value());
    settings->endGroup();
}
