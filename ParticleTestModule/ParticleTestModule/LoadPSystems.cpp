#include "PreHeader.hpp"
#include "Header.hpp"

void ParticleTest::LoadPSystems()
{	
	auto addPSystem = [this]( ui32 count, const vec3 basicPosition, f32 targetSize, f32 sizeFluctuation, const f128color &color, std::initializer_list < const char * > effects )
	{
		ASSUME( count % 64 == 0 );

		UniquePtr < SParticle > p0 = new SParticle[ count ];
		for( ui32 index = 0; index < count; ++index )
		{
			p0[ index ].position = basicPosition;
			p0[ index ].color = color;
			//p0[ index ].size = Funcs::RandomRangeF32( 0.15, 0.05 );
			p0[ index ].size = Funcs::RandomFluctuateF32( targetSize, sizeFluctuation );
		}

		D3D11_BOX box = { _particlesCount * sizeof(SParticle), 0, 0, (_particlesCount + count) * sizeof(SParticle), 1, 1 };
		RendererGlobals::i_ImContext->UpdateSubresource( _particlesVB, 0, &box, p0.Get(), 0, 0 );

		_psystems.EmplaceBack();
		_psystems.Back().count = count;
		_psystems.Back().start = _particlesCount;

		for( uiw index = 0; index < effects.size(); ++index )
		{
			auto creator = EffectsMap.find( CStr( effects.begin()[ index ] ) );
			if( creator == EffectsMap.end() )
			{
				wchar_t str[ 256 ];
				wchar_t convertedName[ 256 ];
				mbstowcs( convertedName, effects.begin()[ index ], 256 );
				swprintf( str, 256, L"эффект с именем %s не найден", convertedName );
				MessageBoxW( 0, str, 0, 0 );
				continue;
			}
			UniformInterface *effect = creator->second->Create( count );
			_psystems.Back().csStack.EmplaceBack( effect, CStr( effects.begin()[ index ] ) );
		}

		_particlesCount += count;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////

	f32 addition = -0.5f;

	for( ui32 index = 0; index < 150; ++index )
	{
		vec3 addPos;
		LiceMath::Vec3Addition( &addPos, &_o_pos, &vec3( 0, addition, 0 ) );

		auto whirlGlobalsData = EffectsMap[ "whirl_global" ]->CreateData < WindEffect::WindData >();
		auto whirlLocalData = EffectsMap[ "whirl_local" ]->CreateData < WindEffect::WindData >();
		auto vectorFieldTestData = EffectsMap[ "vector_field_test" ]->CreateData < WindEffect::WindData >();
		auto windTestData = EffectsMap[ "wind_test" ]->CreateData < WindEffect::WindData >();

		LiceMath::Vec3Addition( &vectorFieldTestData->position, &addPos, &vec3( 0, 0, 0 ) );
		LiceMath::Vec3Addition( &windTestData->position, &addPos, &vec3( 0, 0, 0 ) );

		f128color color;
		if( index % 2 == 0 )
		{
			//color = f128color( Funcs::RandomFluctuateF32( 0.25, 0.1 ), Funcs::RandomFluctuateF32( 0.4, 0.1 ), 1, 1 );
			color = f128color( Funcs::RandomFluctuateF32( 0.25, 0.1 ), 1, Funcs::RandomFluctuateF32( 0.4, 0.1 ), 1 );
		}
		else
		{
			//color = f128color( Funcs::RandomRangeF32( 0.0, 0.1f ), Funcs::RandomRangeF32( 0.0, 0.15 ), Funcs::RandomRangeF32( 0.5, 1 ), 1.0f );
			color = f128color( Funcs::RandomRangeF32( 0.0, 0.1f ), Funcs::RandomRangeF32( 0.5, 1 ), Funcs::RandomRangeF32( 0.0, 0.15 ), 1.0f );
		}

		addPSystem( 256, addPos, 0.01, 0.005, color, { "whirl_global", "whirl_local", /*"wind_test", */"vector_field_test" } );

		addition += 0.0025f;
	}
}