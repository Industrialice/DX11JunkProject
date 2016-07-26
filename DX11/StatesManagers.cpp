#include "PreHeader.hpp"
#include "StatesManagers.hpp"
#include "Globals.hpp"
#include "RendererGlobals.hpp"

#define CREATE_DEFINITIONS( descType, stateType ) \
    struct CONCAT( S, descType ) : CharPOD \
    { \
        descType desc; \
        stateType state; \
        ui32 users; \
        CONCAT( S, descType )( const descType *descIn, stateType stateIn ) \
        { \
            desc = *descIn; \
            state = stateIn; \
            users = 1; \
        } \
    }; \
    CVec < CONCAT( S, descType ), void > CONCAT( descType, s );

#define CREATE_GETSTATE( nameSpace, descType, stateType, CreateStateFunc ) \
    stateType nameSpace::GetState( const descType *descIn ) \
    { \
        for( ui32 index = 0; index < CONCAT( descType, s ).Size(); ++index ) \
        { \
            if( _MemEquals( descIn, &CONCAT( descType, s )[ index ].desc, sizeof(descType) ) ) \
            { \
                ++CONCAT( descType, s )[ index ].users; \
                return CONCAT( descType, s )[ index ].state; \
            } \
        } \
        \
        stateType ss; \
        DXHRCHECK( RendererGlobals::i_Device->CreateStateFunc( descIn, &ss ) ); \
        CONCAT( descType, s ).Append( CONCAT( S, descType )( descIn, ss ) ); \
        \
        return ss; \
    }

#define CREATE_GETDESC( nameSpace, descType, stateType ) \
    descType nameSpace::GetDesc( stateType state ) \
    { \
        for( ui32 index = 0; index < CONCAT( descType, s ).Size(); ++index ) \
        { \
            if( state == CONCAT( descType, s )[ index ].state ) \
            { \
                return CONCAT( descType, s )[ index ].desc; \
            } \
        } \
        \
        HARDBREAK; \
        return descType(); \
    }

#define CREATE_FREESTATE( nameSpace, descType, stateType ) \
    void nameSpace::FreeState( stateType state ) \
    { \
        for( ui32 index = 0; index < CONCAT( descType, s ).Size(); ++index ) \
        { \
            if( state == CONCAT( descType, s )[ index ].state ) \
            { \
                --CONCAT( descType, s )[ index ].users; \
                if( !CONCAT( descType, s )[ index ].users ) \
                { \
                    CONCAT( descType, s )[ index ].state->Release(); \
                    CONCAT( descType, s ).Erase( index, 1 ); \
                } \
                return; \
            } \
        } \
        \
        HARDBREAK; \
    }

#define CREATE_ALL( nameSpace, descType, stateType, CreateStateFunc ) \
    CREATE_GETSTATE( nameSpace, descType, stateType, CreateStateFunc ) \
    CREATE_GETDESC( nameSpace, descType, stateType ) \
    CREATE_FREESTATE( nameSpace, descType, stateType )

namespace
{
    CREATE_DEFINITIONS( D3D11_SAMPLER_DESC, ID3D11SamplerState * );
    CREATE_DEFINITIONS( D3D11_BLEND_DESC, ID3D11BlendState * );
    CREATE_DEFINITIONS( D3D11_RASTERIZER_DESC, ID3D11RasterizerState * );
}

CREATE_ALL( SamplersManager, D3D11_SAMPLER_DESC, ID3D11SamplerState *, CreateSamplerState );

CREATE_ALL( BlendStatesManager, D3D11_BLEND_DESC, ID3D11BlendState *, CreateBlendState );

CREATE_ALL( RasterizerStatesManager, D3D11_RASTERIZER_DESC, ID3D11RasterizerState *, CreateRasterizerState );