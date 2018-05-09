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
#include "CXSocketAddress.h"

namespace CXCommunication
{
	CXSocketAddress::CXSocketAddress(const char *ip, unsigned short port, bool b64bitAddr)
	{
        if (!b64bitAddr)
        {
            memset(&m_addr, 0, sizeof(sockaddr_in));
            m_addr.sin_family = AF_INET;
            m_addr.sin_addr.s_addr = inet_addr(ip);
            m_addr.sin_port = htons(port);
        }
		
        m_b64bitAddr = b64bitAddr;
	}

	CXSocketAddress::CXSocketAddress(const sockaddr_in &addr) : m_addr(addr)
	{
	}

    CXSocketAddress::~CXSocketAddress()
    {
    }

	string CXSocketAddress::GetIP() const
	{
        char buffer[32] = {0};
		byte *bytes = (byte*) &m_addr.sin_addr;
        snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
        return buffer;
	}
}

