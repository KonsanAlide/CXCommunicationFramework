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

Description��
*****************************************************************************/
#include "CXSessionLevelBase.h"


using namespace CXCommunication;
CXSessionLevelBase::CXSessionLevelBase()
{
}


CXSessionLevelBase::~CXSessionLevelBase()
{
}

int CXSessionLevelBase::SessionLogin(PCXBufferObj pBuf, CXConnectionSession ** ppSession)
{
    return RETURN_SUCCEED;
}
int CXSessionLevelBase::SessionLogout(PCXBufferObj pBuf, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}

int CXSessionLevelBase::SessionSetting(PCXBufferObj pBuf, CXConnectionSession &session)
{
    return RETURN_SUCCEED;
}
