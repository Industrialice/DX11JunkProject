#pragma once

#ifdef DX11_TEST
	#define DX11_EXPORT __declspec(dllexport)
#else
	#define DX11_EXPORT __declspec(dllimport)
#endif

inline f32 NSecTimeToF32( ui64 time )
{
    return time / (1000.0 * 1000.0 * 1000.0);
}

inline ui64 F32TimeToNSec( f32 time )
{
    return time * (1000.0 * 1000.0 * 1000.0);
}

template < typename X > struct COMDeleter
{
    COMDeleter( X *huh )
    {
        if( huh )
        {
            huh->Release();
        }
    }
};

template < typename X > struct COMUniquePtr : UniquePtr< X, COMDeleter< X > > 
{
    COMUniquePtr() : UniquePtr< X, COMDeleter< X > >()
    {}

    COMUniquePtr( X *ptr ) : UniquePtr< X, COMDeleter< X > >( ptr )
    {}

    COMUniquePtr( COMUniquePtr &&source ) : UniquePtr< X, COMDeleter< X > >( std::move( source ) )
    {}

    COMUniquePtr &operator = ( COMUniquePtr &&source )
    {
        UniquePtr< X, COMDeleter< X > >::operator =( std::move( source ) );
        return *this;
    }
};

#ifdef DEBUG
    #define DXHRCHECK( hr ) DXHRCHECKFunc( hr, #hr, __FILE__, __LINE__ )
    static void NOINLINE DXHRCHECKFunc( HRESULT hr, const char *cp_code, const char *cp_file, ui32 line )
    {
        if( FAILED( hr ) )
        {
            const char *cp_err = "unknown";
            switch( hr )
            {
                case D3D11_ERROR_FILE_NOT_FOUND:
                    cp_err = "D3D11_ERROR_FILE_NOT_FOUND";
                    break;
                case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
                    cp_err = "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
                    break;
                case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
                    cp_err = "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";
                    break;
                case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
                    cp_err = "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";
                    break;
                case DXGI_ERROR_INVALID_CALL:
                    cp_err = "DXGI_ERROR_INVALID_CALL";
                    break;
                case DXGI_ERROR_WAS_STILL_DRAWING:
                    cp_err = "DXGI_ERROR_WAS_STILL_DRAWING";
                    break;
                case E_FAIL:
                    cp_err = "E_FAIL";
                    break;
                case E_INVALIDARG:
                    cp_err = "E_INVALIDARG";
                    break;
                case E_OUTOFMEMORY:
                    cp_err = "E_OUTOFMEMORY";
                    break;
                case S_FALSE:
                    cp_err = "S_FALSE";
                    break;
            }
            char a_buf[ 1024 ];
            Funcs::PrintToStr( a_buf, 1023, "HR failed in file %s on line %u on code %s with error %s", cp_file, line, cp_code, cp_err );
            ::MessageBoxA( 0, a_buf, 0, 0 );
            SOFTBREAK;
            ::FatalAppExitA( 1, 0 );
        }
    }
#else
    #define DXHRCHECK( a ) a
#endif

#define SENDLOG( tag, ... ) Globals::DefaultLogger->Message( (tag), __VA_ARGS__ )

#define OBJECT_DATA_BUF 0
#define MATERIAL_DATA_BUF 1
#define LIGHT_DATA_BUF 12
#define FRAME_DATA_BUF 13

namespace RStates
{
    CONSTS_OPED( rstate_t, nothing = 0,
                           target = BIT( 0 ),
                           glowmap = BIT( 1 ),
                           depthTest = BIT( 2 ),
                           depthWrite = BIT( 3 ),
                           all = 0xF );
}

namespace StdLib {

namespace LiceMath
{
    inline void M4x4InverseTranspose( m4x4 *RSTR pout, const m4x4 *pm )
    {
	    f32 SubFactor00 = pm->m[2][2] * pm->m[3][3] - pm->m[3][2] * pm->m[2][3];
	    f32 SubFactor01 = pm->m[2][1] * pm->m[3][3] - pm->m[3][1] * pm->m[2][3];
	    f32 SubFactor02 = pm->m[2][1] * pm->m[3][2] - pm->m[3][1] * pm->m[2][2];
	    f32 SubFactor03 = pm->m[2][0] * pm->m[3][3] - pm->m[3][0] * pm->m[2][3];
	    f32 SubFactor04 = pm->m[2][0] * pm->m[3][2] - pm->m[3][0] * pm->m[2][2];
	    f32 SubFactor05 = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	    f32 SubFactor06 = pm->m[1][2] * pm->m[3][3] - pm->m[3][2] * pm->m[1][3];
	    f32 SubFactor07 = pm->m[1][1] * pm->m[3][3] - pm->m[3][1] * pm->m[1][3];
	    f32 SubFactor08 = pm->m[1][1] * pm->m[3][2] - pm->m[3][1] * pm->m[1][2];
	    f32 SubFactor09 = pm->m[1][0] * pm->m[3][3] - pm->m[3][0] * pm->m[1][3];
	    f32 SubFactor10 = pm->m[1][0] * pm->m[3][2] - pm->m[3][0] * pm->m[1][2];
	    f32 SubFactor11 = pm->m[1][1] * pm->m[3][3] - pm->m[3][1] * pm->m[1][3];
	    f32 SubFactor12 = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
	    f32 SubFactor13 = pm->m[1][2] * pm->m[2][3] - pm->m[2][2] * pm->m[1][3];
	    f32 SubFactor14 = pm->m[1][1] * pm->m[2][3] - pm->m[2][1] * pm->m[1][3];
	    f32 SubFactor15 = pm->m[1][1] * pm->m[2][2] - pm->m[2][1] * pm->m[1][2];
	    f32 SubFactor16 = pm->m[1][0] * pm->m[2][3] - pm->m[2][0] * pm->m[1][3];
	    f32 SubFactor17 = pm->m[1][0] * pm->m[2][2] - pm->m[2][0] * pm->m[1][2];
	    f32 SubFactor18 = pm->m[1][0] * pm->m[2][1] - pm->m[2][0] * pm->m[1][1];

	    pout->m[0][0] = + (pm->m[1][1] * SubFactor00 - pm->m[1][2] * SubFactor01 + pm->m[1][3] * SubFactor02);
	    pout->m[0][1] = - (pm->m[1][0] * SubFactor00 - pm->m[1][2] * SubFactor03 + pm->m[1][3] * SubFactor04);
	    pout->m[0][2] = + (pm->m[1][0] * SubFactor01 - pm->m[1][1] * SubFactor03 + pm->m[1][3] * SubFactor05);
	    pout->m[0][3] = - (pm->m[1][0] * SubFactor02 - pm->m[1][1] * SubFactor04 + pm->m[1][2] * SubFactor05);

	    pout->m[1][0] = - (pm->m[0][1] * SubFactor00 - pm->m[0][2] * SubFactor01 + pm->m[0][3] * SubFactor02);
	    pout->m[1][1] = + (pm->m[0][0] * SubFactor00 - pm->m[0][2] * SubFactor03 + pm->m[0][3] * SubFactor04);
	    pout->m[1][2] = - (pm->m[0][0] * SubFactor01 - pm->m[0][1] * SubFactor03 + pm->m[0][3] * SubFactor05);
	    pout->m[1][3] = + (pm->m[0][0] * SubFactor02 - pm->m[0][1] * SubFactor04 + pm->m[0][2] * SubFactor05);

	    pout->m[2][0] = + (pm->m[0][1] * SubFactor06 - pm->m[0][2] * SubFactor07 + pm->m[0][3] * SubFactor08);
	    pout->m[2][1] = - (pm->m[0][0] * SubFactor06 - pm->m[0][2] * SubFactor09 + pm->m[0][3] * SubFactor10);
	    pout->m[2][2] = + (pm->m[0][0] * SubFactor11 - pm->m[0][1] * SubFactor09 + pm->m[0][3] * SubFactor12);
	    pout->m[2][3] = - (pm->m[0][0] * SubFactor08 - pm->m[0][1] * SubFactor10 + pm->m[0][2] * SubFactor12);

	    pout->m[3][0] = - (pm->m[0][1] * SubFactor13 - pm->m[0][2] * SubFactor14 + pm->m[0][3] * SubFactor15);
	    pout->m[3][1] = + (pm->m[0][0] * SubFactor13 - pm->m[0][2] * SubFactor16 + pm->m[0][3] * SubFactor17);
	    pout->m[3][2] = - (pm->m[0][0] * SubFactor14 - pm->m[0][1] * SubFactor16 + pm->m[0][3] * SubFactor18);
	    pout->m[3][3] = + (pm->m[0][0] * SubFactor15 - pm->m[0][1] * SubFactor17 + pm->m[0][2] * SubFactor18);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[0][1] + pm->m[0][2] * pout->m[0][2] + pm->m[0][3] * pout->m[0][3];
        f32 revDet = 1.f / det;

	    LiceMath::M4x4MultScalarInplace( pout, revDet );
    }

    inline void M4x4Inverse( m4x4 *RSTR pout, const m4x4 *pm )
    {
	    f32 SubFactor00 = pm->m[2][2] * pm->m[3][3] - pm->m[3][2] * pm->m[2][3];
	    f32 SubFactor01 = pm->m[2][1] * pm->m[3][3] - pm->m[3][1] * pm->m[2][3];
	    f32 SubFactor02 = pm->m[2][1] * pm->m[3][2] - pm->m[3][1] * pm->m[2][2];
	    f32 SubFactor03 = pm->m[2][0] * pm->m[3][3] - pm->m[3][0] * pm->m[2][3];
	    f32 SubFactor04 = pm->m[2][0] * pm->m[3][2] - pm->m[3][0] * pm->m[2][2];
	    f32 SubFactor05 = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	    f32 SubFactor06 = pm->m[1][2] * pm->m[3][3] - pm->m[3][2] * pm->m[1][3];
	    f32 SubFactor07 = pm->m[1][1] * pm->m[3][3] - pm->m[3][1] * pm->m[1][3];
	    f32 SubFactor08 = pm->m[1][1] * pm->m[3][2] - pm->m[3][1] * pm->m[1][2];
	    f32 SubFactor09 = pm->m[1][0] * pm->m[3][3] - pm->m[3][0] * pm->m[1][3];
	    f32 SubFactor10 = pm->m[1][0] * pm->m[3][2] - pm->m[3][0] * pm->m[1][2];
	    f32 SubFactor11 = pm->m[1][1] * pm->m[3][3] - pm->m[3][1] * pm->m[1][3];
	    f32 SubFactor12 = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
	    f32 SubFactor13 = pm->m[1][2] * pm->m[2][3] - pm->m[2][2] * pm->m[1][3];
	    f32 SubFactor14 = pm->m[1][1] * pm->m[2][3] - pm->m[2][1] * pm->m[1][3];
	    f32 SubFactor15 = pm->m[1][1] * pm->m[2][2] - pm->m[2][1] * pm->m[1][2];
	    f32 SubFactor16 = pm->m[1][0] * pm->m[2][3] - pm->m[2][0] * pm->m[1][3];
	    f32 SubFactor17 = pm->m[1][0] * pm->m[2][2] - pm->m[2][0] * pm->m[1][2];
	    f32 SubFactor18 = pm->m[1][0] * pm->m[2][1] - pm->m[2][0] * pm->m[1][1];

	    pout->m[0][0] = + (pm->m[1][1] * SubFactor00 - pm->m[1][2] * SubFactor01 + pm->m[1][3] * SubFactor02);
	    pout->m[1][0] = - (pm->m[1][0] * SubFactor00 - pm->m[1][2] * SubFactor03 + pm->m[1][3] * SubFactor04);
	    pout->m[2][0] = + (pm->m[1][0] * SubFactor01 - pm->m[1][1] * SubFactor03 + pm->m[1][3] * SubFactor05);
	    pout->m[3][0] = - (pm->m[1][0] * SubFactor02 - pm->m[1][1] * SubFactor04 + pm->m[1][2] * SubFactor05);

	    pout->m[0][1] = - (pm->m[0][1] * SubFactor00 - pm->m[0][2] * SubFactor01 + pm->m[0][3] * SubFactor02);
	    pout->m[1][1] = + (pm->m[0][0] * SubFactor00 - pm->m[0][2] * SubFactor03 + pm->m[0][3] * SubFactor04);
	    pout->m[2][1] = - (pm->m[0][0] * SubFactor01 - pm->m[0][1] * SubFactor03 + pm->m[0][3] * SubFactor05);
	    pout->m[3][1] = + (pm->m[0][0] * SubFactor02 - pm->m[0][1] * SubFactor04 + pm->m[0][2] * SubFactor05);

	    pout->m[0][2] = + (pm->m[0][1] * SubFactor06 - pm->m[0][2] * SubFactor07 + pm->m[0][3] * SubFactor08);
	    pout->m[1][2] = - (pm->m[0][0] * SubFactor06 - pm->m[0][2] * SubFactor09 + pm->m[0][3] * SubFactor10);
	    pout->m[2][2] = + (pm->m[0][0] * SubFactor11 - pm->m[0][1] * SubFactor09 + pm->m[0][3] * SubFactor12);
	    pout->m[3][2] = - (pm->m[0][0] * SubFactor08 - pm->m[0][1] * SubFactor10 + pm->m[0][2] * SubFactor12);

	    pout->m[0][3] = - (pm->m[0][1] * SubFactor13 - pm->m[0][2] * SubFactor14 + pm->m[0][3] * SubFactor15);
	    pout->m[1][3] = + (pm->m[0][0] * SubFactor13 - pm->m[0][2] * SubFactor16 + pm->m[0][3] * SubFactor17);
	    pout->m[2][3] = - (pm->m[0][0] * SubFactor14 - pm->m[0][1] * SubFactor16 + pm->m[0][3] * SubFactor18);
	    pout->m[3][3] = + (pm->m[0][0] * SubFactor15 - pm->m[0][1] * SubFactor17 + pm->m[0][2] * SubFactor18);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[1][0] + pm->m[0][2] * pout->m[2][0] + pm->m[0][3] * pout->m[3][0];
        f32 revDet = 1.f / det;

	    LiceMath::M4x4MultScalarInplace( pout, revDet );
    }

    inline void M4x4InverseTranspose4x3( m4x4 *RSTR pout, const m4x3 *pm )
    {
	    f32 SubFactor00 = pm->m[2][2];
	    f32 SubFactor01 = pm->m[2][1];
	    f32 SubFactor02 = pm->m[2][1] * pm->m[3][2] - pm->m[3][1] * pm->m[2][2];
	    f32 SubFactor03 = pm->m[2][0];
	    f32 SubFactor04 = pm->m[2][0] * pm->m[3][2] - pm->m[3][0] * pm->m[2][2];
	    f32 SubFactor05 = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	    f32 SubFactor06 = pm->m[1][2];
	    f32 SubFactor07 = pm->m[1][1];
	    f32 SubFactor08 = pm->m[1][1] * pm->m[3][2] - pm->m[3][1] * pm->m[1][2];
	    f32 SubFactor09 = pm->m[1][0];
	    f32 SubFactor10 = pm->m[1][0] * pm->m[3][2] - pm->m[3][0] * pm->m[1][2];
	    f32 SubFactor11 = pm->m[1][1];
	    f32 SubFactor12 = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
	    f32 SubFactor15 = pm->m[1][1] * pm->m[2][2] - pm->m[2][1] * pm->m[1][2];
	    f32 SubFactor17 = pm->m[1][0] * pm->m[2][2] - pm->m[2][0] * pm->m[1][2];
	    f32 SubFactor18 = pm->m[1][0] * pm->m[2][1] - pm->m[2][0] * pm->m[1][1];

	    pout->m[0][0] = + (pm->m[1][1] * SubFactor00 - pm->m[1][2] * SubFactor01);
	    pout->m[0][1] = - (pm->m[1][0] * SubFactor00 - pm->m[1][2] * SubFactor03);
	    pout->m[0][2] = + (pm->m[1][0] * SubFactor01 - pm->m[1][1] * SubFactor03);
	    pout->m[0][3] = - (pm->m[1][0] * SubFactor02 - pm->m[1][1] * SubFactor04 + pm->m[1][2] * SubFactor05);

	    pout->m[1][0] = - (pm->m[0][1] * SubFactor00 - pm->m[0][2] * SubFactor01);
	    pout->m[1][1] = + (pm->m[0][0] * SubFactor00 - pm->m[0][2] * SubFactor03);
	    pout->m[1][2] = - (pm->m[0][0] * SubFactor01 - pm->m[0][1] * SubFactor03);
	    pout->m[1][3] = + (pm->m[0][0] * SubFactor02 - pm->m[0][1] * SubFactor04 + pm->m[0][2] * SubFactor05);

	    pout->m[2][0] = + (pm->m[0][1] * SubFactor06 - pm->m[0][2] * SubFactor07);
	    pout->m[2][1] = - (pm->m[0][0] * SubFactor06 - pm->m[0][2] * SubFactor09);
	    pout->m[2][2] = + (pm->m[0][0] * SubFactor11 - pm->m[0][1] * SubFactor09);
	    pout->m[2][3] = - (pm->m[0][0] * SubFactor08 - pm->m[0][1] * SubFactor10 + pm->m[0][2] * SubFactor12);

	    pout->m[3][0] = 0.f;
	    pout->m[3][1] = 0.f;
	    pout->m[3][2] = 0.f;
	    pout->m[3][3] = + (pm->m[0][0] * SubFactor15 - pm->m[0][1] * SubFactor17 + pm->m[0][2] * SubFactor18);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[0][1] + pm->m[0][2] * pout->m[0][2];
        f32 revDet = 1.f / det;

	    LiceMath::M4x4MultScalarInplace( pout, revDet );
    }

    inline void M4x4Inverse4x3( m4x4 *RSTR pout, const m4x3 *pm )
    {
	    f32 SubFactor00 = pm->m[2][2];
	    f32 SubFactor01 = pm->m[2][1];
	    f32 SubFactor02 = pm->m[2][1] * pm->m[3][2] - pm->m[3][1] * pm->m[2][2];
	    f32 SubFactor03 = pm->m[2][0];
	    f32 SubFactor04 = pm->m[2][0] * pm->m[3][2] - pm->m[3][0] * pm->m[2][2];
	    f32 SubFactor05 = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	    f32 SubFactor06 = pm->m[1][2];
	    f32 SubFactor07 = pm->m[1][1];
	    f32 SubFactor08 = pm->m[1][1] * pm->m[3][2] - pm->m[3][1] * pm->m[1][2];
	    f32 SubFactor09 = pm->m[1][0];
	    f32 SubFactor10 = pm->m[1][0] * pm->m[3][2] - pm->m[3][0] * pm->m[1][2];
	    f32 SubFactor11 = pm->m[1][1];
	    f32 SubFactor12 = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
	    f32 SubFactor15 = pm->m[1][1] * pm->m[2][2] - pm->m[2][1] * pm->m[1][2];
	    f32 SubFactor17 = pm->m[1][0] * pm->m[2][2] - pm->m[2][0] * pm->m[1][2];
	    f32 SubFactor18 = pm->m[1][0] * pm->m[2][1] - pm->m[2][0] * pm->m[1][1];

	    pout->m[0][0] = + (pm->m[1][1] * SubFactor00 - pm->m[1][2] * SubFactor01);
	    pout->m[1][0] = - (pm->m[1][0] * SubFactor00 - pm->m[1][2] * SubFactor03);
	    pout->m[2][0] = + (pm->m[1][0] * SubFactor01 - pm->m[1][1] * SubFactor03);
	    pout->m[3][0] = - (pm->m[1][0] * SubFactor02 - pm->m[1][1] * SubFactor04 + pm->m[1][2] * SubFactor05);

	    pout->m[0][1] = - (pm->m[0][1] * SubFactor00 - pm->m[0][2] * SubFactor01);
	    pout->m[1][1] = + (pm->m[0][0] * SubFactor00 - pm->m[0][2] * SubFactor03);
	    pout->m[2][1] = - (pm->m[0][0] * SubFactor01 - pm->m[0][1] * SubFactor03);
	    pout->m[3][1] = + (pm->m[0][0] * SubFactor02 - pm->m[0][1] * SubFactor04 + pm->m[0][2] * SubFactor05);

	    pout->m[0][2] = + (pm->m[0][1] * SubFactor06 - pm->m[0][2] * SubFactor07);
	    pout->m[1][2] = - (pm->m[0][0] * SubFactor06 - pm->m[0][2] * SubFactor09);
	    pout->m[2][2] = + (pm->m[0][0] * SubFactor11 - pm->m[0][1] * SubFactor09);
	    pout->m[3][2] = - (pm->m[0][0] * SubFactor08 - pm->m[0][1] * SubFactor10 + pm->m[0][2] * SubFactor12);

	    pout->m[0][3] = 0.f;
	    pout->m[1][3] = 0.f;
	    pout->m[2][3] = 0.f;
	    pout->m[3][3] = + (pm->m[0][0] * SubFactor15 - pm->m[0][1] * SubFactor17 + pm->m[0][2] * SubFactor18);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[1][0] + pm->m[0][2] * pout->m[2][0];
        f32 revDet = 1.f / det;

	    LiceMath::M4x4MultScalarInplace( pout, revDet );
    }

    inline void M4x4InverseTranspose4x3DoNotIdentity( m4x4 *RSTR pout, const m4x3 *pm )
    {
	    f32 SubFactor00 = pm->m[2][2];
	    f32 SubFactor01 = pm->m[2][1];
	    f32 SubFactor02 = pm->m[2][1] * pm->m[3][2] - pm->m[3][1] * pm->m[2][2];
	    f32 SubFactor03 = pm->m[2][0];
	    f32 SubFactor04 = pm->m[2][0] * pm->m[3][2] - pm->m[3][0] * pm->m[2][2];
	    f32 SubFactor05 = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	    f32 SubFactor06 = pm->m[1][2];
	    f32 SubFactor07 = pm->m[1][1];
	    f32 SubFactor08 = pm->m[1][1] * pm->m[3][2] - pm->m[3][1] * pm->m[1][2];
	    f32 SubFactor09 = pm->m[1][0];
	    f32 SubFactor10 = pm->m[1][0] * pm->m[3][2] - pm->m[3][0] * pm->m[1][2];
	    f32 SubFactor11 = pm->m[1][1];
	    f32 SubFactor12 = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
	    f32 SubFactor15 = pm->m[1][1] * pm->m[2][2] - pm->m[2][1] * pm->m[1][2];
	    f32 SubFactor17 = pm->m[1][0] * pm->m[2][2] - pm->m[2][0] * pm->m[1][2];
	    f32 SubFactor18 = pm->m[1][0] * pm->m[2][1] - pm->m[2][0] * pm->m[1][1];

	    pout->m[0][0] = + (pm->m[1][1] * SubFactor00 - pm->m[1][2] * SubFactor01);
	    pout->m[0][1] = - (pm->m[1][0] * SubFactor00 - pm->m[1][2] * SubFactor03);
	    pout->m[0][2] = + (pm->m[1][0] * SubFactor01 - pm->m[1][1] * SubFactor03);
	    pout->m[0][3] = - (pm->m[1][0] * SubFactor02 - pm->m[1][1] * SubFactor04 + pm->m[1][2] * SubFactor05);

	    pout->m[1][0] = - (pm->m[0][1] * SubFactor00 - pm->m[0][2] * SubFactor01);
	    pout->m[1][1] = + (pm->m[0][0] * SubFactor00 - pm->m[0][2] * SubFactor03);
	    pout->m[1][2] = - (pm->m[0][0] * SubFactor01 - pm->m[0][1] * SubFactor03);
	    pout->m[1][3] = + (pm->m[0][0] * SubFactor02 - pm->m[0][1] * SubFactor04 + pm->m[0][2] * SubFactor05);

	    pout->m[2][0] = + (pm->m[0][1] * SubFactor06 - pm->m[0][2] * SubFactor07);
	    pout->m[2][1] = - (pm->m[0][0] * SubFactor06 - pm->m[0][2] * SubFactor09);
	    pout->m[2][2] = + (pm->m[0][0] * SubFactor11 - pm->m[0][1] * SubFactor09);
	    pout->m[2][3] = - (pm->m[0][0] * SubFactor08 - pm->m[0][1] * SubFactor10 + pm->m[0][2] * SubFactor12);

	    /*pout->m[3][0] = 0.f;
	    pout->m[3][1] = 0.f;
	    pout->m[3][2] = 0.f;*/
	    pout->m[3][3] = + (pm->m[0][0] * SubFactor15 - pm->m[0][1] * SubFactor17 + pm->m[0][2] * SubFactor18);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[0][1] + pm->m[0][2] * pout->m[0][2];
        f32 revDet = 1.f / det;

	    LiceMath::M4x4MultScalarInplace( pout, revDet );
    }

    inline void M4x4Inverse4x3DoNotIdentity( m4x4 *RSTR pout, const m4x3 *pm )
    {
	    f32 SubFactor00 = pm->m[2][2];
	    f32 SubFactor01 = pm->m[2][1];
	    f32 SubFactor02 = pm->m[2][1] * pm->m[3][2] - pm->m[3][1] * pm->m[2][2];
	    f32 SubFactor03 = pm->m[2][0];
	    f32 SubFactor04 = pm->m[2][0] * pm->m[3][2] - pm->m[3][0] * pm->m[2][2];
	    f32 SubFactor05 = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	    f32 SubFactor06 = pm->m[1][2];
	    f32 SubFactor07 = pm->m[1][1];
	    f32 SubFactor08 = pm->m[1][1] * pm->m[3][2] - pm->m[3][1] * pm->m[1][2];
	    f32 SubFactor09 = pm->m[1][0];
	    f32 SubFactor10 = pm->m[1][0] * pm->m[3][2] - pm->m[3][0] * pm->m[1][2];
	    f32 SubFactor11 = pm->m[1][1];
	    f32 SubFactor12 = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];
	    f32 SubFactor15 = pm->m[1][1] * pm->m[2][2] - pm->m[2][1] * pm->m[1][2];
	    f32 SubFactor17 = pm->m[1][0] * pm->m[2][2] - pm->m[2][0] * pm->m[1][2];
	    f32 SubFactor18 = pm->m[1][0] * pm->m[2][1] - pm->m[2][0] * pm->m[1][1];

	    pout->m[0][0] = + (pm->m[1][1] * SubFactor00 - pm->m[1][2] * SubFactor01);
	    pout->m[1][0] = - (pm->m[1][0] * SubFactor00 - pm->m[1][2] * SubFactor03);
	    pout->m[2][0] = + (pm->m[1][0] * SubFactor01 - pm->m[1][1] * SubFactor03);
	    pout->m[3][0] = - (pm->m[1][0] * SubFactor02 - pm->m[1][1] * SubFactor04 + pm->m[1][2] * SubFactor05);

	    pout->m[0][1] = - (pm->m[0][1] * SubFactor00 - pm->m[0][2] * SubFactor01);
	    pout->m[1][1] = + (pm->m[0][0] * SubFactor00 - pm->m[0][2] * SubFactor03);
	    pout->m[2][1] = - (pm->m[0][0] * SubFactor01 - pm->m[0][1] * SubFactor03);
	    pout->m[3][1] = + (pm->m[0][0] * SubFactor02 - pm->m[0][1] * SubFactor04 + pm->m[0][2] * SubFactor05);

	    pout->m[0][2] = + (pm->m[0][1] * SubFactor06 - pm->m[0][2] * SubFactor07);
	    pout->m[1][2] = - (pm->m[0][0] * SubFactor06 - pm->m[0][2] * SubFactor09);
	    pout->m[2][2] = + (pm->m[0][0] * SubFactor11 - pm->m[0][1] * SubFactor09);
	    pout->m[3][2] = - (pm->m[0][0] * SubFactor08 - pm->m[0][1] * SubFactor10 + pm->m[0][2] * SubFactor12);

	    /*pout->m[0][3] = 0.f;
	    pout->m[1][3] = 0.f;
	    pout->m[2][3] = 0.f;*/
	    pout->m[3][3] = + (pm->m[0][0] * SubFactor15 - pm->m[0][1] * SubFactor17 + pm->m[0][2] * SubFactor18);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[1][0] + pm->m[0][2] * pout->m[2][0];
        f32 revDet = 1.f / det;

	    LiceMath::M4x4MultScalarInplace( pout, revDet );
    }

    inline void M3x4Inverse4x3( m3x4 *RSTR pout, const m4x3 *pm )
    {
	    f32 SubFactor00 = pm->m[2][2];
	    f32 SubFactor01 = pm->m[2][1];
	    f32 SubFactor02 = pm->m[2][1] * pm->m[3][2] - pm->m[3][1] * pm->m[2][2];
	    f32 SubFactor03 = pm->m[2][0];
	    f32 SubFactor04 = pm->m[2][0] * pm->m[3][2] - pm->m[3][0] * pm->m[2][2];
	    f32 SubFactor05 = pm->m[2][0] * pm->m[3][1] - pm->m[3][0] * pm->m[2][1];
	    f32 SubFactor06 = pm->m[1][2];
	    f32 SubFactor07 = pm->m[1][1];
	    f32 SubFactor08 = pm->m[1][1] * pm->m[3][2] - pm->m[3][1] * pm->m[1][2];
	    f32 SubFactor09 = pm->m[1][0];
	    f32 SubFactor10 = pm->m[1][0] * pm->m[3][2] - pm->m[3][0] * pm->m[1][2];
	    f32 SubFactor11 = pm->m[1][1];
	    f32 SubFactor12 = pm->m[1][0] * pm->m[3][1] - pm->m[3][0] * pm->m[1][1];

	    pout->m[0][0] = + (pm->m[1][1] * SubFactor00 - pm->m[1][2] * SubFactor01);
	    pout->m[1][0] = - (pm->m[1][0] * SubFactor00 - pm->m[1][2] * SubFactor03);
	    pout->m[2][0] = + (pm->m[1][0] * SubFactor01 - pm->m[1][1] * SubFactor03);
	    pout->m[3][0] = - (pm->m[1][0] * SubFactor02 - pm->m[1][1] * SubFactor04 + pm->m[1][2] * SubFactor05);

	    pout->m[0][1] = - (pm->m[0][1] * SubFactor00 - pm->m[0][2] * SubFactor01);
	    pout->m[1][1] = + (pm->m[0][0] * SubFactor00 - pm->m[0][2] * SubFactor03);
	    pout->m[2][1] = - (pm->m[0][0] * SubFactor01 - pm->m[0][1] * SubFactor03);
	    pout->m[3][1] = + (pm->m[0][0] * SubFactor02 - pm->m[0][1] * SubFactor04 + pm->m[0][2] * SubFactor05);

	    pout->m[0][2] = + (pm->m[0][1] * SubFactor06 - pm->m[0][2] * SubFactor07);
	    pout->m[1][2] = - (pm->m[0][0] * SubFactor06 - pm->m[0][2] * SubFactor09);
	    pout->m[2][2] = + (pm->m[0][0] * SubFactor11 - pm->m[0][1] * SubFactor09);
	    pout->m[3][2] = - (pm->m[0][0] * SubFactor08 - pm->m[0][1] * SubFactor10 + pm->m[0][2] * SubFactor12);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[1][0] + pm->m[0][2] * pout->m[2][0];
        f32 revDet = 1.f / det;

	    LiceMath::M3x4MultScalarInplace( pout, revDet );
    }

    inline void M3x3Inverse4x3( m3x3 *RSTR pout, const m4x3 *pm )
    {
	    pout->m[0][0] = + (pm->m[1][1] * pm->m[2][2] - pm->m[1][2] * pm->m[2][1]);
	    pout->m[1][0] = - (pm->m[1][0] * pm->m[2][2] - pm->m[1][2] * pm->m[2][0]);
	    pout->m[2][0] = + (pm->m[1][0] * pm->m[2][1] - pm->m[1][1] * pm->m[2][0]);

	    pout->m[0][1] = - (pm->m[0][1] * pm->m[2][2] - pm->m[0][2] * pm->m[2][1]);
	    pout->m[1][1] = + (pm->m[0][0] * pm->m[2][2] - pm->m[0][2] * pm->m[2][0]);
	    pout->m[2][1] = - (pm->m[0][0] * pm->m[2][1] - pm->m[0][1] * pm->m[2][0]);

	    pout->m[0][2] = + (pm->m[0][1] * pm->m[1][2] - pm->m[0][2] * pm->m[1][1]);
	    pout->m[1][2] = - (pm->m[0][0] * pm->m[1][2] - pm->m[0][2] * pm->m[1][0]);
	    pout->m[2][2] = + (pm->m[0][0] * pm->m[1][1] - pm->m[0][1] * pm->m[1][0]);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[1][0] + pm->m[0][2] * pout->m[2][0];
        f32 revDet = 1.f / det;

	    LiceMath::M3x3MultScalarInplace( pout, revDet );
    }

    inline void M3x3InverseTranspose4x3( m3x3 *RSTR pout, const m4x3 *pm )
    {
	    pout->m[0][0] = + (pm->m[1][1] * pm->m[2][2] - pm->m[1][2] * pm->m[2][1]);
	    pout->m[0][1] = - (pm->m[1][0] * pm->m[2][2] - pm->m[1][2] * pm->m[2][0]);
	    pout->m[0][2] = + (pm->m[1][0] * pm->m[2][1] - pm->m[1][1] * pm->m[2][0]);

	    pout->m[1][0] = - (pm->m[0][1] * pm->m[2][2] - pm->m[0][2] * pm->m[2][1]);
	    pout->m[1][1] = + (pm->m[0][0] * pm->m[2][2] - pm->m[0][2] * pm->m[2][0]);
	    pout->m[1][2] = - (pm->m[0][0] * pm->m[2][1] - pm->m[0][1] * pm->m[2][0]);

	    pout->m[2][0] = + (pm->m[0][1] * pm->m[1][2] - pm->m[0][2] * pm->m[1][1]);
	    pout->m[2][1] = - (pm->m[0][0] * pm->m[1][2] - pm->m[0][2] * pm->m[1][0]);
	    pout->m[2][2] = + (pm->m[0][0] * pm->m[1][1] - pm->m[0][1] * pm->m[1][0]);

	    f32 det = + pm->m[0][0] * pout->m[0][0] + pm->m[0][1] * pout->m[0][1] + pm->m[0][2] * pout->m[0][2];
        f32 revDet = 1.f / det;

	    LiceMath::M3x3MultScalarInplace( pout, revDet );
    }
} }

struct VertexBufferFieldDesc
{
    CStr semantic;
    DXGI_FORMAT format;
    ui16 offset;
    ui8 instanceStep;  //  0 means no instancing
};

typedef CVec < byte, void > shaderCode_t;

namespace Color
{
    const f96color Red( 1.f, 0.f, 0.f );
    const f96color White( 1.f, 1.f, 1.f );
    const f96color Black( 0.f, 0.f, 0.f );
    const f96color Blue( 0.f, 0.f, 1.f );
    const f96color Green( 0.f, 1.f, 0.f );
    const f96color Yellow( 1.f, 1.f, 0.f );
    const f96color Magenta( 1.f, 0.f, 1.f );
    const f96color Cyan( 0.f, 1.f, 1.f );
}

namespace Colora
{
    const f128color Red( 1.f, 0.f, 0.f );
    const f128color White( 1.f, 1.f, 1.f );
    const f128color Black( 0.f, 0.f, 0.f );
    const f128color Blue( 0.f, 0.f, 1.f );
    const f128color Green( 0.f, 1.f, 0.f );
    const f128color Yellow( 1.f, 1.f, 0.f );
    const f128color Magenta( 1.f, 0.f, 1.f );
    const f128color Cyan( 0.f, 1.f, 1.f );
}

struct ScreenRect
{
    f32 x0, y0, x1, y1;
    ScreenRect()
    {
    }
    ScreenRect( f32 x0, f32 y0, f32 x1, f32 y1 ) : x0( x0 ), y0( y0 ), x1( x1 ), y1( y1 )
    {
    }
};

struct PixelsRect
{
    i32 x0, y0, x1, y1;
    PixelsRect()
    {
    }
    PixelsRect( i32 x0, i32 y0, i32 x1, i32 y1 ) : x0( x0 ), y0( y0 ), x1( x1 ), y1( y1 )
    {
    }
};