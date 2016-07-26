#include "PreHeader.hpp"
#include "SplashScreen.hpp"
#include "Globals.hpp"
#include "IDManager.hpp"
#include <Misc.hpp>
#include "PostProcess.hpp"
#include "TextureLoader.hpp"
#include "StatesManagers.hpp"
#include "RendererGlobals.hpp"

static ScreenRect UntouchRect( const ScreenRect &sourceRect, ui32 w, ui32 h )
{
    return sourceRect;
}

static ScreenRect NicoRect( const ScreenRect &sourceRect, ui32 w, ui32 h )
{
    return ScreenRect( sourceRect.x0, sourceRect.y0, sourceRect.x1, sourceRect.y0 - ((sourceRect.x1 - sourceRect.x0) / ((f32)w / (f32)h) * ((f32)Globals::Width / (f32)Globals::Height)) );
}

static ScreenRect VSRect( const ScreenRect &sourceRect, ui32 w, ui32 h )
{
    return RendererGlobals::PixelsRectToScreenRect( PixelsRect( 0, 0, w, h ) );
}

static ScreenRect VSCodeRect( const ScreenRect &sourceRect, ui32 w, ui32 h )
{
    return RendererGlobals::PixelsRectToScreenRect( PixelsRect( 348, 110, w + 348, h + 110 ) );
}

static void BackMotion( f32 dt, f32 onfly, ScreenRect *rect, ui32 w, ui32 h )
{
    rect->x1 += dt / 175;
    rect->y1 -= dt / 175 / ((f32)w / (f32)h) / ((f32)Globals::Width / (f32)Globals::Height);
}

static void NicoMotion( f32 dt, f32 onfly, ScreenRect *rect, ui32 w, ui32 h )
{
    rect->x0 -= dt / 175;
    rect->y0 += dt / 175 / ((f32)w / (f32)h) / ((f32)Globals::Width / (f32)Globals::Height);
}

static void  NormandysBackMotion( f32 dt, f32 onfly, ScreenRect *rect, ui32 w, ui32 h )
{
    rect->x0 -= dt / 175;
    rect->y0 += dt / 175 / ((f32)w / (f32)h) / ((f32)Globals::Width / (f32)Globals::Height);
}

static void NormandyMotion( f32 dt, f32 onfly, ScreenRect *rect, ui32 w, ui32 h )
{
    rect->x0 += dt / 175;
    rect->y0 -= dt / 175 / ((f32)w / (f32)h) / ((f32)Globals::Width / (f32)Globals::Height);
    rect->x1 += dt / 150;
    rect->y1 -= dt / 150 / ((f32)w / (f32)h) / ((f32)Globals::Width / (f32)Globals::Height);
}

static void VSCodeMotion( f32 dt, f32 onfly, ScreenRect *rect, ui32 w, ui32 h )
{
    rect->x0 += dt / 175;
    rect->y0 -= dt / 175 / ((f32)w / (f32)h) / ((f32)Globals::Width / (f32)Globals::Height);
    rect->x1 += dt / 175;
    rect->y1 -= dt / 175 / ((f32)w / (f32)h) / ((f32)Globals::Width / (f32)Globals::Height);
}

namespace
{
    typedef ScreenRect (*RectFunc_t)( const ScreenRect &sourceRect, ui32 w, ui32 h );
    typedef void (*MotionFunc_t)( f32 dt, f32 onfly, ScreenRect *rect, ui32 w, ui32 h );

    struct EffectSpecial
    {
        const char *const texture;
        ScreenRect rect;
        RectFunc_t rectFunc;
        MotionFunc_t motionFunc;
        ID3D11BlendState *i_blend;
    } specials1[] =
    {
        { "Textures\\loading.dds", ScreenRect( -1, 1, 1, -1 ), UntouchRect, BackMotion },
        { "Textures\\art.dds", ScreenRect( -1, 1, 1, -1 ), UntouchRect, BackMotion },
        //{ "Textures\\oct_the_only.dds", ScreenRect( -1, 1, 1, -1 ), UntouchRect, BackMotion },
        { "Textures\\reapers.dds", ScreenRect( -1, 1, 1, -1 ), UntouchRect, NormandysBackMotion },
        //{ "Textures\\vs_empty.dds", ScreenRect(), VSRect, BackMotion }
    }, 
    specials2[] =
    {
        { "Textures\\nico.dds", ScreenRect( -0.15, 0.5, 0.5, 0.5 ), NicoRect, NicoMotion },
        { "Textures\\char_roman.dds", ScreenRect( -0.15, 0.5, 0.5, 0.5 ), NicoRect, NicoMotion },
        //{ "Textures\\vinyl_cello.dds", ScreenRect( 0, 0.8, 1.2, -1.2 ), NicoRect, NicoMotion },
        { "Textures\\normandy.dds", ScreenRect( -1, 0.5, 0.5, 0.5 ), NicoRect, NormandyMotion },
        //{ "Textures\\vs_code.dds", ScreenRect(), VSCodeRect, VSCodeMotion }
    };

    ui32 TextureOrder;

    struct SEffect
    {
        id_t id;
        ui32 w, h;
        EffectSpecial special;
    } ef1 =
    {
        0, 0, 0, specials1[ 0 ]
    }, ef2 =
    {
        0, 0, 0, specials2[ 0 ]
    };

	bln is_Active;
}

void SplashScreen::Step()
{
	if( !is_Active )
	{
		return;
	}

    static f32 delta;
    static TimeMoment tc;
    static bln is_reset = true;
    if( is_reset )
    {
        tc = TimeMoment::CreateCurrent();
        delta = 0;
        is_reset = false;
    }

	TimeMoment currentMoment = TimeMoment::CreateCurrent();
    f32 curDt = currentMoment.SinceSec32( tc );
	tc = currentMoment;
    delta += curDt;
        
    PostProcess::Effects::Textured *effect0 = (PostProcess::Effects::Textured *)PostProcess::LockEffectByID( ef1.id );

    specials1[ TextureOrder ].motionFunc( curDt, delta, &effect0->_rect, ef1.w, ef1.h );

    PostProcess::Effects::Textured *effect1 = (PostProcess::Effects::Textured *)PostProcess::LockEffectByID( ef2.id );

    specials2[ TextureOrder ].motionFunc( curDt, delta, &effect1->_rect, ef2.w, ef2.h );

    if( delta <= 1 )
    {
        effect0->o_mult.r = effect0->o_mult.g = effect0->o_mult.b = delta;
        effect1->o_mult.r = effect1->o_mult.g = effect1->o_mult.b = delta;
    }
    else 
    {
        effect0->o_mult.r = effect0->o_mult.g = effect0->o_mult.b = 1;
        effect1->o_mult.r = effect1->o_mult.g = effect1->o_mult.b = 1;

        if( delta > 7.5 )
        {
            effect0->o_mult.r = effect0->o_mult.g = effect0->o_mult.b = 8.5 - delta;
            effect1->o_mult.r = effect1->o_mult.g = effect1->o_mult.b = 8.5 - delta;
        }
        if( delta > 8.5 )
        {
            ++TextureOrder;
            if( TextureOrder >= COUNTOF( specials1 ) )
            {
                TextureOrder = 0;
            }

            _MemCpy( &ef1.special, &specials1[ TextureOrder ], sizeof(EffectSpecial) );
            _MemCpy( &ef2.special, &specials2[ TextureOrder ], sizeof(EffectSpecial) );

            TextureLoader::Free( effect0->i_tex );
            effect0->i_tex = TextureLoader::Load( ef1.special.texture, &ef1.w, &ef1.h );
            effect0->_rect = ef1.special.rectFunc( ef1.special.rect, ef1.w, ef1.h );
            effect0->o_mult.r = effect0->o_mult.g = effect0->o_mult.b = 0;
            effect0->i_blend = ef1.special.i_blend;
        
            TextureLoader::Free( effect1->i_tex );
            effect1->i_tex = TextureLoader::Load( ef2.special.texture, &ef2.w, &ef2.h );
            effect1->_rect = ef2.special.rectFunc( ef2.special.rect, ef2.w, ef2.h );
            effect1->o_mult.r = effect1->o_mult.g = effect1->o_mult.b = 0;
            effect1->i_blend = ef2.special.i_blend;

            delta = 0;
			tc = TimeMoment::CreateCurrent();
        }
    }

    PostProcess::Unlock();
}
	
bln SplashScreen::IsActiveGet()
{
	return is_Active;
}

void SplashScreen::IsActiveSet( bln is_active )
{
	is_Active = is_active;
	if( !is_Active )
	{
		((PostProcess::Effects::Textured *)PostProcess::LockEffectByID( ef1.id ))->_is_enabled = false;
		((PostProcess::Effects::Textured *)PostProcess::LockEffectByID( ef2.id ))->_is_enabled = false;
	}
	else
	{
		((PostProcess::Effects::Textured *)PostProcess::LockEffectByID( ef1.id ))->_is_enabled = true;
		((PostProcess::Effects::Textured *)PostProcess::LockEffectByID( ef2.id ))->_is_enabled = true;
	}
	PostProcess::Unlock();
}

void SplashScreen::Create( bln is_active )
{
	is_Active = is_active;

	ASSUME( COUNTOF( specials1 ) == COUNTOF( specials2 ) );

	D3D11_BLEND_DESC o_blend = D3D11_BLEND_DESC();
	o_blend.RenderTarget[ 0 ].BlendEnable = true;
	o_blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	o_blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	o_blend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
	o_blend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
	o_blend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
	o_blend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	o_blend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;

	for( ui32 index = 0; index < COUNTOF( specials1 ); ++index )
	{
		specials1[ index ].i_blend = BlendStatesManager::GetState( &o_blend );
		specials2[ index ].i_blend = BlendStatesManager::GetState( &o_blend );
	}

	o_blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_COLOR;
	o_blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;

	specials2[ 4 ].i_blend = BlendStatesManager::GetState( &o_blend );

	ID3D11ShaderResourceView *i_back = TextureLoader::Load( specials1[ 0 ].texture, &ef1.w, &ef1.h );
	ef1.id = PostProcess::EffectPush( &PostProcess::Effects::Textured( i_back, f128color( 1, 1, 1, 1 ), specials1[ 0 ].i_blend,
		ScreenRect( -1, 1, 1, -1 ), vec2( 0, 0 ), 0, vec2( 1, 1 ), vec2( 0.5, 0.5 ) ) );

	ID3D11ShaderResourceView *i_nicoTex = TextureLoader::Load( specials2[ 0 ].texture, &ef2.w, &ef2.h );
	ef2.id = PostProcess::EffectPush( &PostProcess::Effects::Textured( i_nicoTex, f128color( 1, 1, 1, 1 ), specials2[ 0 ].i_blend,
		ScreenRect( -0.15, 0.5, 0.5, 0.5 - (0.65 / ((f32)ef2.w / (f32)ef2.h) * ((f32)Globals::Width / (f32)Globals::Height)) ), vec2( 0, 0 ), 0 ) );

	((PostProcess::Effects::Textured *)PostProcess::LockEffectByID( ef1.id ))->_is_enabled = is_active;
	((PostProcess::Effects::Textured *)PostProcess::LockEffectByID( ef2.id ))->_is_enabled = is_active;
	PostProcess::Unlock();
}