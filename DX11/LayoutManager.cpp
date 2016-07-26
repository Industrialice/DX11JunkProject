#include "PreHeader.hpp"
#include "LayoutManager.hpp"
#include "Globals.hpp"
#include "RendererGlobals.hpp"

#ifdef DEBUG
    #define LAYOUT_MANAGER_ASSERT_LEVEL_CHECKS
#endif

typedef ui16 shaderSemanticDesctiptionIndex_t;
const shaderSemanticDesctiptionIndex_t shaderSemanticDesctiptionIndex_t_null = TypeDesc < shaderSemanticDesctiptionIndex_t >::max;

typedef ui16 descElementIndex_t;
const descElementIndex_t descElementIndex_t_null = TypeDesc < descElementIndex_t >::max;

namespace
{
    CVec < LayoutManager::SemanticDesc, void > ShaderSemanticDescriptions;

    struct SDescElement
    {
        ui16 checksum;
        shaderSemanticDesctiptionIndex_t compatibleShaderSemanticDescription;
        struct
        {
            ui8 InputSlot;
            ui8 AlignedByteOffset;
            ui8 InstanceDataStepRate;
            ui8 InputSlotClass;
            ui8 Format;
        } o_descData;
    };
    CVec < SDescElement, void > DescElements;

    struct SDescArray
    {
        //ui32 checksum;  //  will be used to compare desc arrays between each other
        CVec < descElementIndex_t, void > descElements;
    };
    CVec < SDescArray, void > DescArrays;

    struct CompiledShaderSemanticDescription
    {
        CVec < shaderSemanticDesctiptionIndex_t, void > ShaderSemanticDescriptionIndexes;
        struct SCombination
        {
            COMUniquePtr< ID3D11InputLayout > i_layout;
            ui32 checksum;  //  will be used to find out exact combination
            CVec< descElementIndex_t, void > elements;
        };
        CVec < SCombination, void > combinations;
    };
    CVec < CompiledShaderSemanticDescription, void > CompiledShaderSemanticDescriptions;
}

static bln IsConvertExternalDescToInternal( const D3D11_INPUT_ELEMENT_DESC *cpo_desc, ui32 descCount, SDescElement *output );
static void ConvertInternalDescToExternal( const descElementIndex_t *elementIndices, ui32 descCount, D3D11_INPUT_ELEMENT_DESC *output );
static descElementIndex_t FindElement( const SDescElement *element );
static descElementIndex_t AddNewElement( SDescElement *element, const char *semanticName, ui32 semanticIndex );
static LayoutManager::ShaderInputDesc_t FindMatchingDesc( const descElementIndex_t *elements, ui32 elementsCount );
static ui32 GenElementsPIChecksum( const descElementIndex_t *array, ui32 count );  //  unused
static ui32 GenElementsPDChecksum( const descElementIndex_t *array, ui32 count );
static shaderSemanticDesctiptionIndex_t CreateShaderSemDesc( const char *semanticName, ui8 semanticIndex );
static shaderSemanticDesctiptionIndex_t FindShaderSemDesc( const char *semanticName, ui8 semanticIndex );
static LayoutManager::compiledSemDesc_t FindMatchingShaderSemanticDesc( const shaderSemanticDesctiptionIndex_t *ShaderSemanticDescriptionIndexes, ui32 sdrisCount );

NOINLINE LayoutManager::ShaderInputDesc_t LayoutManager::ShaderInputDescCompile( const D3D11_INPUT_ELEMENT_DESC *cpo_desc, ui32 descCount )
{
    ASSUME( (descCount != 0) == (cpo_desc != 0) );

    #ifdef LAYOUT_MANAGER_ASSERT_LEVEL_CHECKS
        if( descCount > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
        {
            DBGBREAK;
            return ShaderInputDesc_t_null;
        }
    #endif

    if( descCount == 0 )
    {
        return ShaderInputDesc_t_null;
    }

    SDescElement convertedDesc[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];

    if( !IsConvertExternalDescToInternal( cpo_desc, descCount, convertedDesc ) )
    {
        DBGBREAK;
        return ShaderInputDesc_t_null;
    }

    descElementIndex_t elementIndices[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    bln is_needToAllocANewOneArray = false;
    for( ui32 element = 0; element < descCount; ++element )
    {
        descElementIndex_t index = FindElement( &convertedDesc[ element ] );
        if( index == descElementIndex_t_null )
        {
            is_needToAllocANewOneArray = true;
            index = AddNewElement( &convertedDesc[ element ], cpo_desc[ element ].SemanticName, cpo_desc[ element ].SemanticIndex );
        }
        elementIndices[ element ] = index;
    }

    ShaderInputDesc_t description;

    if( !is_needToAllocANewOneArray )
    {
        description = FindMatchingDesc( elementIndices, descCount );
        if( description == ShaderInputDesc_t_null )
        {
            is_needToAllocANewOneArray = true;
        }
    }

    if( is_needToAllocANewOneArray )
    {
        description = DescArrays.Size();
        DescArrays.PushBackNum();
        DescArrays.Back().descElements.Append( elementIndices, descCount );
    }

    return description;
}

NOINLINE LayoutManager::ShaderInputDesc_t LayoutManager::ShaderInputDescAdd( ShaderInputDesc_t description, const D3D11_INPUT_ELEMENT_DESC *cpo_desc, ui32 descCount )
{
    if( description == ShaderInputDesc_t_null )
    {
        return ShaderInputDescCompile( cpo_desc, descCount );
    }
    if( !descCount )
    {
        return description;
    }
    ASSUME( cpo_desc );

    ASSUME( description < DescArrays.Size() );
    SDescArray *da = &DescArrays[ description ];

    #ifdef LAYOUT_MANAGER_ASSERT_LEVEL_CHECKS
        if( da->descElements.Size() + descCount > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
        {
            DBGBREAK;
            return ShaderInputDesc_t_null;
        }
    #endif

    D3D11_INPUT_ELEMENT_DESC externalDesc[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    ConvertInternalDescToExternal( &da->descElements[ 0 ], da->descElements.Size(), externalDesc );
    _MemCpy( externalDesc + da->descElements.Size(), cpo_desc, sizeof(D3D11_INPUT_ELEMENT_DESC) * descCount );

    return ShaderInputDescCompile( externalDesc, da->descElements.Size() + descCount );
}

NOINLINE LayoutManager::ShaderInputDesc_t LayoutManager::ShaderInputDescUnite( ShaderInputDesc_t *descriptions, ui32 descriptionsCount )
{
    ASSUME( (descriptions != 0) == (descriptionsCount != 0) );
    if( !descriptionsCount )
    {
        return ShaderInputDesc_t_null;
    }

    D3D11_INPUT_ELEMENT_DESC externalDesc[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    ui32 offset = 0;

    for( ui32 index = 0; index < descriptionsCount; ++index )
    {
        ASSUME( descriptions[ index ] < DescArrays.Size() );
        SDescArray *da = &DescArrays[ descriptions[ index ] ];

        #ifdef LAYOUT_MANAGER_ASSERT_LEVEL_CHECKS
            if( da->descElements.Size() + offset > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
            {
                DBGBREAK;
                return ShaderInputDesc_t_null;
            }
        #endif

        ConvertInternalDescToExternal( da->descElements.Data(), da->descElements.Size(), externalDesc + offset );

        offset += da->descElements.Size();
    }

    return ShaderInputDescCompile( externalDesc, offset );
}

NOINLINE LayoutManager::compiledSemDesc_t LayoutManager::ShaderSemanticCompile( const SemanticDesc *compiledShaderSemantics, ui32 compiledShaderSemanticsCount )
{
    ASSUME( (compiledShaderSemanticsCount != 0) == (compiledShaderSemantics != 0) );
    if( compiledShaderSemanticsCount == 0 )
    {
        return compiledSemDesc_t_null;
    }

    #ifdef LAYOUT_MANAGER_ASSERT_LEVEL_CHECKS
        if( compiledShaderSemanticsCount > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
        {
            DBGBREAK;
            return compiledSemDesc_t_null;
        }
    #endif

    shaderSemanticDesctiptionIndex_t sdriIndices[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];

    bln is_needToAllocANewOneSdriDescs = false;
    for( ui32 index = 0; index < compiledShaderSemanticsCount; ++index )
    {
        sdriIndices[ index ] = FindShaderSemDesc( compiledShaderSemantics[ index ].semanticName.CStr(), compiledShaderSemantics[ index ].semanticIndex );
        if( sdriIndices[ index ] == shaderSemanticDesctiptionIndex_t_null )
        {
            sdriIndices[ index ] = CreateShaderSemDesc( compiledShaderSemantics[ index ].semanticName.CStr(), compiledShaderSemantics[ index ].semanticIndex );
            is_needToAllocANewOneSdriDescs = true;
        }
    }

    compiledSemDesc_t sdriIndex;

    if( !is_needToAllocANewOneSdriDescs )
    {
        sdriIndex = FindMatchingShaderSemanticDesc( sdriIndices, compiledShaderSemanticsCount );
        if( sdriIndex == compiledSemDesc_t_null )
        {
            is_needToAllocANewOneSdriDescs = true;
        }
    }

    if( is_needToAllocANewOneSdriDescs )
    {
        sdriIndex = CompiledShaderSemanticDescriptions.Size();
        CompiledShaderSemanticDescriptions.PushBackNum();
        CompiledShaderSemanticDescriptions.Back().ShaderSemanticDescriptionIndexes.Append( sdriIndices, compiledShaderSemanticsCount );
    }

    return sdriIndex;
}

NOINLINE bln LayoutManager::TryToBlend( ShaderInputDesc_t *descriptions, ui32 descriptionsCount, compiledSemDesc_t compiledShaderSemantic, const ShaderCode *shaderCode, ID3D11InputLayout **pi_lo )
{
    if( compiledShaderSemantic == compiledSemDesc_t_null )
    {
        *pi_lo = 0;
        return true;
    }

    if( descriptionsCount == 0 )
    {
        return false;
    }

    #ifdef LAYOUT_MANAGER_ASSERT_LEVEL_CHECKS
        if( descriptionsCount > D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT )
        {
            DBGBREAK;
            return false;
        }
    #endif

    ASSUME( descriptions && compiledShaderSemantic < CompiledShaderSemanticDescriptions.Size() && shaderCode );

    CompiledShaderSemanticDescription *sdriDesc = &CompiledShaderSemanticDescriptions[ compiledShaderSemantic ];

    const ui32 sdriCount = sdriDesc->ShaderSemanticDescriptionIndexes.Size();

    descElementIndex_t elementIndices[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];

    for( ui32 sdriIndex = 0; sdriIndex < sdriCount; ++sdriIndex )
    {
        for( ui32 desc = 0; desc < descriptionsCount; ++desc )
        {
            ui32 arrIndex = descriptions[ desc ];
            ASSUME( arrIndex < DescArrays.Size() );
            SDescArray *da = &DescArrays[ arrIndex ];
            for( ui32 element = 0; element < da->descElements.Size(); ++element )
            {
                descElementIndex_t elementIndex = da->descElements[ element ];
                ASSUME( elementIndex < DescElements.Size() );
                SDescElement *delem = &DescElements[ elementIndex ];
                if( sdriDesc->ShaderSemanticDescriptionIndexes[ sdriIndex ] == delem->compatibleShaderSemanticDescription )
                {
                    elementIndices[ sdriIndex ] = elementIndex;
                    goto elementFound;
                }
            }
        }

        return false;

    elementFound:;
    }

    ui32 checksum = GenElementsPDChecksum( elementIndices, sdriCount );

    ui32 combo = 0;
    for( ; combo < sdriDesc->combinations.Size(); ++combo )
    {
        if( sdriDesc->combinations[ combo ].checksum == checksum )
        {
            for( ui32 element = 0; element < sdriCount; ++element )
            {
                if( elementIndices[ element ] != sdriDesc->combinations[ combo ].elements[ element ] )
                {
                    goto nextCombo;
                }
            }
            break;
        }
    nextCombo:;
    }

    if( combo == sdriDesc->combinations.Size() )
    {
        sdriDesc->combinations.PushBackNum();
        sdriDesc->combinations.Back().checksum = checksum;
        sdriDesc->combinations.Back().elements.Assign( elementIndices, sdriCount );
        D3D11_INPUT_ELEMENT_DESC externalDesc[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
        ConvertInternalDescToExternal( elementIndices, sdriCount, externalDesc );
        DXHRCHECK( RendererGlobals::i_Device->CreateInputLayout( externalDesc, sdriCount, shaderCode->code.Data(), shaderCode->code.Size(), sdriDesc->combinations.Back().i_layout.AddrModifiable() ) );
    }

    *pi_lo = sdriDesc->combinations.Back().i_layout;
    return true;
}

bln LayoutManager::IsCanWorkWithoutInput( compiledSemDesc_t compiledShaderSemantic )
{
    return compiledShaderSemantic == compiledSemDesc_t_null;
}

#pragma optimize( "s", on )
ui32 LayoutManager::Private::RamUsage()
{
    ui32 usage = 0;

    usage += sizeof(LayoutManager::SemanticDesc) * ShaderSemanticDescriptions.Size();
    for( ui32 index = 0; index < ShaderSemanticDescriptions.Size(); ++index )
    {
        usage += ShaderSemanticDescriptions[ index ].semanticName.Size();
    }

    usage += sizeof(SDescElement) * DescElements.Size();

    usage += sizeof(SDescArray) * DescArrays.Size();
    for( ui32 index = 0; index < DescArrays.Size(); ++index )
    {
        usage += sizeof(descElementIndex_t) * DescArrays[ index ].descElements.Size();
    }

    usage += sizeof(CompiledShaderSemanticDescription) * CompiledShaderSemanticDescriptions.Size();
    for( ui32 index = 0; index < CompiledShaderSemanticDescriptions.Size(); ++index )
    {
        usage += sizeof(shaderSemanticDesctiptionIndex_t) * CompiledShaderSemanticDescriptions[ index ].ShaderSemanticDescriptionIndexes.Size();
        usage += sizeof(CompiledShaderSemanticDescription::SCombination) * CompiledShaderSemanticDescriptions[ index ].combinations.Size();
        usage += sizeof(descElementIndex_t) * CompiledShaderSemanticDescriptions[ index ].ShaderSemanticDescriptionIndexes.Size() * CompiledShaderSemanticDescriptions[ index ].combinations.Size();
    }

    return usage;
}
#pragma optimize( "", on )

void SetDescElementChecksum( SDescElement *element, const char *semanticName, ui32 semanticIndex )
{
    element->checksum = element->o_descData.AlignedByteOffset & ui16_max;
    element->checksum = (element->checksum + element->o_descData.Format << 2) & ui16_max;
    element->checksum = (element->checksum + element->o_descData.InputSlot << 4) & ui16_max;
    element->checksum = (element->checksum + element->o_descData.InputSlotClass << 6) & ui16_max;
    element->checksum = (element->checksum + element->o_descData.InstanceDataStepRate << 8) & ui16_max;
    element->checksum = (element->checksum + semanticIndex << 10) & ui16_max;
    element->checksum = (element->checksum + Funcs::CheckSum32( (byte *)semanticName, _StrLen( semanticName ) )) & ui16_max;
}

bln IsConvertExternalDescToInternal( const D3D11_INPUT_ELEMENT_DESC *cpo_desc, ui32 descCount, SDescElement *output )
{
    ASSUME( descCount <= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );

    for( ui32 desc = 0; desc < descCount; ++desc )
    {
        #ifdef LAYOUT_MANAGER_ASSERT_LEVEL_CHECKS
            bln is_correct = cpo_desc[ desc ].AlignedByteOffset == D3D11_APPEND_ALIGNED_ELEMENT || cpo_desc[ desc ].AlignedByteOffset <= ui8_max;
            is_correct &= cpo_desc[ desc ].Format <= ui8_max;
            is_correct &= cpo_desc[ desc ].InputSlot <= ui8_max;
            is_correct &= cpo_desc[ desc ].InputSlotClass <= ui8_max;
            is_correct &= cpo_desc[ desc ].InstanceDataStepRate <= ui8_max;
            is_correct &= cpo_desc[ desc ].SemanticIndex <= ui8_max;
            is_correct &= cpo_desc[ desc ].SemanticName != 0;
            if( !is_correct )
            {
                return false;
            }
        #endif

        output[ desc ].o_descData.AlignedByteOffset = cpo_desc[ desc ].AlignedByteOffset == D3D11_APPEND_ALIGNED_ELEMENT ? ui8_max : cpo_desc[ desc ].AlignedByteOffset;
        output[ desc ].o_descData.Format = cpo_desc[ desc ].Format;
        output[ desc ].o_descData.InputSlot = cpo_desc[ desc ].InputSlot;
        output[ desc ].o_descData.InputSlotClass = cpo_desc[ desc ].InputSlotClass;
        output[ desc ].o_descData.InstanceDataStepRate = cpo_desc[ desc ].InstanceDataStepRate;

        SetDescElementChecksum( &output[ desc ], cpo_desc[ desc ].SemanticName, cpo_desc[ desc ].SemanticIndex );
    }
    return true;
}

void ConvertInternalDescToExternal( const descElementIndex_t *elementIndices, ui32 descCount, D3D11_INPUT_ELEMENT_DESC *output )
{
    ASSUME( descCount <= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );

    for( ui32 desc = 0; desc < descCount; ++desc )
    {
        SDescElement *input = &DescElements[ elementIndices[ desc ] ];

        shaderSemanticDesctiptionIndex_t sdriIndex = input->compatibleShaderSemanticDescription;
        ASSUME( sdriIndex < ShaderSemanticDescriptions.Size() );
        LayoutManager::SemanticDesc *compiledShaderSemantic = &ShaderSemanticDescriptions[ sdriIndex ];

        output[ desc ].AlignedByteOffset = input->o_descData.AlignedByteOffset == ui8_max ? D3D11_APPEND_ALIGNED_ELEMENT : input->o_descData.AlignedByteOffset;
        output[ desc ].Format = (DXGI_FORMAT)input->o_descData.Format;
        output[ desc ].InputSlot = input->o_descData.InputSlot;
        output[ desc ].InputSlotClass = (D3D11_INPUT_CLASSIFICATION)input->o_descData.InputSlotClass;
        output[ desc ].InstanceDataStepRate = input->o_descData.InstanceDataStepRate;
        output[ desc ].SemanticIndex = compiledShaderSemantic->semanticIndex;
        output[ desc ].SemanticName = (LPCSTR)compiledShaderSemantic->semanticName.CStr();
    }
}

descElementIndex_t FindElement( const SDescElement *element )
{
    for( ui32 index = 0; index < DescElements.Size(); ++index )
    {
        if( DescElements[ index ].checksum != element->checksum )
        {
            continue;
        }
        //  TODO: wtf
        /*if( element->compatibleShaderSemanticDescription != DescElements[ index ].compatibleShaderSemanticDescription )
        {
            continue;
        }*/
        if( _MemEquals( &element->o_descData, &DescElements[ index ].o_descData, sizeof(element->o_descData) ) )
        {
            return index;
        }
    }
    return descElementIndex_t_null;
}

static shaderSemanticDesctiptionIndex_t FindCompatibleSdri( const char *semanticName, ui32 semanticIndex )
{
    shaderSemanticDesctiptionIndex_t index = FindShaderSemDesc( semanticName, semanticIndex );
    if( index == shaderSemanticDesctiptionIndex_t_null )
    {
        index = CreateShaderSemDesc( semanticName, semanticIndex );
    }
    return index;
}

descElementIndex_t AddNewElement( SDescElement *element, const char *semanticName, ui32 semanticIndex )
{
    descElementIndex_t index = DescElements.Size();

    element->compatibleShaderSemanticDescription = FindCompatibleSdri( semanticName, semanticIndex );

    DescElements.PushBack( *element );

    return index;
}

static bln IsDescContainsElement( descElementIndex_t element, const SDescArray *desc )
{
    for( ui32 index = 0; index < desc->descElements.Size(); ++index )
    {
        if( element == desc->descElements[ index ] )
        {
            return true;
        }
    }
    return false;
}

LayoutManager::ShaderInputDesc_t FindMatchingDesc( const descElementIndex_t *elements, ui32 elementsCount )
{
    for( ui32 desc = 0; desc < DescArrays.Size(); ++desc )
    {
        if( elementsCount == DescArrays[ desc ].descElements.Size() )
        {
            for( ui32 element = 0; element < elementsCount; ++element )
            {
                if( !IsDescContainsElement( elements[ element ], &DescArrays[ desc ] ) )
                {
                    goto continueSearch;
                }
            }
            return desc;
        }
    continueSearch:;
    }
    return LayoutManager::ShaderInputDesc_t_null;
}

ui32 GenElementsPIChecksum( const descElementIndex_t *array, ui32 count )
{
    ui32 sum = 0;
    for( ui32 index = 0; index < count; ++index )
    {
        ASSUME( array[ index ] < DescElements.Size() );
        SDescElement *element = &DescElements[ array[ index ] ];
        sum += element->compatibleShaderSemanticDescription;
        sum += element->checksum;
    }
    return sum;
}

ui32 GenElementsPDChecksum( const descElementIndex_t *array, ui32 count )
{
    ui32 sum = 0;
    for( ui32 index = 0; index < count; ++index )
    {
        ASSUME( array[ index ] < DescElements.Size() );
        SDescElement *element = &DescElements[ array[ index ] ];
        sum += ROTATE16L( element->compatibleShaderSemanticDescription, index );
        sum += ROTATE16L( element->checksum, index + 2 );
    }
    return sum;
}

shaderSemanticDesctiptionIndex_t CreateShaderSemDesc( const char *semanticName, ui8 semanticIndex )
{
    shaderSemanticDesctiptionIndex_t index = ShaderSemanticDescriptions.Size();

    ShaderSemanticDescriptions.EmplaceBack( semanticName, semanticIndex );

    return index;
}

shaderSemanticDesctiptionIndex_t FindShaderSemDesc( const char *semanticName, ui8 semanticIndex )
{
    for( ui32 sdriIndex = 0; sdriIndex < ShaderSemanticDescriptions.Size(); ++sdriIndex )
    {
        if( semanticIndex == ShaderSemanticDescriptions[ sdriIndex ].semanticIndex )
        {
            if( _StrEqual( semanticName, ShaderSemanticDescriptions[ sdriIndex ].semanticName.CStr() ) )
            {
                return sdriIndex;
            }
        }
    }
    return shaderSemanticDesctiptionIndex_t_null;
}

LayoutManager::compiledSemDesc_t FindMatchingShaderSemanticDesc( const shaderSemanticDesctiptionIndex_t *ShaderSemanticDescriptionIndexes, ui32 sdrisCount )
{
    for( ui32 desc = 0; desc < CompiledShaderSemanticDescriptions.Size(); ++desc )
    {
        if( CompiledShaderSemanticDescriptions[ desc ].ShaderSemanticDescriptionIndexes.Size() == sdrisCount )
        {
            for( ui32 compiledShaderSemantic = 0; compiledShaderSemantic < sdrisCount; ++compiledShaderSemantic )
            {
                if( ShaderSemanticDescriptionIndexes[ compiledShaderSemantic ] != CompiledShaderSemanticDescriptions[ desc ].ShaderSemanticDescriptionIndexes[ compiledShaderSemantic ] )
                {
                    goto continueSearch;
                }
            }
            return desc;
        }
    continueSearch:;
    }
    return LayoutManager::compiledSemDesc_t_null;
}