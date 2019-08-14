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
#include "CXGuidObject.h"
//#include "PlatformDataTypeDefine.h"
#include "PlatformFunctionDefine.h"

using namespace CXCommunication;
void* ThreadProcess(void* lpvoid);

//extern CXSpinLock g_lock;
//extern int g_iTotalProcessMessage;

CXMessageProcessLevelBase::CXMessageProcessLevelBase()
{
    m_pMessageQueue = NULL;
    m_bStart = false;
	m_pLogHandle = NULL;
    m_pRPCObjectManager = NULL;
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
        printf("Create message process thread fails\n");
        m_bStart = false;
        return -2;
    }

    m_bStart = true;
    return RETURN_SUCCEED;
}

int CXMessageProcessLevelBase::ProcessMessage()
{
    int iRet = 0;
    int iLoopNum = 0;
	int64 iBeginTimeInProcess = 0;
    CXGuidObject guidObj(false);
    CXMessageQueue * pQueue = GetMessageQueue();
    while (IsStart())
    {
        pQueue->Wait(1000);
        //if (pQueue->Wait(1000) == WAIT_OBJECT_0)
        {
            iLoopNum = 0;
            PCXMessageData pMes = (PCXMessageData)pQueue->GetMessage();
            while (pMes != NULL)
            {
                CXConnectionObject *pCon = (CXConnectionObject*)pMes->pConObj;
                if (pCon != NULL)
                {
                    //pCon->AddProcessPacketNumber();
                }
				else
				{
					char szInfo[1024] = { 0 };
					sprintf_s(szInfo, 1024, "Find a incorrect packet, the connection object is empty,pMes=%x\n", (byte*)pMes);
					m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
					return -1;
				}

				iRet = pCon->ProcessUnpackedMessage(pMes);
				/*
				iBeginTimeInProcess = pCon->GetCurrentTimeMS(NULL);
                iRet = 0;

                CXConnectionSession * pSession = (CXConnectionSession *)pCon->GetSession();
                string strObjectGuid = guidObj.ConvertGuid(pMes->bodyData.byObjectGuid);

                bool bLoginMes = false;
                int iLoop = 2;
                while (--iLoop>0)
                {
                    if (strObjectGuid != "")
                    {
                        CXRPCObjectServer * pObjectServer = NULL;
                        if (pSession != NULL)
                        {
                            pSession->Lock();
                            pObjectServer = (CXRPCObjectServer *)pSession->GetData(strObjectGuid, false);
                        }

                        if (pObjectServer != NULL)
                        {
							if (pSession != NULL)
								pSession->UnLock();
                        }
                        else
                        {
                            pObjectServer = m_pRPCObjectManager->GetRPCObject(strObjectGuid);
                            if (pObjectServer != NULL)
                            {
								if (!pObjectServer->IsUniqueInstance())
								{
									if(pSession!=NULL)
										pSession->SetData(strObjectGuid, (void*)pObjectServer, false);
									pObjectServer->SetSession(pSession);
								}
								pObjectServer->SetLogHandle(pCon->GetLogHandle());
								pObjectServer->SetJournalLogHandle(pCon->GetJournalLogHandle());
								pObjectServer->SetIOStat(pCon->GetIOStat());
                            }
                            else //unknown object
                            {
								char szInfo[1024] = { 0 };
								sprintf_s(szInfo, 1024, "Not found the object %s, maybe not registered\n", strObjectGuid.c_str());
								m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

								strObjectGuid = "{0307F567-72FC-4355-8192-9E37DC766D2E}";
								if (pSession != NULL)
									pSession->UnLock();
								continue;
                                
                            }
							if (pSession != NULL)
								pSession->UnLock();
                        }
                        

                        if (pObjectServer != NULL)
                        {
                            iRet = pObjectServer->ProcessMessage(pMes);
                            if (iRet != RETURN_SUCCEED)
                            {
                                printf("Process message fails\n");
                                if (iRet == -2)
                                {
                                    pCon->Lock();
                                    ProcessConnectionError(pCon);
                                    pCon->UnLock();
                                }
                            }
                        }  
                    }
                    else
                    {
                        char szInfo[1024] = { 0 };
                        sprintf_s(szInfo, 1024, "Receive a incorrect packet, the object guid is empty\n");
                        m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
                    }
                    break;
                }

				//pCon->OutputJournal(pMes,iBeginTime);
                pCon->FreeBuffer(pMes);

                //<Process the closing event of this socket>
                //pCon->Lock();
                if (pCon->GetState() >= CXConnectionObject::CLOSING)//closing
                {
                    pCon->Lock();
                    pCon->ReduceReceivedPacketNumber();
                    if (pCon->GetState() == CXConnectionObject::CLOSING)//closed
                    {
                        //ProcessConnectionError(pCon);
                        char szInfo[1024] = {0};
                        sprintf_s(szInfo,1024, "Close a closing connection after process a message,connection id=%lld,error code is %d,desc is '%s'\n",
                            pCon->GetConnectionIndex(),errno,strerror(errno));
                        //m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);
                        CXCommunicationServer *pServer = (CXCommunicationServer *)pCon->GetServer();
                        pServer->CloseConnection(*pCon, SOCKET_CLOSED,false);
                    }
                    pCon->UnLock();
                }
                else
                {
                    pCon->ReduceReceivedPacketNumber();
                }
				*/
                //g_lock.Lock();
                //g_iTotalProcessMessage++;
                //g_lock.Unlock();

                pMes = (PCXMessageData)pQueue->GetMessage();
                /*
                while (pMes==NULL && iLoopNum<1000)
                {
                    pMes = (PCXBufferObj)pQueue->GetMessage();
                    iLoopNum++;
                }
                if(pMes)
                    iLoopNum = 0;
                    */
            }
        }
    }
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

    char szInfo[1024] = {0};
    sprintf_s(szInfo,1024, "Failed to process message,close this connection,connection id=%lld,error code is %d,desc is '%s'\n",
                  pCon->GetConnectionIndex(),errno,strerror(errno));
    //m_pLogHandle->Log(CXLog::CXLOG_ERROR, szInfo);

    pServer->CloseConnection(*pCon, ERROR_IN_PROCESS,false);
    return 0;
}
