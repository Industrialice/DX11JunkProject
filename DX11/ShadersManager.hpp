#ifndef __SHADERS_MANAGER_HPP__
#define __SHADERS_MANAGER_HPP__

#include "LayoutsManager.hpp"

struct ShaderStruct;
typedef struct ShaderStruct * sdrhdl;

namespace ShadersManager
{
    DX11_EXPORT void ApplyShader( sdrhdl shader, bln is_skipPS );
    DX11_EXPORT sdrhdl CurrentShader();
    DX11_EXPORT bln Create( const char *cp_name, const byte *cp_vsCode, uiw vsCodeLen, const byte *cp_gsCode, uiw gsCodeLen, const byte *cp_psCode, uiw psCodeLen, bln is_screenConsts, CCRefVec < CStr > shaderInputsDesc, const D3D11_SO_DECLARATION_ENTRY *cpo_soDesc, uiw soDescLen, const UINT *cp_strides, UINT stridesCount, UINT rasterizedStream ); 
    DX11_EXPORT bln CreateFromFiles( const char *cp_name, const char *cp_vsFilePNN, const char *cp_gsFilePNN, const char *cp_psFilePNN, bln is_screenConsts, CCRefVec < CStr > shaderInputsDesc, const D3D11_SO_DECLARATION_ENTRY *cpo_soDesc, uiw soDescLen, const UINT *cp_strides, UINT stridesCount, UINT rasterizedStream );
    DX11_EXPORT sdrhdl AcquireByName( const char *cp_name );
    DX11_EXPORT bln TryToBlend( LayoutsManager::BufferDesc_t bufferDesc, sdrhdl shader, ID3D11InputLayout **pi_lo );

    namespace Private
    {
        void Initialize( bln is_useCache );
        void EndFrame();
        void BeginFrame();
    }
}

#endif __SHADERS_MANAGER_HPP__