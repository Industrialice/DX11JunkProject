#ifndef __LAYOUT_MANAGER_HPP__
#define __LAYOUT_MANAGER_HPP__

#include "Globals.hpp"

struct ShaderCode
{
    CVec< byte, void > code;
    DBGCODE( bln is_created; );

    DBGCODE
    (
        ShaderCode()
        {
            is_created = false;
        }
    );

    void Create( byte *source, ui32 sourceLen )
    {
        CHECK( is_created == false );
        code.Assign( source, sourceLen );
        DBGCODE( is_created = true; );
    }

    void Delete()
    {
        CHECK( is_created == true );
        DBGCODE( is_created = false );
    }
};

namespace LayoutManager
{
    struct SemanticDesc
    {
        const CStr semanticName;
        ui32 semanticIndex;

        SemanticDesc( const char *str, ui32 index ) : semanticName( str ), semanticIndex( index )
        {}
    };

    typedef ui32 ShaderInputDesc_t;
    const ShaderInputDesc_t ShaderInputDesc_t_null = ui32_max;
    typedef ui32 compiledSemDesc_t;
    const compiledSemDesc_t compiledSemDesc_t_null = ui32_max;

    ShaderInputDesc_t ShaderInputDescCompile( const D3D11_INPUT_ELEMENT_DESC *cpo_desc, ui32 descCount );
    ShaderInputDesc_t ShaderInputDescAdd( ShaderInputDesc_t description, const D3D11_INPUT_ELEMENT_DESC *cpo_desc, ui32 descCount );
    ShaderInputDesc_t ShaderInputDescUnite( ShaderInputDesc_t *descriptions, ui32 descriptionsCount );
    compiledSemDesc_t ShaderSemanticCompile( const SemanticDesc *compiledShaderSemantics, ui32 compiledShaderSemanticsCount );
    bln TryToBlend( ShaderInputDesc_t *descriptions, ui32 descriptionsCount, compiledSemDesc_t compiledShaderSemantic, const ShaderCode *shaderCode, ID3D11InputLayout **pi_lo );
    bln IsCanWorkWithoutInput( compiledSemDesc_t compiledShaderSemantic );

    namespace Private
    {
        ui32 RamUsage();
    }
}

#endif __LAYOUT_MANAGER_HPP__