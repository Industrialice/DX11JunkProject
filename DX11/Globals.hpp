#ifndef __GLOBALS_HPP__
#define __GLOBALS_HPP__

#include <FileIO.hpp>
#include "IDManager.hpp"

namespace Globals
{
    DX11_EXPORT extern HINSTANCE h_Inst;
    DX11_EXPORT extern HWND h_Wnd;
    DX11_EXPORT extern i32 Width, Height;
    DX11_EXPORT extern bln is_Windowed;
    DX11_EXPORT extern bln is_UseWARP;
    DX11_EXPORT extern i32 CursorX, CursorY;
    DX11_EXPORT extern f32 DT;
    DX11_EXPORT extern ui64 Time;  //  microseconds count
    DX11_EXPORT extern bln is_StopRendering;
    DX11_EXPORT extern f32 TotalShaderLoadingTime;
    DX11_EXPORT extern f32 TotalTextureLoadingTime;
	DX11_EXPORT extern id_t FilmGrainId;
	DX11_EXPORT extern bln is_DrawDebugShapes;

    DX11_EXPORT extern CLogger *DefaultLogger;
}

#endif __GLOBALS_HPP__