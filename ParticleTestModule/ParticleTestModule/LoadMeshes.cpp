#include "PreHeader.hpp"
#include "Header.hpp"

void ParticleTest::LoadMeshes()
{
	auto addMesh = [this]( UniquePtr < SParticle > vertices, ui32 count, f32 thickness, std::initializer_list < const char * > effects )
	{	
		ASSUME( count % 64 == 0 );

		D3D11_BOX box = { _meshVerticesCount * sizeof(SParticle), 0, 0, (_meshVerticesCount + count) * sizeof(SParticle), 1, 1 };
		RendererGlobals::i_ImContext->UpdateSubresource( _meshesVB, 0, &box, vertices.Get(), 0, 0 );

		_psystems.EmplaceBack();
		_psystems.Back().type = PSystemType::Mesh;
		_psystems.Back().count = count;
		_psystems.Back().start = _meshVerticesCount;
		_psystems.Back().thickness = thickness;

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

		_meshVerticesCount += count;
	};

	#define basicVertexCount 192

	for( ui32 meshIndex = 0; meshIndex < 100; ++meshIndex )
	{
		//f32 r = 1;
		//f32 g = 1;
		//f32 b = 1;
		f32 r = Funcs::RandomFluctuateF32( 0.25, 0.1 );
		f32 g = Funcs::RandomFluctuateF32( 0.4, 0.1 );
		f32 b = 1.0f;
		//f32 intensity = Funcs::RandomRangeF32( 0.1, 0.5 );
		f32 intensity = Funcs::RandomRangeF32( 0.25, 1 );
		r *= intensity;
		g *= intensity;
		b *= intensity;

		auto whirlGlobalsData = EffectsMap[ "whirl_global" ]->CreateData < WhirlEffect::WhirlData >();
		auto whirlLocalData = EffectsMap[ "whirl_local" ]->CreateData < WhirlEffect::WhirlData >();
		auto vectorFieldTestData = EffectsMap[ "vector_field_test" ]->CreateData < VectorFieldEffect::VectorFieldData >();
		auto windTestData = EffectsMap[ "wind_test" ]->CreateData < WindEffect::WindData >();

		vec3 curRot = vec3( 0, Funcs::RandomRangeF32( -f32_pi, f32_pi ), Funcs::RandomRangeF32( -0.05, 0.025f ) );

		auto rotFunc = []( vec3 *curRot, ui32 index ) -> vec3
		{		
			//if( index % 2 == 0 )
			{
				LiceMath::Vec3AdditionInplace( curRot, &vec3( 0, f32_pi / basicVertexCount / 1.5, 0 ) );
			}
			/*else
			{
				LiceMath::Vec3AdditionInplace( curRot, &vec3( 0, 0, f32_pi / 512 ) );
			}*/
			
			return *curRot;
		};

		auto rotVecFunc = []() -> vec3
		{
			return vec3( 0, 0.25, 1 );
		};

		whirlGlobalsData->curRot = std::bind( rotFunc, &curRot, std::placeholders::_1 );
		whirlGlobalsData->rotVec = std::bind( rotVecFunc );

		whirlLocalData->curRot = std::bind( rotVecFunc );

		auto strengthFunc = []() -> vec3
		{
			f32 time = NSecTimeToF32( Globals::Time );
			f32 left = fmod( time, 20.f );
			if( left > 12.f )
			{
				f32 val = Funcs::SaturateF32( (left - 12) / 4 );
				f32 x = Funcs::LerpF32( 0.01, 0.005, val );
				f32 y = Funcs::LerpF32( 0.05, 0.01, val );
				f32 z = Funcs::LerpF32( 0.01, 0.005, val );
				return vec3( x, y, z );
			}
			vec3 v = vec3( 0.01, 0.05, 0.01 );
			LiceMath::Vec3ScaleInplace( &v, Funcs::SaturateF32( left / 4 ) );
			return v;
		};

		vectorFieldTestData->strength = strengthFunc;
		vectorFieldTestData->rotationSpeed = vec3( 0.05, 0.1, 0.05 );
		vectorFieldTestData->rotationSpeed = vec3( 0 );

		UniquePtr < SParticle > vertices = new SParticle[ basicVertexCount ];
		vec3 curPos = this->PosGet();

		for( ui32 index = 0; index < basicVertexCount; ++index )
		{
			vertices[ index ].position = this->PosGet();

			/*if( index < basicVertexCount / 32 )
			{
				vertices[ index ].color = f128color( 1, 0.2, 0.25, Funcs::SaturateF32( (f32)index / (basicVertexCount / 32.f) ) );
			}
			else*/
			{
				vertices[ index ].color = f128color( r, g, b, Funcs::SaturateF32( 3 - (f32)basicVertexCount / (index + 1) ) );
			}

			if( index == 0 )
			{
				vertices[ index ].size_or_texcoord = vec2( 0, 1 );
			}
			else if( index == 1 )
			{
				vertices[ index ].size_or_texcoord = vec2( 0, 0 );
			}
			else if( index == basicVertexCount - 2 )
			{
				vertices[ index ].size_or_texcoord = vec2( 1, 1 );
			}
			else if( index == basicVertexCount - 1 )
			{
				vertices[ index ].size_or_texcoord = vec2( 1, 0 );
			}
			else
			{
				if( index % 2 )
				{
					vertices[ index ].size_or_texcoord = vec2( 0.5, 1 );
				}
				else
				{
					vertices[ index ].size_or_texcoord = vec2( 0.5, 0 );
				}
			}
		}
		
		addMesh( vertices.TakeAway(), basicVertexCount, 0.01f, { "whirl_global", /*"whirl_local", "wind_test",*/ "vector_field_test" } );
	}
}