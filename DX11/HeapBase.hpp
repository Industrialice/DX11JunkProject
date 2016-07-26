#ifndef __HEAP_BASE_HPP__
#define __HEAP_BASE_HPP__

namespace HeapBase
{
    typedef void *RSTR (*AllocProto)( void *id, uiw size );
    typedef void (*FreeProto)( void *id, void *addr );
    typedef void *(*ReallocProto)( void *id, void *addr, uiw size );
    typedef bln (*ReallocInplaceIfPossibleProto)( void *id, void *addr, uiw size );
    typedef void *unicid;
    typedef void *RSTR (*AllocAlignedProto)( void *id, uiw size, uiw alignment );
    typedef void (*FreeAlignedProto)( void *id, void *addr );
    typedef void *(*ReallocAlignedProto)( void *id, void *addr, uiw size, uiw alignment );
    typedef bln (*ReallocAlignedInplaceIfPossibleProto)( void *id, void *addr, uiw size, uiw alignment );
    typedef bln (*ValidateProto)( void *id );
    typedef uiw (*BlockSizeGetProto)( void *id, const void *addr );  //  use only for debug purposes, some heaps can return 0 or bogus values

    struct SHeap
    {
        AllocProto Alloc;
        FreeProto Free;
        ReallocProto Realloc;
        ReallocInplaceIfPossibleProto ReallocInplaceIfPossible;
        unicid id;
        AllocAlignedProto AllocAligned;
        FreeAlignedProto FreeAligned;
        ReallocAlignedProto ReallocAligned;
        ReallocAlignedInplaceIfPossibleProto ReallocAlignedInplaceIfPossible;
        ValidateProto Validate;
        BlockSizeGetProto BlockSizeGet;
    };

    typedef uiw heapid[ sizeof(SHeap) / sizeof(uiw) + (sizeof(SHeap) % sizeof(uiw) != 0) ];  //  uiw alignment should be suitable for required types of structs
}

using HeapBase::heapid;

#endif