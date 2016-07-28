#include "PreHeader.hpp"
#include "Effects.hpp"

std::unordered_map < CStr, UniquePtr < EffectCreator > > EffectsMap;
	
template < typename DataType, typename EffectType > class CreateThisEffect final : public EffectCreator
{
	DataType (*_CreateDataFunc)();
	DataType _data;
	std::shared_ptr < ID3D11ComputeShader > _shader;
	CStr _name;

public:
	CreateThisEffect( DataType (*CreateData)(), const std::shared_ptr < ID3D11ComputeShader > &shader, const char *name ) : _CreateDataFunc( CreateData ), _shader( shader ), _name( name )
	{}

	virtual CStr GetName() override
	{
		return _name;
	}

	virtual void *_CreateData() override
	{
		_data = _CreateDataFunc();
		_data.effectName = _name;
		return &_data;
	}

public:
	virtual UniformInterface *Create( ui32 particlesCount ) override
	{
		return new EffectType( _shader, _data, particlesCount );
	}
};

template < typename EffectType, typename DataType > void AddEffect( const char *name, DataType (*CreateData)(), const std::shared_ptr < ID3D11ComputeShader > &shader )
{
	auto *creator = new CreateThisEffect < DataType, EffectType >( CreateData, shader, name );

	if( EffectsMap.emplace( CStr( name ), creator ).second == false )
	{
		delete creator;
		wchar_t str[ 256 ];
		wchar_t convertedName[ 256 ];
		mbstowcs( convertedName, name, 256 );
		swprintf( str, 256, L"эффект с именем %s уже существует", convertedName );
		MessageBoxW( 0, str, 0, 0 );
	}
}

COMUniquePtr < ID3D11SamplerState > VectorFieldSamplerState;

COMUniquePtr < ID3D11ShaderResourceView > VectorFieldSRV;

void FilloutEffects()
{
	D3D11_SAMPLER_DESC sampDesc;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.BorderColor[ 0 ] = 1.f;
    sampDesc.BorderColor[ 1 ] = 1.f;
    sampDesc.BorderColor[ 2 ] = 1.f;
    sampDesc.BorderColor[ 3 ] = 1.f;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.MaxLOD = FLT_MAX;
    sampDesc.MinLOD = -FLT_MAX;
    sampDesc.MipLODBias = 0.f;

	VectorFieldSamplerState = SamplersManager::GetState( &sampDesc );

	VectorFieldFileLoader::VectorFieldInfo vectorFieldInfo;

	if( VectorFieldFileLoader::Load( L"VectorFields/vecField.150.ivf", &vectorFieldInfo ) )
	{
		SENDLOG( CLogger::Tag::important, "vector field file was loaded\n" );
		if( HRESULT result = RendererGlobals::i_Device->CreateShaderResourceView( vectorFieldInfo.i_texture, 0, VectorFieldSRV.AddrModifiable() ) != S_OK )
		{
			SENDLOG( CLogger::Tag::error, "failed to create shader resource view for vector field's texture\n" );
		}
	}

	std::shared_ptr < ID3D11ComputeShader > whirlCS;
	if( CompileShader( L"Shaders/whirl_cs.hlsl", &whirlCS ) )
	{
		SENDLOG( CLogger::Tag::important, "compiled whirl cs\n" );
	}

	std::shared_ptr < ID3D11ComputeShader > travelCS;
	if( CompileShader( L"Shaders/travel_cs.hlsl", &travelCS ) )
	{
		SENDLOG( CLogger::Tag::important, "compiled travel cs\n" );
	}
	
	std::shared_ptr < ID3D11ComputeShader > windCS;
	if( CompileShader( L"Shaders/wind_cs.hlsl", &windCS ) )
	{
		SENDLOG( CLogger::Tag::important, "compiled wind cs\n" );
	}
	
	std::shared_ptr < ID3D11ComputeShader > vectorFieldCS;
	if( CompileShader( L"Shaders/vectorField_cs.hlsl", &vectorFieldCS ) )
	{
		SENDLOG( CLogger::Tag::important, "compiled vector field cs\n" );
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	auto whirlGlobal = []
	{
		WhirlEffect::WhirlData data;
		data.rotVec.data = vec3( 0, 0.25, 1 );
		data.rotVec.offsetMin = vec3( -0.02, -0.02, -0.15 );
		LiceMath::Vec3ScaleInplace( &data.rotVec.offsetMin, 0.1f );
		LiceMath::Vec3ScaleInplace( &data.rotVec.offsetMax, 0.1f );
		data.rotVec.offsetMax = vec3( 0.02, 0.02, 0.15 );
		data.rotSpeeds.data = vec3( 0, 0.25, 0 );
		data.rotSpeeds.offsetMin = vec3( 0, 0, 0 );
		data.rotSpeeds.offsetMax = vec3( 0, 0, 0 );
		data.rotSpeedMult = Funcs::RandomRangeF32( 0.5, 1.5 );
		data.curRot.data = vec3( 0, Funcs::RandomRangeF32( 0, f32_pi * 2 ), 0 );
		data.curRot.offsetMin = vec3( 0, Funcs::RandomRangeF32( 0, 0 ), 0 );
		data.curRot.offsetMax = vec3( Funcs::RandomFluctuateF32( 0.05, 0.15 ), Funcs::RandomRangeF32( f32_pi, f32_pi ), 0 );
		LiceMath::Vec3ScaleInplace( &data.curRot.offsetMin, 0.25f );
		LiceMath::Vec3ScaleInplace( &data.curRot.offsetMax, 0.25f );
		return data;
	};
	AddEffect < WhirlEffect, WhirlEffect::WhirlData >( "whirl_global", whirlGlobal, whirlCS );

	auto whirlLocal = []
	{
		WhirlEffect::WhirlData data;
		data.rotVec.data = vec3( 0, 0.05, 0 );
		data.rotVec.offsetMin = vec3( -0.05, -0.05, -0.05 );
		data.rotVec.offsetMax = vec3( 0.05, 0.05, 0.05 );
		data.rotVec.scale = 0.1f;
		data.rotSpeeds.data = vec3( 0, 0, 1 );
		data.rotSpeeds.offsetMin = vec3( -0.25, -0.25, -0.25 );
		data.rotSpeeds.offsetMax = vec3( 0.25, 0.25, 0.25 );
		data.curRot.offsetMin = vec3( 0, 0, -f32_pi );
		data.curRot.offsetMax = vec3( 0, 0, f32_pi );
		data.curRot.scale = 0.1f;
		return data;
	};
	AddEffect < WhirlEffect, WhirlEffect::WhirlData >( "whirl_local", whirlLocal, whirlCS );

	auto windTest = []
	{
		WindEffect::WindData data;
		data.position = vec3( 0, 0, 0 );
		data.direction = vec3( 1, 1, 1 );
		LiceMath::Vec3NormalizeInplace( &data.direction );
		data.minForce = 0.5f;
		data.maxForce = 3.5;
		data.minDelta = 1;
		data.maxDelta = 5;
		data.concentration = 0.7;
		return data;
	};
	AddEffect < WindEffect, WindEffect::WindData >( "wind_test", windTest, windCS );

	auto travelTest = []
	{
		TravelEffect::TravelData data;

		data.direction.data = vec3( -1, 0, 0 );
		data.direction.offsetMax = vec3( 0.025, 0.025, 0.025 );
		data.direction.offsetMin = vec3( 0, 0, 0 );

		data.startPosition.data = vec3( 0, 0, 0 );
		data.startPosition.offsetMax = vec3( 2, 0.25, 0.25 );
		data.startPosition.offsetMin = vec3( 0, 0, 0 );

		return data;
	};
	AddEffect < TravelEffect, TravelEffect::TravelData >( "travel_test", travelTest, travelCS );

	auto vectorFieldTest = []
	{
		VectorFieldEffect::VectorFieldData data;
		data.position = vec3( 0, 0, 0 );
		data.scale = vec3( 4, 2, 4 );
		data.rotation = vec3( Funcs::RandomRangeF32( 0, f32_pi * 2 ), Funcs::RandomRangeF32( 0, f32_pi * 2 ), Funcs::RandomRangeF32( 0, f32_pi * 2 ) );
		data.rotationSpeed = vec3( 0.1, 0.25, 0.01 );
		data.strength = 0.025;
		data.strictness = 0;
		data.srv = VectorFieldSRV;
		data.sampler = VectorFieldSamplerState;
		data.directionAddition = vec3( 0, 0, 0 );
		data.dragTarget = 0.1f;
		data.dragFluctuation = 0.0f;
		data.velocityMults.data = vec3( 1, 1, 1 );
		return data;
	};
	AddEffect < VectorFieldEffect, VectorFieldEffect::VectorFieldData >( "vector_field_test", vectorFieldTest, vectorFieldCS );
}