#pragma once

#include <CString.hpp>
#include <CVector.hpp>

class DX11_EXPORT LayoutsManager
{
    CVec < CStr > _semantics;
    typedef ui16 SemanticIndex_t;

    struct BufferFieldDesc
    {
        SemanticIndex_t semanticIndex;
        ui16 offset;
        ui8 inputSlot;
        ui8 instanceStep;  //  0 means no instancing
        DXGI_FORMAT format;
    };
    CVec < BufferFieldDesc > _bufferFieldDescs;
    typedef ui16 BufferFieldDescIndex_t;

    struct BufferDesc
    {
        CVec < BufferFieldDescIndex_t, void > bufferFields;  //  will be sorted by semanticIndex
    };
    CVec < BufferDesc > _bufferDescs;
    typedef ui16 BufferDescIndex_t;

    struct Layout  //  will store a full buffer desc, including unused fields. because of that, there can be same ID3D11InputLayouts, it's a potential optimization spot
    {
        BufferDescIndex_t bufferDescIndex;
        COMUniquePtr < ID3D11InputLayout > i_layout;
    };

    struct ShaderInput
    {
        CVec < SemanticIndex_t, void > inputList;
        CVec < Layout, void > layouts;

        ShaderInput( const CVec < SemanticIndex_t > inputList ) : inputList( inputList )
        {}
        ShaderInput( const ShaderInput & ) = delete;
        ShaderInput &operator = ( const ShaderInput & ) = delete;
        ShaderInput( ShaderInput &&source )
        {
            inputList = std::move( source.inputList );
            layouts = std::move( source.layouts );
        }
    };
    CVec < ShaderInput > _shaderInputs;

	LayoutsManager( const LayoutsManager & ) = delete;
	LayoutsManager &operator = ( const LayoutsManager & ) = delete;

public:
    class BufferDesc_t : CharPOD
    {
        BufferDescIndex_t index;
        friend class LayoutsManager;
    public:
        BufferDesc_t( BufferDescIndex_t index = TypeDesc < decltype(index) >::max ) : index( index ) {}
        bln IsNull() const { return index == TypeDesc < decltype(index) >::max; }
    };
    class ShaderInputDesc_t
    {
        ui16 index = TypeDesc < decltype(index) >::max;
        friend class LayoutsManager;
    public:
        ShaderInputDesc_t( ui16 index = TypeDesc < decltype(index) >::max ) : index( index ) {}
        bln IsNull() const { return index == TypeDesc < decltype(index) >::max; }
    };

	LayoutsManager() {}

    BufferDesc_t CompileBufferDesc( CCRefVec < VertexBufferFieldDesc > fieldDescs, ui8 inputSlot = 0 );  //  input order doesn't matter, check IsNull after the call
    BufferDesc_t UniteCompiledBufferDescs( CCRefVec < BufferDesc_t > compiledBufferDescs, ui8 startInputSlot = 0 );  //  will increase input slot by one for every new buffer desc
    ShaderInputDesc_t CompileShaderInputDesc( CCRefVec < CStr > shaderInputs );  //  input order matters, check IsNull after the call
    bln Blend( BufferDesc_t compiledBufferDesc, ShaderInputDesc_t compiledShaderInputDesc, const shaderCode_t &shaderCode, ID3D11InputLayout **i_layout );

private:
    SemanticIndex_t FindSemanticIndex( const CStr &semantic );
    ShaderInputDesc_t FindShaderInputDesc( CCRefVec < SemanticIndex_t > semanticIndexes );
    BufferDesc_t FindBufferDesc( CCRefVec < BufferFieldDesc > fieldDescs );
    BufferFieldDescIndex_t FindBufferFieldDescIndex( const BufferFieldDesc &bufferFieldDesc );
    ID3D11InputLayout *CreateLayout( const BufferDesc &bufferDesc, const shaderCode_t &shaderCode );
};