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
#ifndef CXSPINLOCK_H
#define CXSPINLOCK_H

#ifdef WIN32
#include <Windows.h> 
#else
#include "linux/spinlock-xchg.h"
#endif

class CXSpinLock
{
    public:
		CXSpinLock(bool bLock=false);
        virtual ~CXSpinLock();
        void Lock();
        void Unlock();
        bool TryLock();
    protected:
    private:
        bool m_bInitLocked;
#ifdef WIN32
		CRITICAL_SECTION m_lock;
#else
		spinlock m_lock;
#endif
		
        
};

#endif // KSPINLOCK_H
