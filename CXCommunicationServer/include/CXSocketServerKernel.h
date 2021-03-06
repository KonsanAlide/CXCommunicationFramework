/****************************************************************************
Copyright (c) 2018 Charles Yang

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description£º
*****************************************************************************/
#ifndef CXSOCKETSERVERKERNEL_H
#define CXSOCKETSERVERKERNEL_H

#include "CXSocketServerImpl.h"
#include "SocketDefine.h"
#include "CXEvent.h"
#include <list>
#include "CXLog.h"
using std::list;

namespace CXCommunication
{
    class CXSocketServerKernel : public CXSocketServerImpl
    {
    private:
            unsigned short m_nListeningPort;
            int    m_iWaitThreadNum;
            uint64 m_uiConnectionNumber;

            cxsocket m_sockListen;
            int    m_epollHandle;
            #ifdef WIN32
            HANDLE m_iocpHandle;
            #endif
            CXThread m_threadListen;
            CXThread m_threadMonitor;

            CXLog   *m_pLogHandle;

			CXThreadPool m_threadPool;

    public:
            CXSocketServerKernel();
            virtual ~CXSocketServerKernel();

            //virtual bool SetNonblocking(int sock)=0;
            //create a socket ,bind it to a port and ip ,listen in it
            int CreateTcpListenPort(cxsocket & sock, unsigned short usPort, char *pszLocalIP=NULL);
            virtual int Start(unsigned short iListeningPort,int iWaitThreadNum);
            virtual int Stop();
			virtual CXThread* FindThread(DWORD dwThreadID) { return m_threadPool.FindThread(dwThreadID); }

            virtual int OnAccept(void *pServer, cxsocket sock, sockaddr_in &addrRemote);
            virtual int OnRead(CXConnectionObject& conObj, PCXBufferObj pBufObj,
                DWORD dwTransDataOfBytes);
            virtual int OnWrite(CXConnectionObject& conObj);
            virtual void OnClose(CXConnectionObject& conObj, ConnectionClosedType emClosedType);

            bool SetNonblocking(int sock);

            int  WaitThread();
            int  ListenThread();

            int  AttachConnetionToModel(CXConnectionObject &conObj);
            int  DetachConnetionToModel(CXConnectionObject &conObj);

            uint64 GetConnectionsNumber() { return m_uiConnectionNumber; }

            void SetLogHandle(CXLog * handle) { m_pLogHandle = handle; }
            CXLog *GetLogHandle() { return m_pLogHandle; }

			bool  SetWaitWritingEvent(CXConnectionObject &conObj,bool bSet);

            //create a tcp socket, connect to the remote peer
            int   CreateTcpConnection(cxsocket & sock,const string &strRemoteIp, WORD wRemotePort,
                const string &strLocalIp, WORD wLocalPort, sockaddr_in& addrRemoteOut);

    public:
            BOOL PostAccept(PCXBufferObj pBufObj);
#ifdef WIN32
            //windows iocp event process
            BOOL ProcessIOCPEvent(CXConnectionObject& conObj, PCXBufferObj pBufObj,
                DWORD dwTransDataOfBytes);

            BOOL ProcessIocpErrorEvent(CXConnectionObject& conObj, LPOVERLAPPED lpOverlapped,
                DWORD dwTransDataOfBytes);
#else
            //windows epoll event process
            int ProcessEpollEvent(CXConnectionObject& conObj, bool bRead=true);
#endif
        protected:

    };
}

#endif // CXSOCKETSERVERKERNEL_H


