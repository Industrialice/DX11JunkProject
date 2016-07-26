#include "PreHeader.hpp"
#include <Misc.hpp>
#include "HeapManager.hpp"

using namespace HeapBase;

namespace
{
    uiw HeapsCount;
    uiw HeapsReserved;
    SHeap *Heaps;
}

static void InitializeManager();

const SHeap *HeapManager::Acquire( uiw index )
{
    ASSUME( index < HeapsCount );
    return &Heaps[ index ];
}

uiw HeapManager::Size()
{
    return HeapsCount;
}

uiw HeapManager::Private::Add( const SHeap *heap )
{
    if( !HeapsReserved )
    {
        InitializeManager();
    }
    if( HeapsCount == HeapsReserved )
    {
        ++HeapsReserved;
        SHeap *newHeaps = (SHeap *)VirtualMem::VM_Alloc( sizeof(SHeap) * HeapsReserved, VirtualMem::PageMode::Read | VirtualMem::PageMode::Write );
        ASSUME( newHeaps );
        _MemCpy( newHeaps, Heaps, sizeof(SHeap) * HeapsCount );
        VirtualMem::VM_Free( Heaps );
        Heaps = newHeaps;
        ASSUME( Heaps );
    }
    _MemCpy( &Heaps[ HeapsCount++ ], heap, sizeof(SHeap) );
    return HeapsCount - 1;
}

void HeapManager::Private::Remove( uiw index )
{
    ASSUME( HeapsReserved && index < HeapsCount );
    --HeapsCount;
    _MemMove( &Heaps[ index ], &Heaps[ index + 1 ], sizeof(SHeap) * (HeapsCount - index) );
    if( HeapsReserved - HeapsCount > 32 )
    {
        HeapsReserved -= 16;
        SHeap *newHeaps = (SHeap *)VirtualMem::VM_Alloc( sizeof(SHeap) * HeapsReserved, VirtualMem::PageMode::Read | VirtualMem::PageMode::Write );
        ASSUME( newHeaps );
        _MemCpy( newHeaps, Heaps, sizeof(SHeap) * HeapsCount );
        VirtualMem::VM_Free( Heaps );
        Heaps = newHeaps;
        ASSUME( Heaps );
    }
}

void InitializeManager()
{
    HeapsReserved = 32;
    Heaps = (SHeap *)VirtualMem::VM_Alloc( sizeof(SHeap) * HeapsReserved, VirtualMem::PageMode::Read | VirtualMem::PageMode::Write );
    ASSUME( Heaps );
}