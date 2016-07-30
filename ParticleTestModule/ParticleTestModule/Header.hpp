#pragma once

#include "Effects.hpp"

class ParticleTest final : public CObject
{
	enum class PSystemType
	{
		RawParticles, Mesh
	};

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
		PSystemType type;
		f32 thickness = 0.0f;
	};

	ui32 _particlesCount = 0;
	ui32 _meshVerticesCount = 0;
	ID3D11BlendState *_bs = nullptr;
	ID3D11RasterizerState *_rs = nullptr;
	ID3D11SamplerState *_ss = nullptr;
	COMUniquePtr < ID3D11Buffer > _particlesVB;
	COMUniquePtr < ID3D11Buffer > _meshesVB;
	COMUniquePtr < ID3D11Buffer > _particlesFrameDataVB;
	sdrhdl _particlesDrawShader;
	sdrhdl _meshesDrawShader;
	ID3D11InputLayout *_meshInputLayout;
	ID3D11ShaderResourceView *_textureView = nullptr;
	ID3D11ShaderResourceView *_circleTextureView = nullptr;
	COMUniquePtr < ID3D11UnorderedAccessView > _particlesVBUAV;
	COMUniquePtr < ID3D11ShaderResourceView > _particlesVBSRV;
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
	void LoadMeshes();
	void DrawAllParticles();
	void DrawAllMeshes();
	void AddPSystem( ui32 count, const vec3 basicPosition, f32 targetSize, f32 sizeFluctuation, const f128color &color, std::initializer_list < const char * > effects );
};