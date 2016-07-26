#ifndef __TEXTURE_LOADER_HPP__
#define __TEXTURE_LOADER_HPP__

namespace TextureLoader
{
    DX11_EXPORT ID3D11ShaderResourceView *Load( const char *cp_pnn, ui32 *p_width = 0, ui32 *p_height = 0 );
    DX11_EXPORT void Free( ID3D11ShaderResourceView *i_view );
}

#endif __TEXTURE_LOADER_HPP__