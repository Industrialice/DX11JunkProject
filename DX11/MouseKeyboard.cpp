#include "PreHeader.hpp"
#include "MouseKeyboard.hpp"
#include "Globals.hpp"
#include <PackedIntArray.hpp>

class CKeyCircularBuffer
{
	MouseKeyboard::SKeyHistorical _buf[ 256 ];
	ui8 _bufPos;
	ui8 _depth;

public:
	CKeyCircularBuffer()
	{
		Clear();
	}

    void Clear()
    {
		using namespace Private;
        for( ui32 index = 0; index < 256; ++index )
        {
            _buf[ index ].key = VKeys::Undefined;
            _buf[ index ].timeWhenPressed = ui64_max;
        }
        _bufPos = 0;
		_depth = 0;
    }

    void Push( ui8 key, ui64 time )
    {
		using namespace Private;
		++_depth;
        ++_bufPos;
        _buf[ _bufPos ].key = key;
        _buf[ _bufPos ].timeWhenPressed = time;
    }

    const MouseKeyboard::SKeyHistorical &Acquire( ui8 offset )
    {
		using namespace Private;
		ASSUME( _depth > offset );
        return _buf[ _bufPos - offset ];
    }

	ui8 Depth()
	{
		using namespace Private;
		return _depth;
	}
} KeyCircularBuffer;

namespace
{
    ui32 KeysDownedCount;
	bln is_DisableEvents;
	packiarr_static < 256, 1, false > o_KeysStatus( 256, 0 );
    ui64 PressedTimes[ 256 ];
	i32 MouseX, MouseY;
	const SMouseKeyboardProcs co_DefProcs = { DummyFunc < void, ui8 >, DummyFunc < void, ui8 >, DummyFunc < void, i32, i32 >, DummyFunc < void, i32 > };
	SMouseKeyboardProcs o_Procs = co_DefProcs;
}

ui32 MouseKeyboard::NumKeysDowned()
{
    return KeysDownedCount;
}

bln MouseKeyboard::IsKeyDown( ui8 key )
{
	return o_KeysStatus.Get( key );
}

ui64 MouseKeyboard::WhenPressed( ui8 key )
{
    if( !o_KeysStatus.Get( key ) )
    {
        return ui64_max;
    }
    return PressedTimes[ key ];
}

f32 MouseKeyboard::TimePressed( ui8 key )
{
    if( !o_KeysStatus.Get( key ) )
    {
        return 0.f;
    }
    return NSecTimeToF32( Globals::Time - PressedTimes[ key ] );
}

i32 MouseKeyboard::MouseXGet()
{
	return MouseX;
}

i32 MouseKeyboard::MouseYGet()
{
	return MouseY;
}

void MouseKeyboard::ProcsSet( const SMouseKeyboardProcs *cpo_procs )
{
	if( !cpo_procs )
	{
		o_Procs = co_DefProcs;
		return;
	}
	o_Procs = *cpo_procs;
	void **pp_source = (void **)cpo_procs;
	void **pp_target = (void **)&o_Procs;
	void **pp_default = (void **)&co_DefProcs;
	ASSUME( MouseKeyboardProcTypes <= sizeof(SMouseKeyboardProcs) / sizeof(void *));
	for( ui32 proc = 0; proc < MouseKeyboardProcTypes; ++proc )
	{
		if( !pp_source[ proc ] )
		{
			pp_target[ proc ] = pp_default[ proc ];
		}
	}
}

void MouseKeyboard::ProcsGet( SMouseKeyboardProcs *po_procs )
{
	ASSUME( po_procs );
	*po_procs = o_Procs;
	void **pp_target = (void **)po_procs;
	void **pp_default = (void **)&co_DefProcs;
	ASSUME( MouseKeyboardProcTypes <= sizeof(SMouseKeyboardProcs) / sizeof(void *));
	for( ui32 proc = 0; proc < MouseKeyboardProcTypes; ++proc )
	{
		if( pp_target[ proc ] == pp_default[ proc ] )
		{
			pp_target[ proc ] = 0;
		}
	}
}

void MouseKeyboard::ResetKeyDowns()
{
	o_KeysStatus = decltype(o_KeysStatus)( 256, 0 );
    KeyCircularBuffer.Clear();
}

void MouseKeyboard::ClearHistory()
{
    KeyCircularBuffer.Clear();
}

const MouseKeyboard::SKeyHistorical &MouseKeyboard::HistoryAcquire( ui8 offsetFromBegin )
{
    return KeyCircularBuffer.Acquire( offsetFromBegin );
}

ui8 MouseKeyboard::HistoryDepth()
{
	return KeyCircularBuffer.Depth();
}

bln MouseKeyboard::IsTopOfHistory( const ui8 *keys, uiw keysCount, bln is_newestToOldest )
{
	if( keysCount > KeyCircularBuffer.Depth() )
	{
		return false;
	}
	if( is_newestToOldest )
	{
		for( uiw index = 0; index < keysCount; ++index )
		{
			if( KeyCircularBuffer.Acquire( index ).key != keys[ index ] )
			{
				return false;
			}
		}
	}
	else
	{
		for( uiw keyIndex = keysCount - 1, historyIndex = 0; keyIndex != uiw_max; --keyIndex, ++historyIndex )
		{
			if( KeyCircularBuffer.Acquire( historyIndex ).key != keys[ keyIndex ] )
			{
				return false;
			}
		}
	}
	return true;
}

void MouseKeyboard::KeyDown( ui8 key )
{
	if( is_DisableEvents )
	{
		return;
	}
	is_DisableEvents = true;
	if( !o_KeysStatus.Get( key ) )
	{
        ++KeysDownedCount;
        ASSUME( KeysDownedCount <= ui8_max );
		o_KeysStatus.Set( key, 1 );
		o_Procs.OnKeyDown( key );
        PressedTimes[ key ] = Globals::Time;
        KeyCircularBuffer.Push( key, Globals::Time );
	}
	is_DisableEvents = false;
}

void MouseKeyboard::KeyUp( ui8 key )
{
	if( is_DisableEvents )
	{
		return;
	}
	is_DisableEvents = true;
	if( o_KeysStatus.Get( key ) )
	{
        ASSUME( KeysDownedCount );
        --KeysDownedCount;
		o_KeysStatus.Set( key, 0 );
		o_Procs.OnKeyUp( key );
	}
	is_DisableEvents = false;
}

void MouseKeyboard::MouseXSet( i32 x )
{
	MouseMove( x - MouseX, 0 );
}

void MouseKeyboard::MouseYSet( i32 y )
{
	MouseMove( 0, y - MouseY );
}

void MouseKeyboard::MouseXYSet( i32 x, i32 y )
{
	MouseMove( x - MouseX, y - MouseY );
}

NOINLINE void MouseKeyboard::MouseMove( i32 x, i32 y )
{
	if( is_DisableEvents )
	{
		return;
	}
	is_DisableEvents = true;
	MouseX += x;
	MouseY += y;
	o_Procs.OnMouseMove( x, y );
	is_DisableEvents = false;
}

void MouseKeyboard::MouseWheelMove( i32 delta )
{
	if( is_DisableEvents )
	{
		return;
	}
	is_DisableEvents = true;
	o_Procs.OnMouseWheelMove( delta );
	is_DisableEvents = false;
}
