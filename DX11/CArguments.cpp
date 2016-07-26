#include "PreHeader.hpp"
#include "CArguments.hpp"

namespace
{
	const char *const cpc_TrueAnswers = "true yes";
	const char *const cpc_FalseAnswers = "false no";
}

CArguments::~CArguments()
{}

CArguments::CArguments()
{
	_cp_trueAnswers = cpc_TrueAnswers;
	_cp_falseAnswers = cpc_FalseAnswers;
	_termDelim = ' ';
	_termNewLine = '\n';
	_termRecognize = '\\';
}

/*CArguments::CArguments( const CArguments &source )
{
    ASSUME( this != &source );
    _lineLen = source._lineLen;
    _cp_trueAnswers = source._cp_trueAnswers;
    _cp_falseAnswers = source._cp_falseAnswers;
    _termDelim = source._termDelim;
    _termNewLine = source._termNewLine;
    _termRecognize = source._termRecognize;
    _o_arguments = source._o_arguments;
    _p_line = source._lineLen ? (char *)malloc( source._lineLen ) : 0;
}

CArguments &CArguments::operator = ( const CArguments &source )
{
    if( this != &source )
    {
        _lineLen = source._lineLen;
        _cp_trueAnswers = source._cp_trueAnswers;
        _cp_falseAnswers = source._cp_falseAnswers;
        _termDelim = source._termDelim;
        _termNewLine = source._termNewLine;
        _termRecognize = source._termRecognize;
        _o_arguments = source._o_arguments;
        if( source._lineLen )
        {
            _p_line = (char *)realloc( _p_line, source._lineLen );
        }
        else
        {
            free( _p_line );
            _p_line = 0;
        }
    }
    return *this;
}*/

NOINLINE void CArguments::AddAgsAL( const char *cp_agLine )
{
    ASSUME( cp_agLine );
    if( !*cp_agLine )
    {
        return;
    }
    for( ; ; )
    {
        ui32 len;
        const char *cp_end = Funcs::StrChrMask( cp_agLine, _termRecognize, _termDelim );
        if( cp_end )
        {
            len = cp_end - cp_agLine;
        }
        else
        {
            len = _StrLen( cp_agLine );
        }

        ui32 excludedLen = Funcs::StrExcludeMaskAdv( 0, cp_agLine, _termRecognize, _termRecognize, len );

        ui32 index = _o_line.Size();
        _o_arguments.AppendNum()->index = index;

		_o_line.AppendNum( excludedLen + 1 );

        Funcs::StrExcludeMaskAdv( _o_line.Data() + index, cp_agLine, _termRecognize, _termRecognize, len );

        _o_arguments.Back().fmt = Dissect( _o_line.Data() + index, excludedLen );

        if( !cp_end )
        {
            break;
        }
        cp_agLine += len + 1;
    }
}

/*va_return CArguments::AddAgsFmt( char ag0type, ... )
{
    DBGCODE( ui32 agsProced = 1 );
    DBGCODE( return agsProced );
    DBGBREAK;
}*/

NOINLINE void CArguments::InsertAgsAL( ui32 startIndex, const char *cp_agLine )
{
    ASSUME( startIndex <= _o_arguments.Size() && cp_agLine );
    if( !*cp_agLine )
    {
        return;
    }
    if( startIndex == _o_arguments.Size() )
    {
        AddAgsAL( cp_agLine );
        return;
    }
    for( ui32 ag = startIndex; ; ++ag )
    {
        ui32 len;
        const char *cp_end = Funcs::StrChrMask( cp_agLine, _termRecognize, _termDelim );
        if( cp_end )
        {
            len = cp_end - cp_agLine;
        }
        else
        {
            len = _StrLen( cp_agLine );
        }

        ui32 excludedLen = Funcs::StrExcludeMaskAdv( 0, cp_agLine, _termRecognize, _termRecognize, len );

        ui32 index = _o_arguments[ ag ].index;
        _o_arguments.InsertNum( ag, 1, false );
        _o_arguments[ ag ].index = index;

		//  TODO: vector insert?
        ui32 cpyLen = _o_line.Size() - index;

		_o_line.AppendNum( excludedLen + 1 );

        _MemMove( _o_line.Data() + index + excludedLen + 1, _o_line.Data() + index, cpyLen );

        Funcs::StrExcludeMaskAdv( _o_line.Data() + index, cp_agLine, _termRecognize, _termRecognize, len );
    
        _o_arguments[ ag ].fmt = Dissect( _o_line.Data() + index, excludedLen );

        for( ui32 correctionIndex = ag + 1; correctionIndex < _o_arguments.Size(); ++correctionIndex )
        {
            _o_arguments[ correctionIndex ].index += excludedLen + 1;
        }

        if( !cp_end )
        {
            break;
        }
        cp_agLine += len + 1;
    }
}

/*va_return CArguments::InsertAgsFmt( ui32 startIndex, char ag0type, ... )
{
    ASSUME( startIndex <= _o_arguments.Size() );
    DBGCODE( ui32 agsProced = 2 );
    DBGCODE( return agsProced );
}*/

ui32 CArguments::Size() const
{
	return _o_arguments.Size();
}

NOINLINE ui32 CArguments::Length() const
{
	ui32 len = 0;
	for( ui32 ag = 0; ag < _o_arguments.Size(); ++ag )
	{
		len += AgLen( ag );
	}
	return len;
}

void CArguments::Clear()
{
	_o_line.Clear();
    _o_arguments.Clear();
}

NOINLINE void CArguments::Remove( ui32 index, ui32 count /* = 1 */ )
{
    ASSUME( index + count <= _o_arguments.Size() );
    if( !count )
    {
        return;
    }
    if( index + count == _o_arguments.Size() )
    {
		_o_line.Resize( _o_arguments[ index ].index );
        _o_arguments.Resize( index );
    }
    else
    {
        uiw remLen = _o_arguments[ index + count ].index - _o_arguments[ index ].index;
		_o_line.Erase( _o_arguments[ index ].index, _o_arguments[ index + count ].index - _o_arguments[ index ].index );
        _o_arguments.Erase( index, count );
        for( ui32 ag = index; ag < _o_arguments.Size(); ++ag )
        {
            _o_arguments[ ag ].index -= remLen;
        }
    }
}

NOINLINE i32 CArguments::AgI( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index) == AgLen( index ) - 1) );
	if( _o_arguments[ index ].fmt == FmtDec )
	{
		return Funcs::StrDecToI32( _o_line.Data() + _o_arguments[ index ].index );
	}
	if( _o_arguments[ index ].fmt == FmtHex )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrHexToInt < i32 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
	if( _o_arguments[ index ].fmt == FmtBin )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrBinToInt < i32 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
    if( _o_arguments[ index ].fmt == FmtFP )
    {
        return (i32)Funcs::StrToF32( _o_line.Data() + _o_arguments[ index ].index );
    }
	return 0;
}

NOINLINE i64 CArguments::AgI64( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
	if( _o_arguments[ index ].fmt == FmtDec )
	{
		return Funcs::StrDecToI64( _o_line.Data() + _o_arguments[ index ].index );
	}
	if( _o_arguments[ index ].fmt == FmtHex )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrHexToInt < i64 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
	if( _o_arguments[ index ].fmt == FmtBin )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrBinToInt < i64 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
    if( _o_arguments[ index ].fmt == FmtFP )
    {
        return (i64)Funcs::StrToF32( _o_line.Data() + _o_arguments[ index ].index );
    }
	return 0;
}

NOINLINE ui32 CArguments::AgU( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
	if( _o_arguments[ index ].fmt == FmtDec )
	{
		return Funcs::StrDecToUI32( _o_line.Data() + _o_arguments[ index ].index );
	}
	if( _o_arguments[ index ].fmt == FmtHex )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrHexToInt < ui32 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
	if( _o_arguments[ index ].fmt == FmtBin )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrBinToInt < ui32 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
    if( _o_arguments[ index ].fmt == FmtFP )
    {
        return (ui32)Funcs::StrToF32( _o_line.Data() + _o_arguments[ index ].index );
    }
	return 0;
}

NOINLINE ui64 CArguments::AgU64( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
	if( _o_arguments[ index ].fmt == FmtDec )
	{
		return Funcs::StrDecToUI64( _o_line.Data() + _o_arguments[ index ].index );
	}
	if( _o_arguments[ index ].fmt == FmtHex )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrHexToInt < ui64 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
	if( _o_arguments[ index ].fmt == FmtBin )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrBinToInt < ui64 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
    if( _o_arguments[ index ].fmt == FmtFP )
    {
        return (ui64)Funcs::StrToF32( _o_line.Data() + _o_arguments[ index ].index );
    }
	return 0;
}

NOINLINE f32 CArguments::AgF( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
	if( _o_arguments[ index ].fmt == FmtDec || _o_arguments[ index ].fmt == FmtFP )
	{
		return Funcs::StrToF32( _o_line.Data() + _o_arguments[ index ].index );
	}
	if( _o_arguments[ index ].fmt == FmtHex )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrHexToInt < ui32 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
	if( _o_arguments[ index ].fmt == FmtBin )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrBinToInt < ui32 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
	return 0.f;
}

NOINLINE f64 CArguments::AgF64( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
	if( _o_arguments[ index ].fmt == FmtDec || _o_arguments[ index ].fmt == FmtFP )
	{
		return Funcs::StrToF64( _o_line.Data() + _o_arguments[ index ].index );
	}
	if( _o_arguments[ index ].fmt == FmtHex )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrHexToInt < ui64 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
	if( _o_arguments[ index ].fmt == FmtBin )
	{
		ASSUME( AgLen( index ) > 2 );
		return Funcs::StrBinToInt < ui64 >( _o_line.Data() + _o_arguments[ index ].index + 2 );
	}
	return 0.0;
}

bln CArguments::AgB( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
	return AgA( index ) == 1;
}

void *CArguments::AgP( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
	return (void *)WSC( AgU, AgU64 )( index );
}

const char *CArguments::AgS( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
    if( _o_arguments[ index ].fmt == FmtUser )
    {
        return 0;
    }
	return _o_line.Data() + _o_arguments[ index ].index;
}

NOINLINE i32 CArguments::AgA( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
	ASSUME( _cp_falseAnswers && _cp_trueAnswers );
	if( _o_arguments[ index ].fmt != FmtString )
	{
        if( _o_arguments[ index ].fmt == FmtUser )
        {
            return -1;
        }
        return AgI64( index ) != 0;
	}
	struct S
	{
		static NOINLINE bln IsContains( const char *cp_where, const char *cp_what )
		{
			ui32 whatLen = _StrLen( cp_what );
			for( ; ; )
			{
				const char *cp_next = _StrChr( cp_where, ' ' );
				ui32 len = cp_next ? (cp_next - cp_where) : _StrLen( cp_where );
				if( len == whatLen && Funcs::StrINEqual( cp_where, cp_what, len ) )
				{
					return true;
				}
				if( !cp_next )
				{
					break;
				}
				cp_where += len + 1;
			}
			return false;
		}
	};
	if( S::IsContains( _cp_falseAnswers, _o_line.Data() + _o_arguments[ index ].index ) )
	{
		return 0;
	}
	if( S::IsContains( _cp_trueAnswers, _o_line.Data() + _o_arguments[ index ].index ) )
	{
		return 1;
	}
	return -1;
}

void *CArguments::AgUser( ui32 index, uiw *p_size ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
    if( _o_arguments[ index ].fmt == FmtUser )
    {
        uiw size;
        uiw addr = (uiw)(_o_line.Data() + _o_arguments[ index ].index);
        _MemCpy( &size, (byte *)addr, sizeof(uiw) );
        DSA( p_size, size );
        addr += sizeof(uiw);
        addr = (addr + (DATA_ALIGNMENT - 1)) & ~(DATA_ALIGNMENT - 1);
        ASSUME( addr + size == (uiw)_o_line.Data() + _o_arguments[ index ].index + AgLen( index ) );
        return (void *)addr;
    }
    DSA( p_size, 0 );
    return 0;
}

NOINLINE ui32 CArguments::AgsLength( ui32 index, ui32 count /* = 1 */ ) const
{
	ASSUME( index + count <= _o_arguments.Size() );
	ui32 len = 0;
	for( ui32 ag = 0; ag < count; ++ag )
	{
        if( _o_arguments[ index + ag ].fmt != FmtUser )
        {
            len += AgLen( index + ag ) - 1;
        }
        else
        {
            uiw size;
            uiw addr = (uiw)(_o_line.Data() + _o_arguments[ index + ag ].index);
            _MemCpy( &size, (byte *)addr, sizeof(uiw) );
            addr += sizeof(uiw);
            addr = (addr + (DATA_ALIGNMENT - 1)) & ~(DATA_ALIGNMENT - 1);
            ASSUME( addr + size == (uiw)_o_line.Data() + _o_arguments[ index + ag ].index + AgLen( index + ag ) );
            len += AgLen( index + ag ) - (addr - (uiw)(_o_line.Data() + _o_arguments[ index + ag ].index));
        }
	}
	return len;
}

void CArguments::AnswerStringsGet( const char **cpp_true, const char **cpp_false ) const
{
	DSA( cpp_true, _cp_trueAnswers );
    DSA( cpp_false, _cp_falseAnswers );
}

void CArguments::AnswerStringsSet( const char *cp_true, const char *cp_false )
{
    if( cp_true )
    {
	    _cp_trueAnswers = cp_true;
    }
    if( cp_false )
    {
	    _cp_falseAnswers = cp_false;
    }
}

void CArguments::TerminatorSymbolsGet( char *p_recognize, char *p_newline, char *p_delim ) const
{
    DSA( p_recognize, _termRecognize );
    DSA( p_newline, _termRecognize );
    DSA( p_delim, _termDelim );
}

void CArguments::TerminatorSymbolsSet( char recognize, char newline, char delim )
{
	if( recognize )
	{
		_termRecognize = recognize;
	}
	if( newline )
	{
		_termNewLine = newline;
	}
	if( delim )
	{
		_termDelim = delim;
	}
}

CArguments::AgFmt CArguments::FmtGet( ui32 index ) const
{
	ASSUME( index < _o_arguments.Size() && (_o_arguments[ index ].fmt == FmtUser || _StrLen( _o_line.Data() + _o_arguments[ index ].index ) == AgLen( index ) - 1) );
    return _o_arguments[ index ].fmt;
}

NOINLINE bln CArguments::MaskS( ui32 index, char mask ) const
{
	if( index >= _o_arguments.Size() )
	{
		return false;
	}
    if( mask == 'u' )
    {
        return _o_arguments[ index ].fmt == FmtUser;
    }
	if( mask == 's' || mask == 'a' )
	{
		return _o_arguments[ index ].fmt != FmtUser;
	}
	if( mask != 'n' )
	{
		SOFTBREAK;
		return false;
	}
	return _o_arguments[ index ].fmt != FmtString && _o_arguments[ index ].fmt != FmtUser;
}

NOINLINE bln CArguments::MaskM( const char *cp_masks, ui32 startIndex /* = 0 */ ) const
{
	ASSUME( cp_masks );
	for( ui32 agIndex = startIndex; *cp_masks; ++agIndex, ++cp_masks )
	{
		if( !MaskS( *cp_masks, agIndex ) )
		{
			return false;
		}
	}
	return true;
}

NOINLINE CArguments::AgFmt CArguments::Dissect( const char *cp_str, ui32 len ) const
{
    if( !len )
    {
        return FmtString;
    }
    if( *cp_str == '+' || *cp_str == '-' )
    {
        --len;
        ++cp_str;
    }
    if( Funcs::IsStrDec( cp_str, len ) )
    {
        return FmtDec;
    }
    if( Funcs::IsStrBin( cp_str, len ) )
    {
        return FmtBin;
    }
    if( Funcs::IsStrHex( cp_str, len ) )
    {
        return FmtHex;
    }
    if( Funcs::IsStrFP( cp_str, len ) )
    {
        return FmtFP;
    }
    return FmtString;
}

ui32 CArguments::AgLen( ui32 index ) const  //  private
{
	ASSUME( index < _o_arguments.Size() );
	if( index == _o_arguments.Size() - 1 )
	{
		return _o_line.Size() - _o_arguments.Back().index;
	}
	return _o_arguments[ index + 1 ].index - _o_arguments[ index ].index;
}
