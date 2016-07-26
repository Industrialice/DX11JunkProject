#ifndef __HEAPWIN_HPP__
#define __HEAPWIN_HPP__

#include "HeapManager.hpp"

namespace HeapWin
{
    void Create( heapid target, DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize );
    void Destroy( heapid target );
}

#endif __HEAPWIN_HPP__