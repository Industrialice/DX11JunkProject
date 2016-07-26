#include "PreHeader.hpp"
#include "HeapRef.hpp"

using namespace HeapBase;

UNIQUEPTRRETURN static void *RSTR HeapRefAlloc( void *id, uiw size );
static void HeapRefFree( void *id, void *addr );
static void *HeapRefRealloc( void *id, void *addr, uiw size );
static bln HeapRefReallocInplaceIfPossible( void *id, void *addr, uiw size );
UNIQUEPTRRETURN static void *RSTR HeapRefAllocAligned( void *id, uiw size, uiw alignment );
static void HeapRefFreeAligned( void *id, void *addr );
static void *HeapRefReallocAligned( void *id, void *addr, uiw size, uiw alignment );
static bln HeapRefReallocAlignedInplaceIfPossible( void *id, void *addr, uiw size, uiw alignment );
static bln HeapRefValidate( void *id );
static uiw HeapRefBlockSizeGet( void *id, const void *addr );

void HeapRef::Create( heapid target )
{
    SHeap *po_heap = (SHeap *)target;
    po_heap->Alloc = HeapRefAlloc;
    po_heap->AllocAligned = HeapRefAllocAligned;
    po_heap->Free = HeapRefFree;
    po_heap->FreeAligned = HeapRefFreeAligned;
    po_heap->Realloc = HeapRefRealloc;
    po_heap->ReallocAligned = HeapRefReallocAligned;
    po_heap->ReallocInplaceIfPossible = HeapRefReallocInplaceIfPossible;
    po_heap->ReallocAlignedInplaceIfPossible = HeapRefReallocAlignedInplaceIfPossible;
    po_heap->Validate = HeapRefValidate;
    po_heap->BlockSizeGet = HeapRefBlockSizeGet;
    *(uiw *)&po_heap->id = HeapManager::Private::Add( po_heap );
}

void HeapRef::Destroy( heapid target )
{
    SHeap *po_heap = (SHeap *)target;
    HeapManager::Private::Remove( *(uiw *)&po_heap->id );
}

void *RSTR HeapRefAlloc( void *id, uiw size )
{
    size += size == 0;
    void *addr = malloc( size );
    ASSUME( addr );
    return addr;
}

void HeapRefFree( void *id, void *addr )
{
    free( addr );
}

void *HeapRefRealloc( void *id, void *addr, uiw size )
{
    size += size == 0;
    addr = realloc( addr, size );
    ASSUME( addr );
    return addr;
}

bln HeapRefReallocInplaceIfPossible( void *id, void *addr, uiw size )
{
    return false;
}

void *RSTR HeapRefAllocAligned( void *id, uiw size, uiw alignment )
{
    size += size == 0;
    void *addr = _aligned_malloc( size, alignment );
    ASSUME( addr );
    return addr;
}

void HeapRefFreeAligned( void *id, void *addr )
{
    _aligned_free( addr );
}

void *HeapRefReallocAligned( void *id, void *addr, uiw size, uiw alignment )
{
    size += size == 0;
    addr = _aligned_realloc( addr, size, alignment );
    ASSUME( addr );
    return addr;
}

bln HeapRefReallocAlignedInplaceIfPossible( void *id, void *addr, uiw size, uiw alignment )
{
    return false;
}

bln HeapRefValidate( void *id )
{
    return true;
}

uiw HeapRefBlockSizeGet( void *id, const void *addr )
{
    return 0;
}