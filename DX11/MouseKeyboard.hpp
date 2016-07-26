#ifndef __MOUSE_KEYBOARD_HPP__
#define __MOUSE_KEYBOARD_HPP__

#include "VKeys.hpp"

const uiw MouseKeyboardProcTypes = 4;

struct SMouseKeyboardProcs  //  function pointers allowed to be null
{
	void (*OnKeyDown)( ui8 key );
	void (*OnKeyUp)( ui8 key );
	void (*OnMouseMove)( i32 x, i32 y );  //  relative
	void (*OnMouseWheelMove)( i32 delta );
};

namespace MouseKeyboard
{
    ui32 NumKeysDowned();
	bln IsKeyDown( ui8 key );
    ui64 WhenPressed( ui8 key );  //  ui64_max is not pressed
    f32 TimePressed( ui8 key );  //  0 if not pressed
	i32 MouseXGet();
	i32 MouseYGet();

	void ProcsSet( const SMouseKeyboardProcs *cpo_procs );
	void ProcsGet( SMouseKeyboardProcs *po_procs );

    struct SKeyHistorical
    {
        ui8 key;
        ui64 timeWhenPressed;
    };
	void ResetKeyDowns();
    void ClearHistory();
    const SKeyHistorical &HistoryAcquire( ui8 offsetFromBegin );
	ui8 HistoryDepth();
	bln IsTopOfHistory( const ui8 *keys, uiw keysCount, bln is_newestToOldest );

	///// will trigger events /////
	void KeyDown( ui8 key );
	void KeyUp( ui8 key );
	void MouseXSet( i32 x );          //  ...
	void MouseYSet( i32 y );          //  ...
	void MouseXYSet( i32 x, i32 y );  //  will call MouseMove event, even if new coordinates are the same
	void MouseMove( i32 x, i32 y );
	void MouseWheelMove( i32 delta );
	///////////////////////////////
}

#endif __MOUSE_KEYBOARD_HPP__
