#pragma once

#include "Effects.hpp"

class ParticleTest final : public CObject
{
	struct PSystem
	{
		ui32 start, count;

		struct CS
		{
			UniformInterface *uniforms = nullptr;
			CStr name;

			CS( UniformInterface *uniforms, CStr &&name ) :
				uniforms( uniforms ), name( std::move( name ) )
			{}

			~CS()
			{
				delete uniforms;
			}

			CS( const CS & ) = delete;
			CS &operator = ( const CS & ) = delete;
			CS( CS &&source )
			{
				uniforms = source.uniforms;
				source.uniforms = nullptr;
				name = std::move( source.name );
			}
			CS &operator = ( CS &&source )
			{
				uniforms = source.uniforms;
				source.uniforms = nullptr;
				name = std::move( source.name );
			}
		};

		CVec < CS > csStack;
	};

	ui32 _particlesCount = 0;
	ID3D11BlendState *_bs = nullptr;
	ID3D11RasterizerState *_rs = nullptr;
	ID3D11SamplerState *_ss = nullptr;
	COMUniquePtr < ID3D11Buffer > _particlesVB;
	COMUniquePtr < ID3D11Buffer > _particlesFrameDataVB;
	sdrhdl _drawShader;
	ID3D11ShaderResourceView *_textureView = nullptr;
	COMUniquePtr < ID3D11UnorderedAccessView > _uav;
	COMUniquePtr < ID3D11ShaderResourceView > _srv;
	COMUniquePtr < ID3D11UnorderedAccessView > _frameDataUAV;
	COMUniquePtr < ID3D11ShaderResourceView > _frameDataSRV;
	std::shared_ptr < ID3D11ComputeShader > _eraseFrameDataCS;
	CVec < PSystem > _psystems;
	COMUniquePtr < ID3D11Buffer > _uniBuffer;

public:
    virtual ~ParticleTest() override;
    ParticleTest( const vec3 &o_pos );
    virtual void Update() override;
    virtual void Draw( bln is_stepTwo ) override;

private:
	void LoadPSystems();
};