#include "PreHeader.hpp"
#include "Header.hpp"

void ParticleTest::LoadPSystems()
{	
	f32 addition = -0.0f;

	for( ui32 index = 0; index < 100; ++index )
	{
		vec3 addPos;
		LiceMath::Vec3Addition( &addPos, &_o_pos, &vec3( 0, addition, 0 ) );

		auto whirlGlobalsData = EffectsMap[ "whirl_global" ]->CreateData < WhirlEffect::WhirlData >();
		auto whirlLocalData = EffectsMap[ "whirl_local" ]->CreateData < WhirlEffect::WhirlData >();
		auto vectorFieldTestData = EffectsMap[ "vector_field_test" ]->CreateData < VectorFieldEffect::VectorFieldData >();
		auto windTestData = EffectsMap[ "wind_test" ]->CreateData < WindEffect::WindData >();

		//whirlGlobalsData->curRot = 

		LiceMath::Vec3Addition( &vectorFieldTestData->position, &addPos, &vec3( 0, 0, 0 ) );
		LiceMath::Vec3Addition( &windTestData->position, &addPos, &vec3( 0, 0, 0 ) );

		f128color color;
		if( index % 2 == 0 )
		{
			color = f128color( Funcs::RandomFluctuateF32( 0.25, 0.1 ), Funcs::RandomFluctuateF32( 0.4, 0.1 ), 1, 1 );
			//color = f128color( Funcs::RandomFluctuateF32( 0.25, 0.1 ), 1, Funcs::RandomFluctuateF32( 0.4, 0.1 ), 1 );
		}
		else
		{
			color = f128color( Funcs::RandomRangeF32( 0.0, 0.1f ), Funcs::RandomRangeF32( 0.0, 0.15 ), Funcs::RandomRangeF32( 0.5, 1 ), 1.0f );
			//color = f128color( Funcs::RandomRangeF32( 0.0, 0.1f ), Funcs::RandomRangeF32( 0.5, 1 ), Funcs::RandomRangeF32( 0.0, 0.15 ), 1.0f );
		}

		AddPSystem( 256, addPos, 0.01, 0.005, color, { "whirl_global", /*"whirl_local", /*"wind_test", */"vector_field_test" } );

		addition += 0.0025f;
	}
}