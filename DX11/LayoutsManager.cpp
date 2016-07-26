#include "PreHeader.hpp"
#include "LayoutsManager.hpp"
#include "RendererGlobals.hpp"

auto LayoutsManager::CompileBufferDesc( CCRefVec < VertexBufferFieldDesc > fieldDescs, ui8 inputSlot ) -> BufferDesc_t
{
    if( inputSlot >= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT || fieldDescs.Size() > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
    {
        return BufferDesc_t();
    }

    BufferFieldDesc bufferFieldDescs[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    uiw semanticsCount = _semantics.Size();
    uiw bufferFieldsCount = _bufferFieldDescs.Size();

    for( uiw index = 0; index < fieldDescs.Size(); ++index )
    {
        bufferFieldDescs[ index ].semanticIndex = FindSemanticIndex( fieldDescs[ index ].semantic );
        bufferFieldDescs[ index ].format = fieldDescs[ index ].format;
        bufferFieldDescs[ index ].inputSlot = inputSlot;
        bufferFieldDescs[ index ].instanceStep = fieldDescs[ index ].instanceStep;
        bufferFieldDescs[ index ].offset = fieldDescs[ index ].offset;
    }

    BufferDesc_t retDesc = FindBufferDesc( MakeRefVec( bufferFieldDescs, fieldDescs.Size() ) );
    if( retDesc.IsNull() )
    {
        _semantics.Resize( semanticsCount );  //  removing added, but unused semantics
        _bufferFieldDescs.Resize( bufferFieldsCount );  //  removing added, but unused buffer fields
    }
    return retDesc;
}

auto LayoutsManager::UniteCompiledBufferDescs( CCRefVec < BufferDesc_t > compiledBufferDescs, ui8 startInputSlot ) -> BufferDesc_t
{
    BufferFieldDesc bufferFieldDescs[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    uiw fieldsCount = 0;

    for( BufferDesc_t compiledBufferDesc : compiledBufferDescs )
    {
        if( startInputSlot >= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
        {
            return BufferDesc_t();  //  error, incorrect input slot
        }

        if( compiledBufferDesc.IsNull() )
        {
            return BufferDesc_t();  //  invalid compiled buffer desc
        }

        for( BufferFieldDescIndex_t bufferFieldIndex : _bufferDescs[ compiledBufferDesc.index ].bufferFields )
        {
            if( fieldsCount == D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
            {
                return BufferDesc_t();  //  error, too many fields
            }

            bufferFieldDescs[ fieldsCount ] =  _bufferFieldDescs[ bufferFieldIndex ];
            bufferFieldDescs[ fieldsCount ].inputSlot = startInputSlot;  //  overriding input slot
            ++fieldsCount;
        }

        ++startInputSlot;
    }

    return FindBufferDesc( MakeRefVec( bufferFieldDescs, fieldsCount ) );
}

//  TODO: check for identical semantics
auto LayoutsManager::CompileShaderInputDesc( CCRefVec < CStr > shaderInputs ) -> ShaderInputDesc_t
{
    if( shaderInputs.Size() > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
    {
        return ShaderInputDesc_t();
    }

    SemanticIndex_t semanticIndexes[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];

    for( uiw index = 0; index < shaderInputs.Size(); ++index )
    {
        semanticIndexes[ index ] = FindSemanticIndex( shaderInputs[ index ] );
    }

    return FindShaderInputDesc( MakeRefVec( semanticIndexes, shaderInputs.Size() ) );
}

bln LayoutsManager::Blend( BufferDesc_t compiledBufferDesc, ShaderInputDesc_t compiledShaderInputDesc, const shaderCode_t &shaderCode, ID3D11InputLayout **i_layout )
{
    ASSUME( i_layout );

    if( compiledBufferDesc.IsNull() || compiledShaderInputDesc.IsNull() )
    {
        return false;  //  invalid input
    }

    ShaderInput &shaderInput = _shaderInputs[ compiledShaderInputDesc.index ];
    BufferDesc &bufferDesc = _bufferDescs[ compiledBufferDesc.index ];

    for( Layout &layout : shaderInput.layouts )
    {
        if( layout.bufferDescIndex == compiledBufferDesc.index )
        {
            *i_layout = layout.i_layout;
            return true;
        }
    }

    //  no such buffer and shader have been successfully blended so far

    if( bufferDesc.bufferFields.Size() < shaderInput.inputList.Size() )
    {
        return false;
    }

    BufferFieldDesc bufferFieldDescs[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    for( uiw index = 0; index < bufferDesc.bufferFields.Size(); ++index )
    {
        bufferFieldDescs[ index ] = _bufferFieldDescs[ bufferDesc.bufferFields[ index ] ];
    }

    for( SemanticIndex_t semanticIndex : shaderInput.inputList )
    {
        if( !Algorithm::Find( bufferFieldDescs, bufferFieldDescs + bufferDesc.bufferFields.Size(), semanticIndex, [](const BufferFieldDesc &desc, SemanticIndex_t semantic) { return desc.semanticIndex == semantic; } ) )
        {
            return false;
        }
    }

    //  buffer desc contains all needed semantics

    ID3D11InputLayout *i_newLayout = CreateLayout( bufferDesc, shaderCode );
    if( i_newLayout == nullptr )
    {
        return false;
    }

    shaderInput.layouts.Append( std::move( Layout { compiledBufferDesc.index, i_newLayout } ) );
    *i_layout = i_newLayout;

    return true;
}

auto LayoutsManager::FindSemanticIndex( const CStr &semantic ) -> SemanticIndex_t
{
    SemanticIndex_t index = 0;

    for( ; index < _semantics.Size(); ++index )
    {
        if( semantic == _semantics[ index ] )
        {
            return index;
        }
    }

    _semantics.Append( semantic );

    return _semantics.LastIndex();
}

auto LayoutsManager::FindShaderInputDesc( CCRefVec < SemanticIndex_t > semanticIndexes ) -> ShaderInputDesc_t
{
    for( const ShaderInput &shaderInput : _shaderInputs )
    {
        if( Algorithm::Equals( semanticIndexes.begin(), semanticIndexes.end(), shaderInput.inputList.begin(), shaderInput.inputList.end() ) )
        {
            return &shaderInput - &_shaderInputs[ 0 ];
        }
    }

    _shaderInputs.Append( ShaderInput( semanticIndexes ) );

    return _shaderInputs.LastIndex();
}

//  TODO: check for duplicate semantics
auto LayoutsManager::FindBufferDesc( CCRefVec < BufferFieldDesc > fieldDescs ) -> BufferDesc_t
{
    if( fieldDescs.Size() == 0 )
    {
        return BufferDesc_t();
    }
     
    BufferFieldDescIndex_t fieldDescsIndexes[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    //  convert field descs to indexes
    for( uiw index = 0; index < fieldDescs.Size(); ++index )
    {
        fieldDescsIndexes[ index ] = FindBufferFieldDescIndex( fieldDescs[ index ] );
    }

    const BufferFieldDesc *storedBufferFieldDescs = _bufferFieldDescs.Data();
    auto sortingLambda = [storedBufferFieldDescs]( BufferFieldDescIndex_t left, BufferFieldDescIndex_t right ) 
    { 
        const BufferFieldDesc &leftDesc = storedBufferFieldDescs[ left ];
        const BufferFieldDesc &rightDesc = storedBufferFieldDescs[ right ];
        return leftDesc.semanticIndex < rightDesc.semanticIndex; 
    };
    Algorithm::Sort( fieldDescsIndexes, fieldDescsIndexes + fieldDescs.Size(), sortingLambda );

    for( const auto &storedFieldDescs : _bufferDescs )
    {
        if( Algorithm::Equals( storedFieldDescs.bufferFields.begin(), storedFieldDescs.bufferFields.end(), fieldDescsIndexes, fieldDescsIndexes + fieldDescs.Size() ) )
        {
            return &storedFieldDescs - &_bufferDescs[ 0 ];  //  returning existing buffer desc index
        }
    }

    _bufferDescs.Append( { { 0, fieldDescsIndexes, fieldDescs.Size() } } );

    return _bufferDescs.LastIndex();
}

auto LayoutsManager::FindBufferFieldDescIndex( const BufferFieldDesc &bufferFieldDesc ) -> BufferFieldDescIndex_t
{
    for( const BufferFieldDesc &storedBufferFieldDesc : _bufferFieldDescs )
    {
        if( _MemEquals( &storedBufferFieldDesc, &bufferFieldDesc, sizeof(BufferFieldDesc) ) )
        {
            return &storedBufferFieldDesc - &_bufferFieldDescs[ 0 ];
        }
    }

    //  there's no such field desc, adding a new one
    _bufferFieldDescs.Append( bufferFieldDesc );
    return _bufferFieldDescs.LastIndex();
}

ID3D11InputLayout *LayoutsManager::CreateLayout( const BufferDesc &bufferDesc, const shaderCode_t &shaderCode )
{
    ASSUME( bufferDesc.bufferFields.Size() <= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );

    D3D11_INPUT_ELEMENT_DESC dxInputDesc[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    CStr semanticStringParts[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    
    for( uiw index = 0; index < bufferDesc.bufferFields.Size(); ++index )
    {
        const BufferFieldDesc &fieldDesc = _bufferFieldDescs[ bufferDesc.bufferFields[ index ] ];

        dxInputDesc[ index ].AlignedByteOffset = fieldDesc.offset;
        dxInputDesc[ index ].Format = fieldDesc.format;
        dxInputDesc[ index ].InputSlot = fieldDesc.inputSlot;
        dxInputDesc[ index ].InputSlotClass = fieldDesc.instanceStep ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
        dxInputDesc[ index ].InstanceDataStepRate = fieldDesc.instanceStep;

        const CStr &semantic = _semantics[ fieldDesc.semanticIndex ];

        CStr semanticNumberPart;
        for( uiw stringIndex = semantic.Size() - 1; stringIndex != uiw_max; --stringIndex )
        {
            if( !Funcs::IsChrDec( semantic[ stringIndex ] ) )
            {
                break;
            }
            semanticNumberPart += semantic[ stringIndex ];
        }
        Algorithm::Reverse( semanticNumberPart.begin(), semanticNumberPart.end() );
        semanticStringParts[ index ].Assign( semantic.CStr(), semantic.Size() - semanticNumberPart.Size() );

        dxInputDesc[ index ].SemanticName = semanticStringParts[ index ].CStr();
        dxInputDesc[ index ].SemanticIndex = Funcs::StrDecToUI32( semanticNumberPart.CStr() );
    }

    ID3D11InputLayout *i_newLayout;
    HRESULT hr = RendererGlobals::i_Device->CreateInputLayout( dxInputDesc, bufferDesc.bufferFields.Size(), shaderCode.Data(), shaderCode.Size(), &i_newLayout );
    if( hr != S_OK )
    {
        return nullptr;
    }

    return i_newLayout;
}