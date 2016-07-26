#include "PreHeader.hpp"
#include "Globals.hpp"

namespace Globals
{
    HINSTANCE h_Inst;
    HWND h_Wnd;
    i32 Width, Height;
    bln is_Windowed;
    bln is_UseWARP;
    i32 CursorX, CursorY;
    f32 DT;
    ui64 Time;
    bln is_StopRendering = true;
    f32 TotalShaderLoadingTime;
    f32 TotalTextureLoadingTime;
	id_t FilmGrainId;
	bln is_DrawDebugShapes = false;

    CLogger *DefaultLogger;

    heapid DHeap;
}