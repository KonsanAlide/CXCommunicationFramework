#include <iostream>
#include "CXCommunicationServer.h"
#include "CXLog.h"
#include "CXThread.h"
#include "CXFileRPCProxyServer.h"
#include "CXFastDataParserHandle.h"
#include "CXRPCObjectManager.h"
#include "CXSessionLevelBase.h"
#include "PlatformFunctionDefine.h"
#include "CXUnknownRPCObject.h"
#include "CXStorageRPCClient.h"

using namespace std;
using namespace CXCommunication;

CXLog m_logHandle;
CXLog m_logJouralHandle;
CXCommunicationServer server;

//CXSpinLock g_lock;
//int g_iTotalProcessPacket = 0;
//int g_iTotalMessage = 0;
//int g_iTotalProcessMessage = 0;
//int g_iTotalCloseNum = 0;
//int g_iTotalProcessCloseNum = 0;

bool g_bRunning = false;
string g_strLocalPath = "";
DWORD ThreadCount(void* lpvoid)
{
    //server.WaitThreadsExit();
    uint64 uiLastConnectionNumber = 0;
    uint64 uiLastIONumber = 0;
    while (g_bRunning)
    {
#ifdef WIN32
        Sleep(1000);
#else
        sleep(1);
#endif

        //uint64 uiConnectionNumber = server.GetConnectionManager().GetTotalConnectionsNumber();
        uint64 uiConnectionNumber = server.GetTotalConnectionsNumber();
        uint64 uiIONumber = server.GetTotalReceiveBuffers();
        printf("Received connections per second: %lld, iops:%lld.\n",
            uiConnectionNumber - uiLastConnectionNumber,uiIONumber - uiLastIONumber);

        //printf("Packet:[Received :%lld,processed :%d], message:[dismantled : %d, processed :%d],close event:[Received: %d,process:%d].\n",
        //    uiIONumber, g_iTotalProcessPacket,g_iTotalMessage ,g_iTotalProcessMessage,g_iTotalCloseNum,g_iTotalProcessCloseNum);
        uiLastConnectionNumber = uiConnectionNumber;
        uiLastIONumber = uiIONumber;
    }
    return 0;
}


int main()
{
    char szInfo[1024] = { 0 };

#ifdef WIN32
    char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 };
    char szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
    _splitpath(szFilePath, szDrive, szDir, szFileName, szExt);
    g_strLocalPath = szDrive;
    g_strLocalPath.append(szDir);
#else
    char szPath[1024] = { 0 };

    int ret = readlink("/proc/self/exe", szPath, sizeof(szPath) - 1);
    if (ret > 0)
    {
        g_strLocalPath = szPath;
        size_t iPos = g_strLocalPath.rfind('/');
        if (iPos != string::npos)
        {
            g_strLocalPath = g_strLocalPath.substr(0, iPos + 1);
            printf("%s\n", g_strLocalPath.c_str());
        }
    }
#endif

    if (g_strLocalPath == "")
    {
        printf("Failed to get the local path.\n");
        return -3;
    }

    if (!m_logHandle.Initialize(g_strLocalPath + "proxy.log",CXLog::CXLOG_DEBUG))
    //if (!m_logHandle.Initialize(g_strLocalPath + "server.log", CXLog::CXLOG_INFO))
    {
        printf("Failed to initialize the log recorder.\n");
        return -1;
    }

	if (!m_logJouralHandle.Initialize(g_strLocalPath + "proxy_journal.log", CXLog::CXLOG_DEBUG))
	{
		sprintf_s(szInfo, 1024, "Failed to initialize the journal log recorder.\n");
		m_logHandle.Log(CXLog::CXLOG_ERROR, szInfo);
		return -2;
	}

	

#ifdef WIN32
	CoInitialize(NULL);
#endif

    
	CXFastDataParserHandle dataParserHandle;
	dataParserHandle.LoadRSAKeyFiles(g_strLocalPath + "privateKey.conf", g_strLocalPath + "pubKey.conf");
	dataParserHandle.LoadBlowfishKeyFiles(g_strLocalPath + "key.conf", g_strLocalPath + "iv.conf");

	CXFileRPCProxyServer fileRPCServerObj;
    server.Register(fileRPCServerObj.GetClassID(),(CXRPCObjectServer*)&fileRPCServerObj);

    CXStorageRPCClient fileProxyClient;
	server.Register(fileProxyClient.GetClassID(), (CXRPCObjectServer*)&fileProxyClient);

    CXSessionLevelBase sessionLoginHandle;
    server.Register(sessionLoginHandle.GetClassID(), (CXRPCObjectServer*)&sessionLoginHandle);

	CXUnknownRPCObject unknownObj;
	server.Register(unknownObj.GetClassID(),(CXRPCObjectServer*)&unknownObj);

    server.SetDataParserHandle(&dataParserHandle);
    server.SetLogHandle(&m_logHandle);
	server.SetJournalLogHandle(&m_logJouralHandle);
    int iRet = server.Start(4355, 8);
    if(iRet !=0)
    {
        sprintf_s(szInfo, 1024, "Failed to start proxy server, return value is %d\n",iRet);
        m_logHandle.Log(CXLog::CXLOG_ERROR, szInfo);
        printf(szInfo);
#ifdef WIN32
		CoUninitialize();
#endif
        return -3;
    }
    else
    {
        sprintf_s(szInfo, 1024, "Start proxy server successfully.\n");
        m_logHandle.Log(CXLog::CXLOG_INFO, szInfo);

        printf(szInfo);
    }
    g_bRunning = true;
    CXThread threadCount;
    threadCount.Start(ThreadCount,NULL);



    cin.get();
    g_bRunning = false;
    server.Stop();
    threadCount.Wait();

	sprintf_s(szInfo, 1024, "Proxy server exit.\n");
	m_logHandle.Log(CXLog::CXLOG_INFO, szInfo);
    //delete pUserMessageProcessHandle;
#ifdef WIN32
	CoUninitialize();
#endif
    return 0;
}
