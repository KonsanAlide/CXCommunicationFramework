/****************************************************************************
Copyright (c) 2018 Chance Yang

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
#include "CXTcpClient.h"
#include "CXCommonPacketStructure.h"
#include "CXSessionPacketStructure.h"
#include "CXPacketCodeDefine.h"

#ifdef WIN32
#else
#include "PlatformFunctionDefine.h"
#include <string.h>
#endif

namespace CXCommunication
{
    CXTcpClient::CXTcpClient()
    {
        m_bCreated = false;
        m_bConnected = false;
        m_bClosing = false;
        m_iPacketNumber = 0;

        m_pbySendBuffer = NULL;
        m_dwSendBufferLen = 0;

		m_pbyCacheSendBuffer = NULL;
		m_dwCacheSendBufferLen = 0;

        m_pbyRealSendBuffer = NULL;
        m_dwRealSendBufferLen = 0;

        m_pbyRecvBuffer = NULL;
        m_dwRecvBufferLen = 0;

        m_pbyParserBuffer = NULL;
        m_dwParserBufferLen = 0;

		//using to uncompress and decrypt data
		m_pbyCacheRecvBuffer = NULL;
	    m_dwCacheRecvBufferLen = 0;

        m_pDataParserHandle = NULL;

		m_encryptType = CXDataParserImpl::CXENCRYPT_TYPE_NONE;
		m_compressType = CXDataParserImpl::CXCOMPRESS_TYPE_NONE;

        m_dwLeftDataBeginPos = 0;
        m_dwLeftRecvDataLen = 0;

		memset(m_byRPCObjectGuid, 0, CX_GUID_LEN);
        memset(m_byRequestID, 0, CX_GUID_LEN);
        

		m_bCloseInDeconstruction = true;

		m_lpCacheObj = NULL;
		m_bUsedMemoryCachePool = false;

        m_bGetRealObjectID = false;
    }


    CXTcpClient::~CXTcpClient()
    {
		if (m_bCloseInDeconstruction)
		{
			Close();
		}
        if (!m_bUsedMemoryCachePool)
        {
            if (m_pbySendBuffer != NULL)
            {
                delete[]m_pbySendBuffer;
            }

            if (m_pbyRealSendBuffer != NULL)
            {
                delete[]m_pbyRealSendBuffer;
            }

            if (m_pbyCacheSendBuffer != NULL)
            {
                delete[]m_pbyCacheSendBuffer;
            }

            if (m_pbyRecvBuffer != NULL)
            {
                delete[]m_pbyRecvBuffer;
            }

            if (m_pbyParserBuffer != NULL)
            {
                delete[]m_pbyParserBuffer;
            }

            if (m_pbyCacheRecvBuffer != NULL)
            {
                delete[]m_pbyCacheRecvBuffer;
            }
        }

        m_pbySendBuffer = NULL;
        m_dwSendBufferLen = 0;

        m_pbyCacheSendBuffer = NULL;
        m_dwCacheSendBufferLen = 0;

        m_pbyRealSendBuffer = NULL;
        m_dwRealSendBufferLen = 0;

        m_pbyRecvBuffer = NULL;
        m_dwRecvBufferLen = 0;

        m_pbyParserBuffer = NULL;
        m_dwParserBufferLen = 0;
        
        m_pbyCacheRecvBuffer = NULL;
        m_dwCacheRecvBufferLen = 0;

        m_bGetRealObjectID = false;
    }

    //create a socket,set the address and flags
    //Received value: ==RETURN_SUCCEED the socket was created successfully
    //                ==INVALID_PARAMETER invalid inputed parameters
    //                ==-2 socket creation failed
    //                ==-3 socket creation failed, a error accured when allocated memory
    int CXTcpClient::Create(bool bAccepted, cxsocket sock)
    {
        if (bAccepted && sock <= 0)
        {
            return INVALID_PARAMETER;
        }
		
		if (!SetSendBufferSize(CLIENT_BUF_SIZE))
		{
			return -3;
		}
		if (!SetRecvBufferSize(CLIENT_BUF_SIZE))
		{
			return -3;
		}

        if (bAccepted)
        {
            if (m_socket.CreateByAccepted(sock) == 0)
            {
                m_bConnected = true;
                m_bCreated = true;
            }
        }
        else
        {
            if (m_socket.Create() == 0)
            {
                m_bCreated = true;
            }
        }

        if (!m_bCreated)
        {
            return -2;
        }
        else
        {
            m_socket.SetNoDelay(true);
            return RETURN_SUCCEED;
        }
        
        return RETURN_SUCCEED;
    }

    //connect to the peer by the ip address
    //Received value: ==RETURN_SUCCEED the socket was created successfully
    //                ==INVALID_PARAMETER invalid inputed parameters
    //                ==-2 this socket object had not been created
    //                ==-3 failed to connect the peer
    int CXTcpClient::Connect(const CXSocketAddress& address)
    {
        if (IsConnected())
        {
            return RETURN_SUCCEED;
        }
        if (!IsCreated())
        {
            return -2;
        }
        int iRet = m_socket.Connect(address);
        if (iRet == 0)
        {
            m_socket.SetNoDelay(true);
            m_bConnected = true;
            m_addressRemote.SetAddress(address.GetAddress());
            return RETURN_SUCCEED;
        }
        else
        {
            DWORD dwEr = GetLastError();
            Close();
            return -3;
        }
    }

    int CXTcpClient::Recv(byte *bpBuf, int iWantRecvLen, int &iReceivedBytes)
    {
        if (bpBuf==NULL || iWantRecvLen <= 0 )
        {
            return INVALID_PARAMETER;
        }
        if (!IsConnected() || !IsCreated()|| IsClosing())
        {
            return -2;
        }

        int iRecv = 0;
        unsigned int iOffset = 0;
        bool bRecvFailed = false;
        while (iOffset < iWantRecvLen)
        {
            if (IsClosing())
            {
                bRecvFailed = true;
                break;
            }

            iRecv = m_socket.RecvBytes(bpBuf+ iOffset, iWantRecvLen- iOffset);

            if (iRecv != SOCKET_ERROR)
            {
                if (iRecv == 0)
                {
                    Close();
                    break;
                }
                iOffset += iRecv;
            }
            else
            {
                bRecvFailed = true;
                Close();
                break;
            }

        }

        iReceivedBytes = iOffset;

        if (bRecvFailed)
        {
            return -3;
        }
        else if (iRecv == 0) // read to the end , this socket had been closed
        {
            return 0;
        }
        else
        {
            return iOffset;
        }
    }

    int CXTcpClient::Send(const byte *bpBuf, int iWantSendLen, int &iSentBytes)
    {
        if (bpBuf == NULL || iWantSendLen <= 0)
        {
            return INVALID_PARAMETER;
        }
        if (!IsConnected() || !IsCreated() || IsClosing())
        {
            return -2;
        }

        int iSent = 0;
        unsigned int iOffset = 0;
        bool bSendFailed = false;
        do
        {
            if (IsClosing())
            {
                bSendFailed = true;
                break;
            }

            iSent = m_socket.SendBytes(bpBuf + iOffset, iWantSendLen - iOffset);

            if (iSent != SOCKET_ERROR)
            {
                if (iSent == 0)
                    break;
                iOffset += iSent;
            }
            else
            {
				DWORD dwEr = GetLastError();
                bSendFailed = true;
                break;
            }

        } while (iOffset < iWantSendLen);

        iSentBytes = iOffset;

        if (bSendFailed)
        {
            return -3;
        }
        else if (iSent == 0) // read to the end , this socket had been closed
        {
            return 0;
        }
        else
        {
            return iOffset;
        }
    }

    int CXTcpClient::Close()
    {
        if (IsConnected())
        {
            m_socket.SetNoDelay(true);
            m_socket.SetLinger(true,0);
            m_bClosing = true;
            m_socket.Close();
            //m_socket.Shutdown();
        }
        else
        {
            if (IsCreated())
            {
                m_socket.Close();
            }
        }

        m_bClosing = false;
        m_bConnected = false;

        m_dwLeftDataBeginPos = 0;
        m_dwLeftRecvDataLen = 0;

        m_bGetRealObjectID = false;

        return 0;
    }

    //send a packet to the peer
    int  CXTcpClient::SendPacket(const byte* pbData, DWORD dwLen, DWORD dwMesCode)
    {
        if (pbData==NULL || dwLen > 1024 * 1024* 1024)
        {
            return INVALID_PARAMETER;
        }

		if (m_encryptType != CXDataParserImpl::CXENCRYPT_TYPE_NONE ||
			m_compressType != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
		{
			if (m_pDataParserHandle == NULL)
			{
				return INVALID_PARAMETER;
			}
		}

        byte *pbyBodyData = m_pbyRealSendBuffer+sizeof(CXPacketHeader);
        DWORD dwPacketLen = dwLen + sizeof(CXPacketData) - 1;
        DWORD dwOrignDataLen = dwPacketLen- sizeof(CXPacketHeader);

        m_guidObj.GenerateNewGuid(m_byRequestID);

        //use a same buffer
        if (!(pbData >= m_pbySendBuffer && pbData < (m_pbySendBuffer + m_dwSendBufferLen)))
        {
            if (!SetSendBufferSize(dwLen))
            {
                return -2;
            }
        }

		if (m_encryptType != CXDataParserImpl::CXENCRYPT_TYPE_NONE ||
			m_compressType != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
		{
			DWORD dwSrcLen = dwLen + sizeof(CXPacketBodyData) - 1;
			DWORD dwDestLen = m_dwRealSendBufferLen - sizeof(CXPacketHeader);

			PCXPacketBodyData pBodyData = (PCXPacketBodyData)(m_pbySendBuffer + sizeof(CXPacketHeader));
			pBodyData->dwMesCode = dwMesCode;
			pBodyData->dwPacketNum = ++m_iPacketNumber;
			memcpy(pBodyData->byObjectGuid, m_byRPCObjectGuid, CX_GUID_LEN);
            memcpy(pBodyData->byRequestID,m_byRequestID, CX_GUID_LEN);

			// not use the socket buffer to save data
			if (pbData < m_pbySendBuffer || pbData >(m_pbySendBuffer + m_dwSendBufferLen))
			{
				if (dwLen > 0)
				{
					memcpy(pBodyData->buf, pbData, dwLen);
				}
			}
			
			if (!m_pDataParserHandle->PrepareData(m_encryptType, m_compressType,
				(const byte*)pBodyData, dwSrcLen,
				pbyBodyData, dwDestLen, 
				m_pbyCacheSendBuffer, m_dwCacheSendBufferLen,
				dwDestLen))
			{
				return -3;
			}
			
			dwPacketLen = dwDestLen + sizeof(CXPacketHeader);
		}
        else
        {
            PCXPacketBodyData pBodyData = (PCXPacketBodyData)(m_pbyRealSendBuffer + sizeof(CXPacketHeader));
            pBodyData->dwMesCode = dwMesCode;
            pBodyData->dwPacketNum = ++m_iPacketNumber;
			memcpy(pBodyData->byObjectGuid, m_byRPCObjectGuid, CX_GUID_LEN);
            memcpy(pBodyData->byRequestID, m_byRequestID, CX_GUID_LEN);

            // not use the socket buffer to save data
            if (pbData < m_pbyRealSendBuffer || pbData >(m_pbyRealSendBuffer + m_dwRealSendBufferLen))
            {
                if (dwLen>0)
                {
                    memcpy(pBodyData->buf, pbData, dwLen);
                }
            }
        }

        int iPacketBodyLen = dwPacketLen - sizeof(CXPacketHeader);

        DWORD dwCheckSum = 0;
        for (int i = 0; i<iPacketBodyLen; i++)
        {
            dwCheckSum += (unsigned int)pbyBodyData[i];
        }

        //build the header
        PCXPacketHeader pTcpHeader = (PCXPacketHeader)m_pbyRealSendBuffer;
        pTcpHeader->dwDataLen = iPacketBodyLen;
        pTcpHeader->byCompressFlag =(byte)m_compressType;
        pTcpHeader->byEncryptFlag = (byte)m_encryptType;
        pTcpHeader->wFlag = CX_PACKET_HEADER_FLAG;
        pTcpHeader->dwCheckSum = dwCheckSum;
        pTcpHeader->dwOrignDataLen = dwOrignDataLen;
       

        int iTransDataLen = 0;
        int iTransRet = Send(m_pbyRealSendBuffer, dwPacketLen, iTransDataLen);
        if (iTransRet != dwPacketLen)
        {
            Close();
            return -6;
        }
        return 0;
    }

    int CXTcpClient::RecvPacket(byte *pData, int iDataLen,int &iReadBytes, DWORD &dwMesCode)
    {
		if (pData == NULL )
		{
			return INVALID_PARAMETER;
		}
		memset(pData, 0, iDataLen);
        m_dwLeftRecvDataLen = 0;
        m_dwLeftDataBeginPos = 0;
		
		int iRecvLen = 0;
		int iRet = RecvPacket(iRecvLen, dwMesCode);
		if (iRet == 0 && iRecvLen>0)
		{	
			PCXPacketBodyData pBodyData = (PCXPacketBodyData)m_pbyRecvBuffer;
			if (iDataLen < iRecvLen)
			{
				memcpy(pData, pBodyData->buf, iDataLen);
				m_dwLeftRecvDataLen = iRecvLen - iDataLen;
				m_dwLeftDataBeginPos = iDataLen;
				iReadBytes = iDataLen;
				return -8;
			}
			else
			{
				memcpy(pData, pBodyData->buf, iRecvLen);
				iReadBytes = iRecvLen;
			}
		}

		return iRet;
    }

	int CXTcpClient::RecvPacket(int &iReadBytes, DWORD &dwMesCode)
	{
		if (!IsConnected() || !IsCreated() || IsClosing())
		{
			return -2;
		}
		
		m_dwLeftDataBeginPos = 0;
		m_dwLeftRecvDataLen = 0;


		int iTransRet = 0;
		int iTransLen = 0;
		iReadBytes = 0;
		CXPacketHeader header;
		memset(&header, 0, sizeof(CXPacketHeader));

		int iNeedSend = sizeof(CXPacketHeader);

		iTransRet = Recv((byte*)&header, iNeedSend, iTransLen);

		if (iTransRet <= 0)
		{
			return -3;
		}

		if (header.dwDataLen > 1024 * 1024* 1024)
		{
			Close();
			return -4;
		}

		if (header.wFlag != CX_PACKET_HEADER_FLAG)
		{
			Close();
			return -5;
		}

		if (!SetRecvBufferSize(header.dwDataLen))
		{
			return -6;
		}

		iTransRet = 0;
		iTransLen = 0;
		iNeedSend = header.dwDataLen;
		iTransRet = Recv(m_pbyRecvBuffer, iNeedSend, iTransLen);

		if (iTransRet != iNeedSend)
		{
			return -7;
		}

        PCXPacketBodyData pBodyData = NULL;
		if ((CXDataParserImpl::CXENCRYPT_TYPE)header.byEncryptFlag != CXDataParserImpl::CXENCRYPT_TYPE_NONE ||
			(CXDataParserImpl::CXCOMPRESS_TYPE)header.byCompressFlag != CXDataParserImpl::CXCOMPRESS_TYPE_NONE)
		{
			if (m_pDataParserHandle == NULL)
			{
				return -3;
			}
			DWORD dwDestLen = m_dwParserBufferLen;

			if (!m_pDataParserHandle->ParseData((CXDataParserImpl::CXENCRYPT_TYPE)header.byEncryptFlag, 
				(CXDataParserImpl::CXCOMPRESS_TYPE)header.byCompressFlag,
				m_pbyRecvBuffer, iTransLen,
				m_pbyParserBuffer, dwDestLen, 
				m_pbyCacheRecvBuffer, m_dwCacheRecvBufferLen,
				dwDestLen))
			{
				return -3;
			}
			memcpy(m_pbyRecvBuffer, m_pbyParserBuffer, dwDestLen);

			pBodyData = (PCXPacketBodyData)m_pbyRecvBuffer;
			dwMesCode = pBodyData->dwMesCode;
			DWORD dwRealDataLen = dwDestLen - (sizeof(CXPacketBodyData) - 1);

			iReadBytes = dwRealDataLen;
		}
		else
		{
			pBodyData = (PCXPacketBodyData)m_pbyRecvBuffer;
			dwMesCode = pBodyData->dwMesCode;
			DWORD dwRealDataLen = iTransLen - (sizeof(CXPacketBodyData) - 1);
			
			iReadBytes = dwRealDataLen;
		}

        if (!m_bGetRealObjectID)
        {
            //if the object id had been changed by the server, also change it in here 
            if (memcmp(pBodyData->byObjectGuid, m_byRPCObjectGuid, CX_GUID_LEN) != 0)
            {
                if (memcmp(pBodyData->byRequestID, m_byRequestID, CX_GUID_LEN) == 0)
                {
                    memcpy(m_byRPCObjectGuid, pBodyData->byObjectGuid, CX_GUID_LEN);
                    m_bGetRealObjectID = true;
                }
            }
        }

		return RETURN_SUCCEED;
	}

    bool CXTcpClient::ReadLeftData(byte *pData, int iDataLen, int &iReadBytes)
    {
        if (m_dwLeftRecvDataLen == 0)
            return true;

        iReadBytes = 0;
        if (iDataLen < m_dwLeftRecvDataLen) //left some data
        {
            return false;
        }
        else
        {
            PCXPacketBodyData pBodyData = (PCXPacketBodyData)m_pbyRecvBuffer;
            memcpy(pData, pBodyData->buf + m_dwLeftDataBeginPos, m_dwLeftRecvDataLen);
            iReadBytes = m_dwLeftRecvDataLen;
            m_dwLeftRecvDataLen = 0;
            m_dwLeftDataBeginPos = 0;
            return true;
        }
    }


    bool CXTcpClient::SetSendBufferSize(DWORD dwBufSize)
    {
        dwBufSize += (sizeof(CXPacketData)-1);
        if (dwBufSize > 0 && dwBufSize <= m_dwSendBufferLen)
        {
            return true;
        }

        byte *pData = NULL;
        byte *pDataReal = NULL;
		byte *pCacheData = NULL;

		//used memory cache
		if (m_lpCacheObj && m_bUsedMemoryCachePool)
		{
			pData = (byte *)m_lpCacheObj->GetBuffer(dwBufSize);
			pDataReal = (byte *)m_lpCacheObj->GetBuffer(dwBufSize * 2);
			pCacheData = (byte *)m_lpCacheObj->GetBuffer(dwBufSize * 2);
			if (pData != NULL && pDataReal != NULL && pCacheData != NULL)
			{
				if (m_pbySendBuffer != NULL)
				{
					delete[]m_pbySendBuffer;
					m_pbySendBuffer = NULL;
				}
				m_pbySendBuffer = pData;
				m_dwSendBufferLen = dwBufSize;

				if (m_pbyRealSendBuffer != NULL)
				{
					delete[]m_pbyRealSendBuffer;
					m_pbyRealSendBuffer = NULL;
				}
				m_pbyRealSendBuffer = pDataReal;
				m_dwRealSendBufferLen = dwBufSize * 2;

				if (m_pbyCacheSendBuffer != NULL)
				{
					delete[]m_pbyCacheSendBuffer;
					m_pbyCacheSendBuffer = NULL;
				}
				m_pbyCacheSendBuffer = pCacheData;
				m_dwCacheSendBufferLen = dwBufSize * 2;
				return true;
			}
			else
			{
				if (pData != NULL)
				{
					m_lpCacheObj->FreeBuffer(pData);
				}
				if (pDataReal != NULL)
				{
					m_lpCacheObj->FreeBuffer(pDataReal);
				}
				if (pCacheData != NULL)
				{
					m_lpCacheObj->FreeBuffer(pCacheData);
				}
				return false;
			}
		}


        try
        {
            pData = new byte[dwBufSize];
            if (pData == NULL)
            {
                return false;
            }
			pDataReal = new byte[dwBufSize*2];
            if (pDataReal == NULL)
            {
                delete[]pData;
                return false;
            }

			pCacheData = new byte[dwBufSize * 2];
			if (pCacheData == NULL)
			{
				delete[]pData;
				delete[]pDataReal;
				return false;
			}

            
            if (m_pbySendBuffer != NULL)
            {
                delete[]m_pbySendBuffer;
                m_pbySendBuffer = NULL;
            }
            m_pbySendBuffer = pData;
            m_dwSendBufferLen = dwBufSize;

            if (m_pbyRealSendBuffer != NULL)
            {
                delete[]m_pbyRealSendBuffer;
                m_pbyRealSendBuffer = NULL;
            }
            m_pbyRealSendBuffer = pDataReal;
            m_dwRealSendBufferLen = dwBufSize*2;

			if (m_pbyCacheSendBuffer != NULL)
			{
				delete[]m_pbyCacheSendBuffer;
				m_pbyCacheSendBuffer = NULL;
			}
			m_pbyCacheSendBuffer = pCacheData;
			m_dwCacheSendBufferLen = dwBufSize * 2;

            return true;
        }
        catch (const bad_alloc& e)
        {
            return false;
        }
    }

    byte *CXTcpClient::GetSendBuffer(DWORD &dwBufSize)
    {
        dwBufSize = m_dwSendBufferLen- (sizeof(CXPacketData) - 1);
        return (m_pbySendBuffer+ (sizeof(CXPacketData) - 1));
    }

    bool CXTcpClient::SetRecvBufferSize(DWORD dwBufSize)
    {
        dwBufSize+= (sizeof(CXPacketBodyData) - 1);
        if (dwBufSize > 0 && dwBufSize <= m_dwRecvBufferLen)
        {
            return true;
        }

        byte *pData = NULL;
        byte *pDataParser = NULL;
		byte *pCacheData = NULL;

		//used memory cache
		if (m_lpCacheObj && m_bUsedMemoryCachePool)
		{
			pData = (byte *)m_lpCacheObj->GetBuffer(dwBufSize);
			pDataParser = (byte *)m_lpCacheObj->GetBuffer(dwBufSize * 2);
			pCacheData = (byte *)m_lpCacheObj->GetBuffer(dwBufSize * 2);
			if (pData != NULL && pDataParser != NULL && pCacheData != NULL)
			{
				if (m_pbyRecvBuffer != NULL)
				{
					delete[]m_pbyRecvBuffer;
					m_pbyRecvBuffer = NULL;
				}
				m_pbyRecvBuffer = pData;
				m_dwRecvBufferLen = dwBufSize;

				if (m_pbyParserBuffer != NULL)
				{
					delete[]m_pbyParserBuffer;
					m_pbyParserBuffer = NULL;
				}

				m_pbyParserBuffer = pDataParser;
				m_dwParserBufferLen = dwBufSize * 2;

				if (m_pbyCacheRecvBuffer != NULL)
				{
					delete[]m_pbyCacheRecvBuffer;
					m_pbyCacheRecvBuffer = NULL;
				}

				m_pbyCacheRecvBuffer = pCacheData;
				m_dwCacheRecvBufferLen = dwBufSize * 2;
				return true;
			}
			else
			{
				if (pData != NULL)
				{
					m_lpCacheObj->FreeBuffer(pData);
				}
				if (pDataParser != NULL)
				{
					m_lpCacheObj->FreeBuffer(pDataParser);
				}
				if (pCacheData != NULL)
				{
					m_lpCacheObj->FreeBuffer(pCacheData);
				}
				return false;
			}
		}

        try
        {
            pData = new byte[dwBufSize];
            if (pData == NULL)
            {
                return false;
            }

            pDataParser = new byte[dwBufSize*2];
            if (pDataParser == NULL)
            {
                delete[]pData;
                return false;
            }

			pCacheData = new byte[dwBufSize * 2];
			if (pCacheData == NULL)
			{
				delete[]pData;
				delete[]pDataParser;
				return false;
			}

            if (m_pbyRecvBuffer != NULL)
            {
                delete[]m_pbyRecvBuffer;
                m_pbyRecvBuffer = NULL;
            }
            m_pbyRecvBuffer = pData;
            m_dwRecvBufferLen = dwBufSize;
 
            if (m_pbyParserBuffer != NULL)
            {
                delete[]m_pbyParserBuffer;
                m_pbyParserBuffer = NULL;
            }

            m_pbyParserBuffer = pDataParser;
            m_dwParserBufferLen = dwBufSize * 2;

			if (m_pbyCacheRecvBuffer != NULL)
			{
				delete[]m_pbyCacheRecvBuffer;
				m_pbyCacheRecvBuffer = NULL;
			}

			m_pbyCacheRecvBuffer = pCacheData;
			m_dwCacheRecvBufferLen = dwBufSize * 2;

            return true;
        }
        catch (const bad_alloc& e)
        {
            return false;
        }
    }

    byte *CXTcpClient::GetRecvBuffer(DWORD &dwBufSize)
    {
        dwBufSize = m_dwRecvBufferLen - (sizeof(CXPacketBodyData) - 1);
        return (m_pbyRecvBuffer + (sizeof(CXPacketBodyData) - 1));
    }

    //get the parse buffer
    byte * CXTcpClient::GetRealRecvBuffer(DWORD &dwBufSize)
    {
        dwBufSize = m_dwParserBufferLen - (sizeof(CXPacketBodyData) - 1);
        return (m_pbyParserBuffer + (sizeof(CXPacketBodyData) - 1));
    }

	void CXTcpClient::SetCloseInDeconstruction(bool bSet)
	{ 
		m_bCloseInDeconstruction = bSet; 
		m_socket.SetCloseInDeconstruction(bSet);
	}

	void CXTcpClient::SetUsedMemoryCachePool(bool bSet, CXMemoryCacheManager* pCacheObj)
	{
		m_lpCacheObj = pCacheObj;
		m_bUsedMemoryCachePool = bSet;
	}


	cxsocket CXTcpClient::GetSocket()
	{
		return m_socket.GetSocketValue();
	}
}
