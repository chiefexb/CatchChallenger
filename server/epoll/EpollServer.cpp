#ifndef SERVERSSL

#include "EpollServer.h"
#include "EpollSocket.h"
#include "Epoll.h"

#include <iostream>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "../base/ServerStructures.h"
#include "../base/GlobalServerData.h"

using namespace CatchChallenger;

EpollServer::EpollServer()
{
    ready=false;
    sfd=-1;

    normalServerSettings.server_ip      = QString();
    normalServerSettings.server_port    = 42489;
    normalServerSettings.useSsl         = true;
    #ifdef Q_OS_LINUX
    CommonSettings::commonSettings.tcpCork                      = true;
    #endif
}

EpollServer::~EpollServer()
{
    close();
}

void EpollServer::preload_finish()
{
    BaseServer::preload_finish();
    qDebug() << QStringLiteral("Waiting connection on port %1").arg(normalServerSettings.server_port);
    ready=true;
}

bool EpollServer::isReady()
{
    return ready;
}

void EpollServer::quitForCriticalDatabaseQueryFailed()
{
    abort();
}

bool EpollServer::tryListen()
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    if(!normalServerSettings.server_ip.isEmpty())
        s = getaddrinfo(normalServerSettings.server_ip.toUtf8().constData(), QString::number(normalServerSettings.server_port).toUtf8().constData(), &hints, &result);
    else
        s = getaddrinfo(NULL, QString::number(normalServerSettings.server_port).toUtf8().constData(), &hints, &result);
    if (s != 0)
    {
        std::cerr << "getaddrinfo:" << gai_strerror(s) << std::endl;
        return false;
    }

    for(rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sfd == -1)
            continue;

        s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0)
        {
            /* We managed to bind successfully! */
            break;
        }

        ::close(sfd);
    }

    if(rp == NULL)
    {
        std::cerr << qPrintable(normalServerSettings.server_ip) << " resolved into:" << std::endl;
        for(rp = result; rp != NULL; rp = rp->ai_next)
        {
            std::cerr
                    << "familly: " << rp->ai_family
                    << ", rp->ai_socktype: " << rp->ai_socktype
                    << ", rp->ai_protocol: " << rp->ai_protocol
                    << ", rp->ai_addr: " << rp->ai_addr
                    << ", rp->ai_addrlen: " << rp->ai_addrlen
                    << std::endl;
            sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if(sfd == -1)
                std::cerr << "socket not created:" << sfd << ", error (errno): " << errno << ", error string: " << strerror(errno) << std::endl;
            else
            {
                s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
                std::cerr << "bind failed:" << s << ", error (errno): " << errno << ", error string: " << strerror(errno) << std::endl;
                ::close(sfd);
            }
        }
        return false;
    }

    freeaddrinfo (result);

    s = EpollSocket::make_non_blocking(sfd);
    if(s == -1)
    {
        close();
        std::cerr << "Can't put in non blocking" << std::endl;
        return false;
    }
    yes=1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)))
    {
        close();
        std::cerr << "Can't put in reuse" << std::endl;
        return false;
    }

    s = listen(sfd, SOMAXCONN);
    if(s == -1)
    {
        close();
        std::cerr << "Unable to listen" << std::endl;
        return false;
    }
    int one=1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one)!=0)
        std::cerr << "Unable to apply SO_REUSEADDR" << std::endl;
    one=1;
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof one)!=0)
        std::cerr << "Unable to apply SO_REUSEPORT" << std::endl;

    epoll_event event;
    event.data.ptr = this;
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    s = Epoll::epoll.ctl(EPOLL_CTL_ADD, sfd, &event);
    if(s == -1)
    {
        close();
        std::cerr << "epoll_ctl error" << std::endl;
        return false;
    }
    return true;
}

void EpollServer::close()
{
    if(sfd!=-1)
        ::close(sfd);
}

int EpollServer::accept(sockaddr *in_addr,socklen_t *in_len)
{
    return ::accept(sfd, in_addr, in_len);
}

int EpollServer::getSfd()
{
    return sfd;
}

BaseClassSwitch::Type EpollServer::getType() const
{
    return BaseClassSwitch::Type::Server;
}

void EpollServer::preload_the_data()
{
    BaseServer::preload_the_data();
}

void EpollServer::unload_the_data()
{
    BaseServer::unload_the_data();
}

void EpollServer::setNormalSettings(const NormalServerSettings &settings)
{
    normalServerSettings=settings;
    loadAndFixSettings();
}

NormalServerSettings EpollServer::getNormalSettings() const
{
    return normalServerSettings;
}

void EpollServer::loadAndFixSettings()
{
    if(normalServerSettings.server_port<=0)
        normalServerSettings.server_port=42489;
    if(normalServerSettings.proxy_port<=0)
        normalServerSettings.proxy=QString();
}
#endif
