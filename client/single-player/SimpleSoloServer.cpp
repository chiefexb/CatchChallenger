#include "SimpleSoloServer.h"
#include "ui_SimpleSoloServer.h"

SimpleSoloServer::SimpleSoloServer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SimpleSoloServer)
{
    ui->setupUi(this);
    solowindow=new SoloWindow(this,QCoreApplication::applicationDirPath()+"/datapack/",QCoreApplication::applicationDirPath()+"/savegames/",true);
    connect(solowindow,&SoloWindow::play,this,&SimpleSoloServer::play);

    socket=new CatchChallenger::ConnectedSocket(new CatchChallenger::QFakeSocket());
    CatchChallenger::Api_client_real::client=new CatchChallenger::Api_client_virtual(socket,QCoreApplication::applicationDirPath()+"/datapack/");
    internalServer=new CatchChallenger::InternalServer();
    connect(internalServer,&CatchChallenger::InternalServer::is_started,this,&SimpleSoloServer::is_started,Qt::QueuedConnection);
    connect(internalServer,&CatchChallenger::InternalServer::error,this,&SimpleSoloServer::serverError,Qt::QueuedConnection);
    connect(CatchChallenger::Api_client_real::client,               &CatchChallenger::Api_protocol::protocol_is_good,   this,&SimpleSoloServer::protocol_is_good);
    connect(CatchChallenger::Api_client_real::client,               &CatchChallenger::Api_protocol::disconnected,       this,&SimpleSoloServer::disconnected);
    connect(CatchChallenger::Api_client_real::client,               &CatchChallenger::Api_protocol::message,            this,&SimpleSoloServer::message);
    connect(socket,                                                 &CatchChallenger::ConnectedSocket::stateChanged,    this,&SimpleSoloServer::stateChanged);
    if(CatchChallenger::BaseWindow::baseWindow!=NULL)
        delete CatchChallenger::BaseWindow::baseWindow;
    CatchChallenger::BaseWindow::baseWindow=new CatchChallenger::BaseWindow();
    CatchChallenger::BaseWindow::baseWindow->setMultiPlayer(false);
    ui->stackedWidget->addWidget(CatchChallenger::BaseWindow::baseWindow);
    ui->stackedWidget->addWidget(solowindow);
    ui->stackedWidget->setCurrentWidget(solowindow);
    //solowindow->show();
    setWindowTitle("CatchChallenger");
}

SimpleSoloServer::~SimpleSoloServer()
{
    delete ui;
    if(socket!=NULL)
    {
        socket->abort();
        socket->deleteLater();
    }
    CatchChallenger::BaseWindow::baseWindow->deleteLater();
    CatchChallenger::BaseWindow::baseWindow=NULL;
}

void SimpleSoloServer::play(const QString &savegamesPath)
{
    sendSettings(internalServer,savegamesPath);
    internalServer->start();
    ui->stackedWidget->setCurrentWidget(CatchChallenger::BaseWindow::baseWindow);
    timeLaunched=QDateTime::currentDateTimeUtc().toTime_t();
    QSettings metaData(savegamesPath+"metadata.conf",QSettings::IniFormat);
    if(!metaData.contains("pass"))
    {
        QMessageBox::critical(NULL,tr("Error"),tr("Unable to load internal value"));
        return;
    }
    launchedGamePath=savegamesPath;
    haveLaunchedGame=true;
    pass=metaData.value("pass").toString();

    CatchChallenger::BaseWindow::baseWindow->serverIsLoading();
}

void SimpleSoloServer::sendSettings(CatchChallenger::InternalServer * internalServer,const QString &savegamesPath)
{
    CatchChallenger::ServerSettings formatedServerSettings=internalServer->getSettings();

    formatedServerSettings.max_players=1;
    formatedServerSettings.tolerantMode=false;
    formatedServerSettings.commmonServerSettings.sendPlayerNumber = false;
    formatedServerSettings.commmonServerSettings.compressionType=CatchChallenger::CompressionType_None;

    formatedServerSettings.database.type=CatchChallenger::ServerSettings::Database::DatabaseType_SQLite;
    formatedServerSettings.database.sqlite.file=savegamesPath+"catchchallenger.db.sqlite";
    formatedServerSettings.mapVisibility.mapVisibilityAlgorithm	= CatchChallenger::MapVisibilityAlgorithm_none;
    formatedServerSettings.bitcoin.enabled=false;
    formatedServerSettings.datapack_basePath=CatchChallenger::Api_client_real::client->get_datapack_base_name();

    internalServer->setSettings(formatedServerSettings);
}

void SimpleSoloServer::protocol_is_good()
{
    CatchChallenger::Api_client_real::client->tryLogin("admin",pass);
}

void SimpleSoloServer::disconnected(QString reason)
{
    QMessageBox::information(this,tr("Disconnected"),tr("Disconnected by the reason: %1").arg(reason));
    haveShowDisconnectionReason=true;
    resetAll();
}

void SimpleSoloServer::message(QString message)
{
    qDebug() << message;
}

void SimpleSoloServer::stateChanged(QAbstractSocket::SocketState socketState)
{
    if(socketState==QAbstractSocket::UnconnectedState)
        resetAll();
    if(CatchChallenger::BaseWindow::baseWindow!=NULL)
        CatchChallenger::BaseWindow::baseWindow->stateChanged(socketState);
}

void SimpleSoloServer::serverError(const QString &error)
{
    QMessageBox::critical(NULL,tr("Error"),tr("The engine is closed due to: %1").arg(error));
    resetAll();
}

void SimpleSoloServer::is_started(bool started)
{
    if(!started)
    {
        if(internalServer!=NULL)
        {
            delete internalServer;
            internalServer=NULL;
        }
        saveTime();
        if(!isVisible())
            QCoreApplication::quit();
        else
            resetAll();
    }
    else
    {
        CatchChallenger::BaseWindow::baseWindow->serverIsReady();
        socket->connectToHost("localhost",9999);
    }
}

void SimpleSoloServer::saveTime()
{
    if(internalServer==NULL)
        return;
    //save the time
    if(haveLaunchedGame)
    {
        bool settingOk=false;
        QSettings metaData(launchedGamePath+"metadata.conf",QSettings::IniFormat);
        if(metaData.isWritable())
        {
            if(metaData.status()==QSettings::NoError)
            {
                QString locaction=CatchChallenger::BaseWindow::baseWindow->lastLocation();
                QString mapPath=internalServer->getSettings().datapack_basePath+DATAPACK_BASE_PATH_MAP;
                if(locaction.startsWith(mapPath))
                    locaction.remove(0,mapPath.size());
                if(!locaction.isEmpty())
                    metaData.setValue("location",locaction);
                quint64 current_date_time=QDateTime::currentDateTimeUtc().toTime_t();
                if(current_date_time>timeLaunched)
                    metaData.setValue("time_played",metaData.value("time_played").toUInt()+(current_date_time-timeLaunched));
                settingOk=true;
            }
            else
                qDebug() << "Settings error: " << metaData.status();
        }
        solowindow->updateSavegameList();
        if(!settingOk)
        {
            QMessageBox::critical(NULL,tr("Error"),tr("Unable to save internal value at game stopping"));
            return;
        }
        haveLaunchedGame=false;
    }
}

void SimpleSoloServer::resetAll()
{
    if(CatchChallenger::Api_client_real::client!=NULL)
        CatchChallenger::Api_client_real::client->resetAll();
    ui->stackedWidget->setCurrentWidget(solowindow);
}