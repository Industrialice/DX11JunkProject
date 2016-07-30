#pragma once

CObject *CreateBox();

class UniformInterface
{
	std::shared_ptr < ID3D11ComputeShader > _computeShader;

public:
	virtual ~UniformInterface() {}
	UniformInterface( const std::shared_ptr < ID3D11ComputeShader > &computeShader ) : _computeShader( computeShader )
	{}
	virtual void SetUniformsAndUAVs( ID3D11Buffer *uniBuffer, ui32 start ) = 0;
	virtual void UpdateDebugInfo() = 0;
	virtual void DrawDebugInfo( bln is_stepTwo ) = 0;
	ID3D11ComputeShader *GetComputeShader()
	{
		return _computeShader.get();
	}
};

inline void CreateBuf( ui32 count, uiw sizeOfElement, const void *data, ID3D11UnorderedAccessView **uav )
{
	D3D11_BUFFER_DESC vbufDesc =
	{
		count * sizeOfElement,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_UNORDERED_ACCESS,
		0,
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		sizeOfElement
	};

	COMUniquePtr < ID3D11Buffer > buffer;

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = data;

	DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &vbufDesc, &sd, buffer.AddrModifiable() ) );
	DXHRCHECK( RendererGlobals::i_Device->CreateUnorderedAccessView( buffer, 0, uav ) );
}

struct Vec3WithRand
{
	vec3 data;
	vec3 offsetMin, offsetMax;
	f32 scale;
};

static f32 ExpRand( f32 from, f32 to )
{
	if( to == from )
	{
		return to;
	}
	f32 diff = to - from;
	f32 center = diff * 0.5f;
	f32 r = Funcs::RandomRangeF32( from, to );
	f32 dist = abs( r - center ) / center;
	r = Funcs::RandomFluctuateF32( center, abs( 1.f / exp( dist ) * center ) );
	return r;
}

static vec3 GetDataWithExpRand( const Vec3WithRand &input )
{
	f32 rx = ExpRand( input.offsetMin.x * input.scale, input.offsetMax.x * input.scale );
	f32 ry = ExpRand( input.offsetMin.y * input.scale, input.offsetMax.y * input.scale );
	f32 rz = ExpRand( input.offsetMin.z * input.scale, input.offsetMax.z * input.scale );

	return vec3( 
		input.data.x * input.scale + rx,
		input.data.y * input.scale + ry,
		input.data.z * input.scale + rz );
}

class WhirlEffect final : public UniformInterface
{
	COMUniquePtr < ID3D11UnorderedAccessView > uav;

public:
	struct WhirlData
	{
		std::function < vec3( ui32 ) > rotVec;
		std::function < vec3( ui32 ) > rotSpeeds;
		std::function < vec3( ui32 ) > curRot;
		f32 rotSpeedMult = 1.0f;
		CStr effectName;
	};
	
	WhirlEffect( const std::shared_ptr < ID3D11ComputeShader > &computeShader, const WhirlData &data, ui32 particlesCount ) : UniformInterface( computeShader )
	{
		struct SWhirlData
		{
			vec3 rotVec;
			f32 padding0;
			vec3 rotSpeeds;
			f32 padding1;
			vec3 curRot;
			f32 padding2;
		};

		UniquePtr < SWhirlData > pdata = new SWhirlData[ particlesCount ];

		for( ui32 particle = 0; particle < particlesCount; ++particle )
		{
			pdata[ particle ].rotVec = data.rotVec( particle );
			pdata[ particle ].curRot = data.curRot( particle );
			LiceMath::Vec3Scale( &pdata[ particle ].rotSpeeds, &data.rotSpeeds( particle ), data.rotSpeedMult );
		}

		CreateBuf( particlesCount, sizeof(SWhirlData), pdata, uav.AddrModifiable() );
	}

	virtual void SetUniformsAndUAVs( ID3D11Buffer *uniBuffer, ui32 start ) override
	{
		struct UniView
		{
			ui32 start;
		};

		D3D11_MAPPED_SUBRESOURCE sr;
		DXHRCHECK( RendererGlobals::i_ImContext->Map( uniBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sr ) );

		UniView *view = (UniView *)sr.pData;

		view->start = start;

		RendererGlobals::i_ImContext->Unmap( uniBuffer, 0 );

		RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 2, 1, uav.Addr(), 0 );
	}

	virtual void UpdateDebugInfo() override
	{}

	virtual void DrawDebugInfo( bln is_stepTwo ) override
	{}
};

class TravelEffect final : public UniformInterface
{
	COMUniquePtr < ID3D11UnorderedAccessView > uav;

public:
	struct TravelData
	{
		Vec3WithRand startPosition;
		Vec3WithRand direction;
		CStr effectName;
	};

	TravelEffect( const std::shared_ptr < ID3D11ComputeShader > &computeShader, const TravelData &data, ui32 particlesCount ) : UniformInterface( computeShader )
	{
		struct STravelData
		{
			vec3 position;
			f32 _padding0;
			vec3 direction;
			f32 _padding1;
		};

		UniquePtr < STravelData > pdata = new STravelData[ particlesCount ];

		for( ui32 particle = 0; particle < particlesCount; ++particle )
		{
			pdata[ particle ].position = GetDataWithExpRand( data.startPosition );
			pdata[ particle ].direction = GetDataWithExpRand( data.direction );
		}

		CreateBuf( particlesCount, sizeof(STravelData), pdata, uav.AddrModifiable() );
	}

	virtual void SetUniformsAndUAVs( ID3D11Buffer *uniBuffer, ui32 start ) override
	{
		struct UniView
		{
			ui32 start;
		};

		D3D11_MAPPED_SUBRESOURCE sr;
		DXHRCHECK( RendererGlobals::i_ImContext->Map( uniBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sr ) );

		UniView *view = (UniView *)sr.pData;

		view->start = start;

		RendererGlobals::i_ImContext->Unmap( uniBuffer, 0 );

		RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 2, 1, uav.Addr(), 0 );
	}

	virtual void UpdateDebugInfo() override
	{}

	virtual void DrawDebugInfo( bln is_stepTwo ) override
	{}
};

class WindEffect final : public UniformInterface
{
	f32 newForce = 0;
	f32 curForce = 0;
	TimeMoment nextForceGeneration = TimeMoment::CreateCurrent();

	COMUniquePtr < ID3D11UnorderedAccessView > uav;

public:
	struct WindData
	{
		vec3 position; // позиция ветра
		f32 maxForce, minForce; // минимальная и максимальная силы, значение выбирается рандомно из этого диапазона
		f32 minDelta, maxDelta; // время перегенерации значения силы
		vec3 direction; // направление ветра
		f32 concentration; // концентрация, диаметр конуса
		CStr effectName;
	} wd;

	WindEffect( const std::shared_ptr < ID3D11ComputeShader > &computeShader, const WindData &data, ui32 particlesCount ) : wd( data ), UniformInterface( computeShader )
	{
		struct SWindPerParticleData
		{
			vec3 shiftedPosition;
			f32 weigthRev;
		};

		UniquePtr < SWindPerParticleData > pdata = new SWindPerParticleData[ particlesCount ];

		f32 weightRev = Funcs::RandomRangeF32( 0.15f, 0.75f );
		if( weightRev > 0.65 )
		{
			weightRev += Funcs::RandomF32();
		}

		for( ui32 particle = 0; particle < particlesCount; ++particle )
		{
			pdata[ particle ].shiftedPosition = vec3( 0 );
			//pdata[ particle ].weigthRev = rand() % 2 ? 0.25f : 1.f;
			pdata[ particle ].weigthRev = weightRev;
		}

		CreateBuf( particlesCount, sizeof(SWindPerParticleData), pdata, uav.AddrModifiable() );
	}

	virtual void SetUniformsAndUAVs( ID3D11Buffer *uniBuffer, ui32 start ) override
	{
		struct UniView
		{
			ui32 start;
			vec3 _padding;
			vec3 position;
			f32 concentration;
			vec3 direction;
			f32 force;
		};

		TimeMoment curMoment = TimeMoment::CreateCurrent();
		if( curMoment >= nextForceGeneration )
		{
			f32 delta = Funcs::RandomRangeF32( wd.minDelta, wd.maxDelta );
			newForce = Funcs::RandomRangeF32( wd.minForce, wd.maxForce );
			nextForceGeneration = TimeMoment::CreateShiftedSec( curMoment, delta );
		}

		curForce = Funcs::LerpF32( curForce, newForce, Funcs::Min( Globals::DT, 1.f ) );

		D3D11_MAPPED_SUBRESOURCE sr;
		DXHRCHECK( RendererGlobals::i_ImContext->Map( uniBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sr ) );

		UniView *view = (UniView *)sr.pData;

		view->start = start;
		view->position = wd.position;
		view->direction = wd.direction;
		view->force = curForce;
		view->concentration = wd.concentration;

		RendererGlobals::i_ImContext->Unmap( uniBuffer, 0 );

		RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 2, 1, uav.Addr(), 0 );
	}

	virtual void UpdateDebugInfo() override
	{}

	virtual void DrawDebugInfo( bln is_stepTwo ) override
	{}
};

class VectorFieldEffect final : public UniformInterface
{
	COMUniquePtr < ID3D11UnorderedAccessView > _particlesVBUAV;
	UniquePtr < CObject > _boxVisualisation;
	TimeMoment _directionAdditionNextChange = TimeMoment::CreateCurrent();
	TimeMoment _directionAdditionLastChange = TimeMoment::CreateCurrent();
	vec3 newDirectionAddition = vec3( 0 );
	vec3 oldDirectionAddition = vec3( 0 );

public:
	struct VectorFieldData
	{
		vec3 position; // позиция вектор филда
		vec3 scale; // размер вектор филда, для осей X Y Z независимо
		vec3 rotation; // текущее вращение
		vec3 rotationSpeed; // скорость вращения
		vec3 strength; // множитель для сил вектор филда, чем больше, тем больше влияние сил на частицы, 0 - нет эффекта
		f32 strictness; // строгость следования частиц силам из вектор филда, 0 - нет влияния, 1 - частицы строго следуют в направлении сил
		vec3 directionAddition; // дополнительный вектор для сил в вектор филде, прибавляется к векторам всех сил, может пригодиться для задания рандомности
		f32 dragTarget, dragFluctuation; // сопротивление "воздуха", чем больше значение, тем быстрее торможение частиц
		Vec3WithRand velocityMults = { vec3( 1, 1, 1 ), vec3( 0, 0, 0 ), vec3( 0, 0, 0 ), 1.0f }; // домножение скорости частиц, нужен для рандомности
		
		ID3D11ShaderResourceView *srv;
		ID3D11SamplerState *sampler;
		CStr effectName;
	} vfd;

	VectorFieldEffect( const std::shared_ptr < ID3D11ComputeShader > &computeShader, const VectorFieldData &data, ui32 particlesCount ) : vfd( data ), UniformInterface( computeShader )
	{
		struct SVectorFieldParticleData
		{
			vec3 velocity;
			f32 drag;
			vec3 velocityMults;
			f32 _padding0;
		};

		UniquePtr < SVectorFieldParticleData > pdata = new SVectorFieldParticleData[ particlesCount ];

		for( ui32 particle = 0; particle < particlesCount; ++particle )
		{
			pdata[ particle ].velocity = vec3( 0, 0, 0 ); // текущая скорость частицы
			//pdata[ particle ].drag = Funcs::RandomRangeF32( 0.25f, 2.0f );
			pdata[ particle ].drag = Funcs::RandomFluctuateF32( data.dragTarget, data.dragFluctuation );
			pdata[ particle ].velocityMults = GetDataWithExpRand( data.velocityMults );
		}

		CreateBuf( particlesCount, sizeof(SVectorFieldParticleData), pdata, _particlesVBUAV.AddrModifiable() );

		_boxVisualisation = CreateBox();
	}

	virtual void SetUniformsAndUAVs( ID3D11Buffer *uniBuffer, ui32 start ) override
	{
		struct UniView
		{
			ui32 start;
			vec3 strength;
			m4x4 worldInverse;
			m4x4 world;
			vec3 directionAddition;
			f32 strictness;
		};

		vec3 rotScaled;
		LiceMath::Vec3Scale( &rotScaled, &vfd.rotationSpeed, Globals::DT );
		LiceMath::Vec3AdditionInplace( &vfd.rotation, &rotScaled );

		f32 delta = TimeMoment::CreateCurrent().SinceSec32( _directionAdditionNextChange );
		if( delta >= 0.0f )
		{
			newDirectionAddition = vec3( Funcs::RandomRangeF32( -1, 1 ), Funcs::RandomRangeF32( -1, 1 ), Funcs::RandomRangeF32( -1, 1 ) );
			oldDirectionAddition = vfd.directionAddition;
			_directionAdditionLastChange = TimeMoment::CreateCurrent();
			_directionAdditionNextChange = TimeMoment::CreateShiftedSec( _directionAdditionLastChange, Funcs::RandomRangeF32( 1, 2 ) );
		}

		delta = TimeMoment::CreateCurrent().SinceSec32( _directionAdditionLastChange );
		delta = Funcs::Min( delta * 0.2f, 1.0f );
		vfd.directionAddition = vec3(
			Funcs::LerpF32( oldDirectionAddition.x, newDirectionAddition.x, delta ),
			Funcs::LerpF32( oldDirectionAddition.y, newDirectionAddition.y, delta ),
			Funcs::LerpF32( oldDirectionAddition.z, newDirectionAddition.z, delta ) );

		m4x3 world;
		LiceMath::M4x3Scale3DRotateXYZTranslate3DVec( &world, &vfd.scale, &vfd.rotation, &vfd.position );

		vec3 posAddition;
		LiceMath::Vec3TransformCoordByM4x3( &posAddition, &vec3( -0.5, -0.5, -0.5 ), &world );

		world.e30 = posAddition.x;
		world.e31 = posAddition.y;
		world.e32 = posAddition.z;

		D3D11_MAPPED_SUBRESOURCE sr;
		DXHRCHECK( RendererGlobals::i_ImContext->Map( uniBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sr ) );

		UniView *view = (UniView *)sr.pData;

		view->start = start;
		LiceMath::M4x4InverseTranspose4x3( &view->worldInverse, &world );
		view->strength = vfd.strength;
		view->strictness = vfd.strictness;
		view->world = (m4x4)world;
		//view->directionAddition = vfd.directionAddition;
		view->directionAddition = vec3( 0 );

		RendererGlobals::i_ImContext->Unmap( uniBuffer, 0 );

		RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 2, 1, _particlesVBUAV.Addr(), 0 );

		RendererGlobals::i_ImContext->CSSetShaderResources( 0, 1, &vfd.srv );

		RendererGlobals::i_ImContext->CSSetSamplers( 0, 1, &vfd.sampler );
	}

	virtual void UpdateDebugInfo() override
	{
		_boxVisualisation->PosSet( vfd.position );
		_boxVisualisation->SizeSet( vfd.scale );
		_boxVisualisation->RotRadSet( vfd.rotation );

		_boxVisualisation->Update();
	}

	virtual void DrawDebugInfo( bln is_stepTwo ) override
	{
		if( Globals::is_DrawDebugShapes )
		{
			_boxVisualisation->Draw( is_stepTwo );
		}
	}
};

void FilloutEffects();

class EffectCreator
{
protected:
	virtual void *_CreateData() = 0;

public:
	virtual ~EffectCreator() {}
	virtual UniformInterface *Create( ui32 particlesCount ) = 0;
	virtual CStr GetName() = 0;
	template < typename EffectType > EffectType *CreateData()
	{
		return (EffectType *)_CreateData();
	}
};

extern std::unordered_map < CStr, UniquePtr < EffectCreator > > EffectsMap;

extern std::shared_ptr < ID3D11ComputeShader > EraseFrameDataCS;