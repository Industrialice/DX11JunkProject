#ifndef __HEAPREF_HPP__
#define __HEAPREF_HPP__

#include "HeapManager.hpp"

namespace HeapRef
{
    void Create( heapid target );
    void Destroy( heapid target );
}

#endif __HEAPREF_HPP__