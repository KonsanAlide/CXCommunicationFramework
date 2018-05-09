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
#include "CXMessageProcessLevelBase.h"
#include "CXPacketCodeDefine.h"
#include "CXConnectionsManager.h"
#include "CXCommunicationServer.h"


using namespace CXCommunication;
void* ThreadProcess(void* lpvoid);

CXMessageProcessLevelBase::CXMessageProcessLevelBase()
{
    m_pMessageQueue = NULL;
    m_bStart = false;
}


CXMessageProcessLevelBase::~CXMessageProcessLevelBase()
{
}

int  CXMessageProcessLevelBase::Run()
{
    RunFun funThread = &ThreadProcess;
    int iRet = m_threadProcess.Start(funThread, (void*)this);
    if (iRet != 0)
    {
        printf_s("Create message process thread fails\n");
        m_bStart = false;
        return -2;
    }
    
    m_bStart = true;
    return RETURN_SUCCEED;
}

int CXMessageProcessLevelBase::ProcessMessage()
{
    int iRet = 0;
    CXMessageQueue * pQueue = GetMessageQueue();
    while (IsStart())
    {
        if (pQueue->Wait(1000) == WAIT_OBJECT_0)
        {
            PCXBufferObj pMes = (PCXBufferObj)pQueue->GetMessage();
            while (pMes != NULL)
            {
                //ProcessPacket(pMes);
                PCXPacketHeader pTcpHeader = (PCXPacketHeader)pMes->wsaBuf.buf;
                PCXPacketData pPacket = (PCXPacketData)(pMes->wsaBuf.buf);
                CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
                if (pCon->GetSession() == NULL)
                {
                    if (pPacket->dwMesCode != CX_SESSION_LOGIN_CODE)
                    {
                        printf_s("The first packet is not the login packet of the session\n");
                    }
                    else
                    {
                        CXConnectionSession * pSession = NULL;
                        iRet = m_sessionLevelProcess.SessionLogin(pMes,&pSession);
                        if (iRet == ERROR_SUCCESS)
                        {
                            //pCon->SetSession((void*)pSession);
                        }
                        else
                        {
                            printf_s("Login fails\n");
                        }
                    }
                }
                else
                {
                    CXConnectionSession * pSession = (CXConnectionSession *)pCon->GetSession();
                    DWORD dwMessageCode = pPacket->dwMesCode;
                    switch (dwMessageCode)
                    {
                    case CX_SESSION_LOGIN_CODE:
                        break;
                    case CX_SESSION_LOGOUT_CODE:
                        iRet = m_sessionLevelProcess.SessionLogout(pMes, *pSession);
                        if (iRet != ERROR_SUCCESS)
                        {
                            printf_s("SessionLogout fails\n");
                            if (iRet == -2)
                            {
                                ProcessConnectionError(pCon);
                            }
                        }
                        break;
                    case CX_SESSION_SETITING_CODE:
                        iRet = m_sessionLevelProcess.SessionSetting(pMes, *pSession);
                        if (iRet != ERROR_SUCCESS)
                        {
                            printf_s("SessionSetting fails\n");
                            if (iRet == -2)
                            {
                                ProcessConnectionError(pCon);
                            }
                        }
                        break;
                    default:
                        iRet = m_userMessageProcess.ProcessPacket(pMes, pCon, pSession);
                        if (iRet != ERROR_SUCCESS)
                        {
                            printf_s("Process message fails\n");
                            if (iRet == -2)
                            {
                                ProcessConnectionError(pCon);
                            }
                        }

                        break;
                    } 
                }
                pCon->FreeBuffer(pMes);
                pMes = (PCXBufferObj)pQueue->GetMessage();

            }
        }
    }
    return RETURN_SUCCEED;
}

int  CXMessageProcessLevelBase::ProcessSessionPacket(PCXBufferObj pBuf)
{
    return RETURN_SUCCEED;
}

int  CXMessageProcessLevelBase::ProcessPacket(PCXBufferObj pBuf, CXConnectionObject * pCon,
    void *pSession)
{
    CXConnectionSession * pCurSession = (CXConnectionSession *)pSession;
    pCon->AddProcessPacketNumber();
    pCon->ReduceReceivedPacketNumber();

    //CXSessionsManager *pSessionManager = pCon->GetSessionsManager();
    //printf("####Process packet ,connection id = %I64i,datalen = %d,packet number :%I64i\n", 
    //    pCon->GetConnectionIndex(), pBuf->wsaBuf.len, pCon->GetProcessPacketNumber());

    return RETURN_SUCCEED;
}

void* ThreadProcess(void* lpvoid)
{
    CXMessageProcessLevelBase* pServer = (CXMessageProcessLevelBase*)lpvoid;
    pServer->ProcessMessage();
    return 0;
}


void CXMessageProcessLevelBase::Stop()
{
    m_bStart = false;
    m_threadProcess.Wait();
}

int CXMessageProcessLevelBase::ProcessConnectionError(CXConnectionObject * pCon)
{
    CXCommunicationServer *pServer = (CXCommunicationServer *)pCon->GetServer();
    CXConnectionsManager & connectionsManager = pServer->GetConnectionManager();
    pServer->CloseConnection(*pCon);
    return 0;
}