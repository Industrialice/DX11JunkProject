#ifndef __HEAP_MANAGER_HPP__
#define __HEAP_MANAGER_HPP__

#include "HeapBase.hpp"

namespace HeapManager
{
    const HeapBase::SHeap *Acquire( uiw index );
    uiw Size();

    namespace Private
    {
        uiw Add( const HeapBase::SHeap *heap );
        void Remove( uiw index );
    }
}

#endif __HEAP_MANAGER_HPP__