#include "PreHeader.hpp"
#include "TextureLoader.hpp"
#include "Globals.hpp"
#include <Misc.hpp>
#include "RendererGlobals.hpp"

namespace
{
    struct STex
    {
        COMUniquePtr< ID3D11ShaderResourceView > i_view;
        CStr pnn;
        ui32 users;

        STex( ID3D11ShaderResourceView *view, const char *pnn, ui32 users ) : i_view( view ), pnn( pnn ), users( users )
        {}
    };

    CVec < STex > Textures;
}

ID3D11ShaderResourceView *TextureLoader::Load( const char *cp_pnn, ui32 *p_width /* = 0 */, ui32 *p_height /* = 0 */ )
{
    ID3D11ShaderResourceView *i_tex = 0;
    for( ui32 index = 0; index < Textures.Size(); ++index )
    {
        if( Funcs::StrIEqual( Textures[ index ].pnn.CStr(), cp_pnn ) )
        {
            i_tex = Textures[ index ].i_view;
            ++Textures[ index ].users;
        }
    }
    if( !i_tex )
    {
        TimeMoment tc = TimeMoment::CreateCurrent();
        D3DX11_IMAGE_LOAD_INFO o_ili = D3DX11_IMAGE_LOAD_INFO();
        o_ili.Format = DXGI_FORMAT_FROM_FILE;
        DXHRCHECK( ::D3DX11CreateShaderResourceViewFromFileA( RendererGlobals::i_Device, cp_pnn, &o_ili, 0, &i_tex, 0 ) );
        Textures.EmplaceBack( i_tex, cp_pnn, 1 );
        Globals::TotalTextureLoadingTime += TimeMoment::CreateCurrent().SinceSec32( tc );
    }
    if( p_width || p_height )
    {
        ID3D11Resource *i_resource;
        i_tex->GetResource( &i_resource );
        D3D11_TEXTURE2D_DESC desc;
        ID3D11Texture2D *i_texReal = (ID3D11Texture2D *)i_resource;
        i_texReal->GetDesc( &desc );
        DSA( p_width, desc.Width );
        DSA( p_height, desc.Height );
    }
    return i_tex;
}

void TextureLoader::Free( ID3D11ShaderResourceView *i_view )
{
    for( ui32 index = 0; index < Textures.Size(); ++index )
    {
        if( Textures[ index ].i_view == i_view )
        {
            --Textures[ index ].users;
            if( !Textures[ index ].users )
            {
                //Textures.Erase( index, 1 );
            }
            return;
        }
    }
    HARDBREAK;
}