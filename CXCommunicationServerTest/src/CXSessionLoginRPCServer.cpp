#include "CXSessionLoginRPCServer.h"
#include "CXPacketCodeDefine.h"
#include "CXSessionPacketStructure.h"
#include "CXPacketCodeDefine.h"
#include <string>
#ifndef WIN32
#include<string.h>
#endif
using namespace CXCommunication;
using namespace std;

CXSessionLoginRPCServer::CXSessionLoginRPCServer()
{
    m_bIsUniqueInstance = true;
    GetClassGuid();
}


CXSessionLoginRPCServer::~CXSessionLoginRPCServer()
{
}

CXRPCObjectServer* CXSessionLoginRPCServer::CreateObject()
{
    return (CXRPCObjectServer*) new CXSessionLoginRPCServer;
}

void CXSessionLoginRPCServer::Destroy()
{
	Reset();
}

void CXSessionLoginRPCServer::MessageToString(PCXMessageData pMes, string &strMes)
{

}
