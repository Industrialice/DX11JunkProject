#ifndef __POST_PROCESS_HPP__
#define __POST_PROCESS_HPP__

#include "IDManager.hpp"

#define POSTPROCESS_THREADSAFE

namespace PostProcess
{
    namespace Effects
    {
        namespace Private
        {
            namespace ProcessActions
            {
                CONSTS_OPED( actionsEnum, nothing = 0,
                                          draw = BIT( 1 ), 
                                          destruct = BIT( 2 ) );
            }

            struct EffectBase
            {
                typedef uiw (*ProcessFunc_t)( void *p_mem, Private::ProcessActions::actionsEnum actions );
                ProcessFunc_t _procFunc;
                id_t _id;
                ScreenRect _rect;
                m3x3 _transformation;
                bln _is_enabled;
        
                EffectBase( ProcessFunc_t procFunc, const ScreenRect &rectIn, const m3x3 &transformationIn = co_Iden3x3 )
                {
                    _procFunc = procFunc;
                    _rect = rectIn;
                    _is_enabled = true;
                    _transformation = transformationIn;
                }
        
            protected:
                static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
            };
        }

        struct FilmGrain : Private::EffectBase
        {
            f32 _noiseScale;

            FilmGrain( f32 noiseScale, const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ) ) : EffectBase( Process, rectIn )
            {
                _noiseScale = noiseScale;
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };

        struct FilmGrainColored : Private::EffectBase
        {
            f32 _noiseScale;

            FilmGrainColored( f32 noiseScale, const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ) ) : EffectBase( Process, rectIn )
            {
                _noiseScale = noiseScale;
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };

        struct BlackWhite : Private::EffectBase
        {
            f96color _o_colorLow;
            f96color _o_colorHigh;

            BlackWhite( const f96color &o_colorLow, const f96color &o_colorHigh, const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ), const m3x3 &transformationIn = co_Iden3x3 ) : EffectBase( Process, rectIn, transformationIn )
            {
                _o_colorLow = o_colorLow;
                _o_colorHigh = o_colorHigh;
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };

        struct InvertColors : Private::EffectBase
        {
            InvertColors( const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ), const m3x3 &transformationIn = co_Iden3x3 ) : EffectBase( Process, rectIn, transformationIn )
            {
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };

        struct PrettyColors : Private::EffectBase
        {
            f128color o_color;

            PrettyColors( const f128color &o_colorIn, const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ) ) : EffectBase( Process, rectIn )
            {
                o_color = o_colorIn;
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };

        struct Gray : Private::EffectBase
        {
            Gray( const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ) ) : EffectBase( Process, rectIn )
            {
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };

        struct GrayFactor : Private::EffectBase
        {
            ui32 _colors;

            GrayFactor( ui32 colors, const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ) ) : EffectBase( Process, rectIn )
            {
                _colors = colors;
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };

        struct Textured : Private::EffectBase
        {
            ID3D11ShaderResourceView *i_tex;
            f128color o_mult;
            ID3D11BlendState *i_blend;
            vec2 texOffset;
            f32 texRotate;
            vec2 texScale;
            vec2 texRotateCenter;

            Textured( ID3D11ShaderResourceView *i_texIn, const f128color &o_multIn, ID3D11BlendState *i_blendIn, const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ), const vec2 &texOffsetIn = vec2( 0, 0 ), f32 texRotateIn = 0, const vec2 &texScaleIn = vec2( 1, 1 ), const vec2 &texRotateCenterIn = vec2( 0.5, 0.5 ) ) : EffectBase( Process, rectIn )
            {
                i_tex = i_texIn;
                o_mult = o_multIn;
                i_blend = i_blendIn;
                texOffset = texOffsetIn;
                texRotate = texRotateIn;
                texScale = texScaleIn;
                texRotateCenter = texRotateCenterIn;
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };

        struct Colored : Private::EffectBase
        {
            f128color o_color;
            ID3D11BlendState *i_blend;

            Colored( const f128color &o_colorIn, ID3D11BlendState *i_blendIn, const ScreenRect &rectIn = ScreenRect( -1, 1, 1, -1 ) ) : EffectBase( Process, rectIn )
            {
                o_color = o_colorIn;
                i_blend = i_blendIn;
            }

            static uiw Process( void *p_mem, Private::ProcessActions::actionsEnum actions );
        };
    }

    void *LockEffectByIndex( ui32 index );
    void *LockEffectByID( id_t id );
    void Unlock();  //  must be used after locks when POSTPROCESS_THREADSAFE is defined
    void EffectSetByIndex( ui32 index, const void *effect );
    void EffectSetByID( id_t index, const void *effect );
    id_t EffectPush( const void *effect );
    void EffectDeleteByIndex( ui32 index );
    void EffectDeleteByID( id_t id );
    void EffectPop();
    void EffectsClear();
    ui32 EffectsCount();

    namespace Private
    {
        void Initialize();
        void Draw();
    }
}

#endif __POST_PROCESS_HPP__