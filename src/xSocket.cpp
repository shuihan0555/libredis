#include "all.h"
#include "xSocket.h"


xSocket::xSocket(xEventLoop *loop,std::string ip,int32_t port)
:loop(loop),
 ip(ip),
 port(port)
{
	   listenSocketFd          = -1;
	   onlineNumber            = 0;
	   createTcpListenSocket();

}

xSocket::xSocket()
{
    listenSocketFd          = -1;
    onlineNumber            = 0;
}

xSocket::~xSocket()
{
	 ::close(listenSocketFd);
}


int  xSocket::getListenFd()
{
	return listenSocketFd;

}


int  xSocket::createNonBloackSocket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

int xSocket::connect(int sockfd,std::string ip,int port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(ip.c_str());

	int ret = ::connect(sockfd, (struct sockaddr *)&sin, sizeof(sin));
	return ret;
}

bool xSocket::createTcpListenSocket()
{
    struct sockaddr_in serverAdress;
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_port   = htons(port);

    if (ip.length() > 0 )
    {
        serverAdress.sin_addr.s_addr = inet_addr(ip.c_str());
    }
    else
    {
        serverAdress.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    listenSocketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenSocketFd < 0)
    {
        //TRACE("Create Tcp Socket Failed! <%s>", strerror(errno));
        return false;
    }

    if (!setSocketNonBlock(listenSocketFd))
    {
        //TRACE("Set listen socket <%d> to non-block failed!", listenSocketFd);
        return false;
    }

    int optval = 1;
    if (setsockopt(listenSocketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        //TRACE("Set listen socket<%d> failed! error:%s", listenSocketFd, strerror(errno));
        close(listenSocketFd);
        return false;
    }

    if (bind(listenSocketFd, (struct sockaddr*)&serverAdress, sizeof(serverAdress)) < 0 )
    {
        //TRACE("Bind listen socket<%d> failed! error:%s", listenSocketFd, strerror(errno));
        close(listenSocketFd);
        return false;
    }

    if (listen(listenSocketFd, SOMAXCONN))
    {
        //TRACE("Listen listen socket<%d> failed! error:%s", listenSocketFd, strerror(errno));
        close(listenSocketFd);
        return false;
    }
    setsockopt(listenSocketFd, IPPROTO_TCP, TCP_NODELAY,&optval, static_cast<socklen_t>(sizeof optval));

    int len = 65536;
    setsockopt(listenSocketFd, SOL_SOCKET, SO_RCVBUF, (void*)&len, sizeof(len));
    setsockopt(listenSocketFd, SOL_SOCKET, SO_SNDBUF, (void*)&len, sizeof(len));

    return true;
}

bool xSocket::setSocketNonBlock(int socketFd)
{
    int opt = fcntl(socketFd, F_GETFL);
    if (opt < 0)
    {
        //TRACE("fcntl(%d, F_GETFL) failed! error:%s", socketFd, strerror(errno));
        return false;
    }

    opt = opt | O_NONBLOCK | O_NDELAY;
    if (fcntl(socketFd, F_SETFL, opt) < 0)
    {
        //TRACE("fcntl(%d, F_GETFL, %d) failed! error:%s", socketFd, opt, strerror(errno));
        return false;
    }

    return true;
}



