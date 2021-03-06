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

Description:
*****************************************************************************/
#include "CXNetworkInitEnv.h"

namespace CXCommunication
{
    CXNetworkInitEnv::CXNetworkInitEnv()
    {
    }


    CXNetworkInitEnv::~CXNetworkInitEnv()
    {
    }

    int CXNetworkInitEnv::InitEnv()
    {
#ifdef WIN32
        WORD wVer = MAKEWORD(2, 2);
        WSAData data;
        if (::WSAStartup(wVer, &data) != 0)
        {
            return -1;
        }
#else

#endif

       return 0;
    }

    int CXNetworkInitEnv::ClearEnv()
    {

#ifdef WIN32
        return WSACleanup();
#else

#endif
        return 0;

    }
}
