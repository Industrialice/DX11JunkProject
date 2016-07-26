#ifndef __HEAP_HPP__
#define __HEAP_HPP__

#include "HeapManager.hpp"

namespace Heap
{
    struct DefaultDeleter
    {
        DefaultDeleter( void *addr )
        {
            HeapBase::SHeap *po_heap = (HeapBase::SHeap *)HeapManager::Acquire( 0 );
            po_heap->Free( po_heap->id, addr );
        }
    };

    static UNIQUEPTRRETURN void *RSTR Alloc( uiw size, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return po_heap->Alloc( po_heap->id, size );
    }

    template < typename X > UNIQUEPTRRETURN X *RSTR Alloc( uiw count, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return (X *)po_heap->Alloc( po_heap->id, count * sizeof(X) );
    }

    static void Free( void *addr, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        po_heap->Free( po_heap->id, addr );
    }

    template < typename X > X *Realloc( X *addr, uiw size, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return (X *)po_heap->Realloc( po_heap->id, addr, size );
    }

    static bln ReallocInplaceIfPossible( void *addr, uiw size, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return po_heap->ReallocInplaceIfPossible( po_heap->id, addr, size );
    }

    static UNIQUEPTRRETURN void *RSTR AllocAligned( uiw size, uiw alignment, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return po_heap->AllocAligned( po_heap->id, size, alignment );
    }

    static void FreeAligned( void *addr, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        po_heap->FreeAligned( po_heap->id, addr );
    }

    template < typename X > X *ReallocAligned( X *addr, uiw size, uiw alignment, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return (X *)po_heap->ReallocAligned( po_heap->id, addr, size, alignment );
    }

    static bln ReallocAlignedInplaceIfPossible( void *addr, uiw size, uiw alignment, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return po_heap->ReallocAlignedInplaceIfPossible( po_heap->id, addr, size, alignment );
    }

    static bln Validate( heapid heap = (uiw *)HeapManager::Acquire( 0 ) )
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return po_heap->Validate( po_heap->id );
    }

    static uiw BlockSizeGet( const void *addr, heapid heap = (uiw *)HeapManager::Acquire( 0 ) )  //  use only for debug purposes, most heaps will return 0 or bogus value
    {
        HeapBase::SHeap *po_heap = (HeapBase::SHeap *)heap;
        return po_heap->BlockSizeGet( po_heap->id, addr );
    }
}

#endif __HEAP_HPP__