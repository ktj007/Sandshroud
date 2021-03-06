/***
 * Demonstrike Core
 */

#pragma once

#ifndef NETLIB_IOCP

/** Standard ListenSocket
 */

template<class T>
class SERVER_DECL ListenSocket : public BaseSocket
{
    SOCKET new_fd;
    sockaddr_in new_peer;
    sockaddr_in address;
public:

    ListenSocket()
    {
        m_fd = socket(AF_INET, SOCK_STREAM, 0);
        u_long arg = 1;
        setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&arg, sizeof(u_long));
        if(m_fd == INVALID_SOCKET)
        {
            printf("WARNING: ListenSocket constructor: could not create socket() %u (%s)\n", errno, strerror(errno));
        }
        m_connected = false;
        m_deleted = false;
    }

    void OnRead(size_t len)
    {
        if(!m_connected)
            return;

#if PLATFORM == PLATFORM_WIN
        int len2 = sizeof(sockaddr_in);
#else
        socklen_t len2 = sizeof(sockaddr_in);
#endif
        new_fd = accept(m_fd, (sockaddr*)&new_peer, &len2);
        if(new_fd > 0)
        {
            T * s = new T(new_fd, &new_peer);
            s->Finalize();
        }
    }

    void OnWrite(size_t len) {}
    void OnError(int err) {}
    void OnAccept(void * pointer) {}
    
    bool Open(const char * hostname, u_short port)
    {
        if(m_fd == INVALID_SOCKET)
        {
            printf("No fd in listensocket\n");
            return false;
        }

        if(!strcmp(hostname, "0.0.0.0"))
            address.sin_addr.s_addr = htonl(INADDR_ANY);
        else
        {
            hostent * h = gethostbyname(hostname);
            if(!h)
            {
                printf("Could not resolve listen address\n");
                return false;
            }

            memcpy(&address.sin_addr, h->h_addr_list[0], sizeof(in_addr));
        }

        address.sin_family = AF_INET;
        address.sin_port = ntohs(port);

        if(bind(m_fd, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
        {
            printf("Could not bind\n");
            return false;
        }

        if(listen(m_fd, 5) < 0)
        {
            printf("Could not bind\n");
            return false;
        }

        // add to mgr
        m_connected = true;
        sSocketEngine.AddSocket(this);
        return true;
    }

    bool Writable() { return false; }

    void Delete()
    {
        if(m_deleted) return;
        m_deleted = true;
        
        /* queue deletion */
        sSocketDeleter.Add(this);

        if(m_connected) Disconnect();
    }

    void Disconnect()
    {
        if(!m_connected) return;
        m_connected = false;

        sSocketEngine.RemoveSocket(this);
        closesocket(m_fd);
        if(!m_deleted) Delete();
    }
};

#else

/** IOCP ListenSocket
 */

#include <mswsock.h>

static int Length = sizeof(sockaddr_in)+16;

template<class T>
class SERVER_DECL ListenSocket : public BaseSocket
{
    SOCKET new_fd;
    sockaddr_in new_peer;
    sockaddr_in address;
public:

    ListenSocket()
    {
        //m_fd = socket(AF_INET, SOCK_STREAM, 0);
        m_fd = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
        m_connected = false;
        m_deleted = false;
    }

    void OnRead(size_t len) { }
    void OnWrite(size_t len) {}
    void OnError(int err) {}

    void OnAccept(void * pointer)
    {
        // Our socket was pushed to front of passback buffer
        SOCKET fd = *(SOCKET*)pointer;
        sockaddr *local, *remote;
        int localOut = 0, remoteOut = 0;
        GetAcceptExSockaddrs(((char*)pointer)+sizeof(SOCKET), 0, Length, Length, &local, &localOut, &remote, &remoteOut);

        sockaddr_in *addr = (sockaddr_in*)remote;
        T * s = new T(fd, addr);
        s->Finalize();
        free(pointer);

        /* post the next accept event */
        PostEvent();
    }

    bool Writable() { return false; }

    void Delete()
    {
        if(m_deleted) return;
        m_deleted = true;

        /* queue deletion */
        sSocketDeleter.Add(this);

        if(m_connected) Disconnect();
    }

    void Disconnect()
    {
        if(!m_connected)
            return;
        m_connected = false;

        sSocketEngine.RemoveSocket(this);
        closesocket(m_fd);
        if(!m_deleted) Delete();
    }

    void PostEvent()
    {
        Overlapped * ov = new Overlapped;
        memset(ov, 0, sizeof(Overlapped));
        ov->m_op = IO_EVENT_ACCEPT;
        ov->m_acceptBuffer = malloc(1024);
        memset(ov->m_acceptBuffer, 0, 1024);

        SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
        *(SOCKET*)&((char*)ov->m_acceptBuffer)[0] = s;
        DWORD bytes;

        if(!AcceptEx(m_fd, s, ((char*)ov->m_acceptBuffer) + sizeof(SOCKET), 0, Length, Length, &bytes, &ov->m_ov))
        {
            if(WSA_IO_PENDING != WSAGetLastError())
                printf("AcceptEx error: %u\n", WSAGetLastError());
        }
    }

    bool Open(const char * hostname, u_short port)
    {
        if(m_fd == INVALID_SOCKET)
            return false;

        if(!strcmp(hostname, "0.0.0.0"))
            address.sin_addr.s_addr = htonl(INADDR_ANY);
        else
        {
            hostent * h = gethostbyname(hostname);
            if(!h)
                return false;

            memcpy(&address.sin_addr, h->h_addr_list[0], sizeof(in_addr));
        }

        address.sin_family = AF_INET;
        address.sin_port = ntohs(port);

        if(bind(m_fd, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
            return false;

        if(listen(m_fd, 5) < 0)
            return false;

        m_connected = true;
        sSocketEngine.AddSocket(this);
        PostEvent();
        return true;
    }
};

#endif      // NETLIB_IOCP

/* Common Functions */
template<class T>
bool CreateListenSocket(const char * hostname, u_short port)
{
    ListenSocket<T> * s = new ListenSocket<T>();
    if(!s->Open(hostname, port))
    {
        s->Delete();
        return false;
    }
    else
        return true;
}
