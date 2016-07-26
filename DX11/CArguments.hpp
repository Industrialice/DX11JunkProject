#ifndef __CARGUMENTS_HPP__
#define __CARGUMENTS_HPP__

#include <CVector.hpp>

class CArguments
{
	enum AgFmt { FmtDec, FmtBin, FmtHex, FmtFP, FmtString, FmtUser };
	struct SAg
	{
		ui32 index;
		AgFmt fmt;
	};
	StdLib::CVec < SAg > _o_arguments;
	StdLib::CVec < char > _o_line;
	const char *_cp_trueAnswers, *_cp_falseAnswers;
	char _termRecognize, _termNewLine, _termDelim;

public:
	~CArguments();
	CArguments();
    //CArguments( const CArguments &source );
    //CArguments &operator = ( const CArguments &source );

    void AddAgsAL( const char *cp_agLine );
    //va_return AddAgsFmt( char ag0type, ... );
    void InsertAgsAL( ui32 startIndex, const char *cp_agLine );  //  index must be less or equal than current arguments count
    //va_return InsertAgsFmt( ui32 startIndex, char ag0type, ... );  //  index must be less or equal than current arguments count

	ui32 Size() const;
    ui32 Length() const;  //  overall length of all arguments without zero symbols
    void Clear();
    void Remove( ui32 index, ui32 count = 1 );  //  count will be clamped

    i32         AgI   ( ui32 index ) const;
    i64         AgI64 ( ui32 index ) const;
    ui32        AgU   ( ui32 index ) const;
    ui64        AgU64 ( ui32 index ) const;
    f32         AgF   ( ui32 index ) const;
    f64         AgF64 ( ui32 index ) const;
    bln         AgB   ( ui32 index ) const;  //  equals to AgA( index ) == 1
    void       *AgP   ( ui32 index ) const;  //  convert ag to 4/8 byte address
    const char *AgS   ( ui32 index ) const;  //  will become invalid after Clear() or Remove( index - n (n <= 0) ), will return 0 if not applicable
    i32         AgA   ( ui32 index ) const;  //  return answer from argument, 1 - true, 0 - false, -1 - fail
    void       *AgUser( ui32 index, uiw *p_size ) const;  //  p_size can be null. will become invalid after Clear() or Remove( index - n (n <= 0) ), will return 0 and will set p_size to 0 if not applicable

    ui32 AgsLength( ui32 index, ui32 count = 1 ) const;  //  lengths of arguments (strings zero symbol is not counted)

    void AnswerStringsGet( const char **cpp_true, const char **cpp_false ) const;  //  pass null to ignore
    void AnswerStringsSet( const char *cp_true, const char *cp_false );  //  pass null to ignore. simple pointers assignment, so make memory management on your side

    void TerminatorSymbolsGet( char *p_recognize, char *p_newline, char *p_delim ) const;  //  pass null pointer to ignore
    void TerminatorSymbolsSet( char recognize, char newline, char delim );  //  pass '\0' symbol to left the old one

    AgFmt FmtGet( ui32 index ) const;

    //  n - number, s - string, u - user, a - answer. mask by string is true on numbers. zero length argument is string
    bln MaskS( ui32 index, char mask ) const;  //  if index is out of arguments count range - always false
    bln MaskM( const char *cp_masks, ui32 startIndex = 0 ) const;  //  mask is empty - always true, out of arguments count range - always false

private:

    AgFmt Dissect( const char *cp_str, ui32 len ) const;
    ui32 AgLen( ui32 index ) const;
};

#endif __CARGUMENTS_HPP__
