#ifndef __ID_MANAGER_HPP__
#define __ID_MANAGER_HPP__

#include <PackedIntArray.hpp>

typedef struct _id_t 
{ 
    ui32 id; 
    bln operator ==( const _id_t &other ) 
    { 
        return other.id == id; 
    }
    bln operator !=( const _id_t &other ) 
    { 
        return other.id != id; 
    }
} id_t;
const id_t id_t_null = { ui32_max };

class CIDManagerRandom
{
public:
    ~CIDManagerRandom();
    CIDManagerRandom( uiw treeDepth = 16, uiw lastBenchDivide = 0 );
    bln ReserveID( id_t id );
    bln IsIDReserved( id_t id ) const;
    void FreeID( id_t id );
    id_t FindFirstFreeID() const;
};

class CIDManagerSequential
{
    packiarr_dynamic < 1, false > _ids;
    id_t _firstFreeId;
    id_t _lastKnownFirstFreeId;

public:
    ~CIDManagerSequential();
    CIDManagerSequential();
    bln ReserveID( id_t id );
    bln IsIDReserved( id_t id ) const;
    void FreeID( id_t id );
    id_t FindFirstFreeID();
};

class CIDManagerAuto
{
public:
    ~CIDManagerAuto();
    CIDManagerAuto();
    CIDManagerAuto( CNoInit );
    void Construct();
    id_t GenerateID();
    void FreeID( id_t id );
};

#endif __ID_MANAGER_HPP__