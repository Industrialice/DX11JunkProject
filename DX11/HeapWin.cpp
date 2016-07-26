#include "PreHeader.hpp"
#include "HeapWin.hpp"

using namespace HeapBase;

UNIQUEPTRRETURN static void *RSTR HeapWinAlloc( void *id, uiw size );
static void HeapWinFree( void *id, void *addr );
static void *HeapWinRealloc( void *id, void *addr, uiw size );
static bln HeapWinReallocInplaceIfPossible( void *id, void *addr, uiw size );
UNIQUEPTRRETURN static void *RSTR HeapWinAllocAligned( void *id, uiw size, uiw alignment );
static void HeapWinFreeAligned( void *id, void *addr );
static void *HeapWinReallocAligned( void *id, void *addr, uiw size, uiw alignment );
static bln HeapWinReallocAlignedInplaceIfPossible( void *id, void *addr, uiw size, uiw alignment );
static bln HeapWinValidate( void *id );
static uiw HeapWinBlockSizeGet( void *id, const void *addr );

struct SWinHeapInfo
{
    HANDLE handle;
    ui32 managerIndex;
};

void HeapWin::Create( heapid target, DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize )
{
    ASSUME( target && (dwInitialSize <= dwMaximumSize || dwMaximumSize == 0) );
    SHeap *po_heap = (SHeap *)target;
    SWinHeapInfo *info = (SWinHeapInfo *)::HeapAlloc( ::GetProcessHeap(), 0, sizeof(SWinHeapInfo) );
    ASSUME( info );
    po_heap->Alloc = HeapWinAlloc;
    po_heap->AllocAligned = HeapWinAllocAligned;
    po_heap->Free = HeapWinFree;
    po_heap->FreeAligned = HeapWinFreeAligned;
    po_heap->id = info;
    po_heap->Realloc = HeapWinRealloc;
    po_heap->ReallocAligned = HeapWinReallocAligned;
    po_heap->ReallocInplaceIfPossible = HeapWinReallocInplaceIfPossible;
    po_heap->ReallocAlignedInplaceIfPossible = HeapWinReallocAlignedInplaceIfPossible;
    po_heap->Validate = HeapWinValidate;
    info->managerIndex = HeapManager::Private::Add( po_heap );
    info->handle = ::HeapCreate( flOptions, dwInitialSize, dwMaximumSize );
    ASSUME( info->handle );
}

void HeapWin::Destroy( heapid target )
{
    SHeap *po_heap = (SHeap *)target;
    SWinHeapInfo *info = (SWinHeapInfo *)po_heap->id;
    HeapManager::Private::Remove( info->managerIndex );
    BOOL result = ::HeapDestroy( info->handle );
    ASSUME( result );
    result = ::HeapFree( ::GetProcessHeap(), 0, info );
    ASSUME( result );
}

void *RSTR HeapWinAlloc( void *id, uiw size )
{
    SWinHeapInfo *info = (SWinHeapInfo *)id;
    void *addr = ::HeapAlloc( info->handle, 0, size );
    ASSUME( addr );
    return addr;
}

void HeapWinFree( void *id, void *addr )
{
    SWinHeapInfo *info = (SWinHeapInfo *)id;
    if( addr )
    {
        BOOL result = ::HeapFree( info->handle, 0, addr );
        ASSUME( result );
    }
}

void *HeapWinRealloc( void *id, void *addr, uiw size )
{
    SWinHeapInfo *info = (SWinHeapInfo *)id;
    if( !addr )
    {
        addr = ::HeapAlloc( info->handle, 0, size );
    }
    else
    {
        addr = ::HeapReAlloc( info->handle, 0, addr, size );
    }
    ASSUME( addr );
    return addr;
}

bln HeapWinReallocInplaceIfPossible( void *id, void *addr, uiw size )
{
    return false;
}

void *RSTR HeapWinAllocAligned( void *id, uiw size, uiw alignment )
{
    SWinHeapInfo *info = (SWinHeapInfo *)id;
    alignment = (alignment + DATA_ALIGNMENT - 1) & ~(DATA_ALIGNMENT - 1);
    void *mem = ::HeapAlloc( info->handle, 0, size + sizeof(void *) + alignment - 1 );
    ASSUME( mem );
    uiw umem = (uiw)mem;
    umem += sizeof(void *);
    umem = (umem + alignment - 1) & ~(alignment - 1);
    void *realAddress = (void *)(umem - sizeof(void *));
    _MemCpy( realAddress, &mem, sizeof(void *) );
    return (void *)umem;
}

void HeapWinFreeAligned( void *id, void *addr )
{
    SWinHeapInfo *info = (SWinHeapInfo *)id;
    if( !addr )
    {
        return;
    }
    void *realAddress;
    _MemCpy( &realAddress, (byte *)addr - sizeof(void *), sizeof(void *) );
    BOOL result = ::HeapFree( info->handle, 0, realAddress );
    ASSUME( result );
}

void *HeapWinReallocAligned( void *id, void *addr, uiw size, uiw alignment )
{
    if( !addr )
    {
        return HeapWinAllocAligned( id, size, alignment );
    }
    SWinHeapInfo *info = (SWinHeapInfo *)id;
    alignment = (alignment + DATA_ALIGNMENT - 1) & ~(DATA_ALIGNMENT - 1);
    void *realAddress;
    _MemCpy( &realAddress, (byte *)addr - sizeof(void *), sizeof(void *) );
    void *mem = ::HeapReAlloc( info->handle, 0, realAddress, size + sizeof(void *) + alignment - 1 );
    ASSUME( mem );
    uiw umem = (uiw)mem;
    umem += sizeof(void *);
    umem = (umem + alignment - 1) & ~(alignment - 1);
    realAddress = (void *)(umem - sizeof(void *));
    _MemCpy( realAddress, &mem, sizeof(void *) );
    return (void *)umem;
}

bln HeapWinReallocAlignedInplaceIfPossible( void *id, void *addr, uiw size, uiw alignment )
{
    return false;
}

bln HeapWinValidate( void *id )
{
    SWinHeapInfo *info = (SWinHeapInfo *)id;
    return ::HeapValidate( info->handle, 0, 0 ) != 0;
}

uiw HeapWinBlockSizeGet( void *id, const void *addr )
{
    SWinHeapInfo *info = (SWinHeapInfo *)id;
    SIZE_T result = ::HeapSize( info->handle, 0, addr );
    ASSUME( result != SIZE_MAX - 1 );
    return result;
}