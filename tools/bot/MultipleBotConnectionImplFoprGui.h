#ifndef MULTIPLEBOTCONNECTIONIMPLFOPRGUI_H
#define MULTIPLEBOTCONNECTIONIMPLFOPRGUI_H

#include "MultipleBotConnection.h"

class MultipleBotConnectionImplFoprGui : public MultipleBotConnection
{
    Q_OBJECT
public:
    explicit MultipleBotConnectionImplFoprGui();
    ~MultipleBotConnectionImplFoprGui();
    void characterSelect(const quint32 &charId);
public slots:
    void detectSlowDown();
public:
    QString mLogin;
    QString mPass;
    bool mMultipleConnexion;
    bool mAutoCreateCharacter;
    int mConnectBySeconds;
    int mConnexionCount;
    int mMaxDiffConnectedSelected;
    QString mProxy;
    quint16 mProxyport;
    QString mHost;
    quint16 mPort;
private:
    QString login();
    QString pass();
    bool multipleConnexion();
    bool autoCreateCharacter();
    int connectBySeconds();
    int connexionCount();
    int maxDiffConnectedSelected();
    QString proxy();
    quint16 proxyport();
    QString host();
    quint16 port();
private:
    virtual void insert_player(const CatchChallenger::Player_public_informations &player,const quint32 &mapId,const quint16 &x,const quint16 &y,const CatchChallenger::Direction &direction);
    virtual void logged(const QList<CatchChallenger::CharacterEntry> &characterEntryList);
    virtual void sslHandcheckIsFinished();
    virtual void readForFirstHeader();
    virtual void newCharacterId(const quint8 &returnCode, const quint32 &characterId);
    virtual void haveTheDatapack();
    virtual void sslErrors(const QList<QSslError> &errors);
    virtual void protocol_is_good();
    virtual void newSocketError(QAbstractSocket::SocketError error);
    virtual void newError(QString error,QString detailedError);
    virtual void have_current_player_info(const CatchChallenger::Player_private_and_public_informations &informations);
    virtual void disconnected();
    virtual void connectTimerSlot();

    virtual void connectTheExternalSocket(CatchChallengerClient * client);
signals:
    void loggedDone(const QList<CatchChallenger::CharacterEntry> &characterEntryList,bool haveTheDatapack);
    void statusError(QString error);
    void chat_text(const CatchChallenger::Chat_type &chat_type,const QString &text,const QString &pseudo,const CatchChallenger::Player_type &type) const;
    void emit_detectSlowDown(QString text);
};

#endif // MULTIPLEBOTCONNECTIONIMPLFOPRGUI_H
