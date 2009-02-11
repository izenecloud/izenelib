/**
 * @brief	Source file of UString class 
 * @author	Do Hyun Yun
 * @date	2008-06-26
 */

#include <fstream>

#include <ustring/UString.h>
#include <ustring/UStringException.h>

#include <ustring/UCS2_Table.h>
#include <ustring/converters.h>
#include <boost/shared_ptr.hpp>

using namespace std;
using namespace iconvLibrary;

namespace sf1lib {



// -----------------------------[ Static member variables ]

ConvertFunction UString::ConvertFunctionList[UString::TOTAL_ENCODING_TYPE_NO] = {

    // UNKNOWN 
    {0, 0},

    // UTF_8 
    {&utf8_mbtowc, &utf8_wctomb},

    // korean EUC_KR & CP_949
    //{&euc_kr_mbtowc, &euc_kr_wctomb},
    {&cp949_mbtowc, &cp949_wctomb},
    {&cp949_mbtowc, &cp949_wctomb},

    // japanese EUC_JP & SJIS
    {&euc_jp_mbtowc, &euc_jp_wctomb},
    {&sjis_mbtowc, &sjis_wctomb},

    // chinese GB2312 & BIG_5
    {&gb2312_mbtowc, &gb2312_wctomb},
    {&big5_mbtowc, &big5_wctomb},

    // latin ISO8859_15
    {&iso8859_15_mbtowc, &iso8859_15_wctomb},
};

const char UString::EncodingTypeString[10][30] = {
    "UNKNOWN",
    "UTF-8",
    "EUC-KR", "CP949",
    "EUC-JP", "SJIS",
    "GB2312", "BIG-5",
    "ISO8859-15",
    "TOTAL_ENCODING_TYPE_NO"
};  ///< a string which offers encoding type of the string.

UString::EncodingType UString::convertEncodingTypeFromStringToEnum(const char* encodingTypeString)
{

    // Index variables are used for string matching. 
    int stringIndex;
    int encodingTypeIndex;
    char givenCharacter, encodingTypeCharacter;
    bool isEncodingTypeFound = false;

    stringIndex = encodingTypeIndex = 0;

    while(1)
    {
        givenCharacter = encodingTypeString[stringIndex];

        // to change upper charactor 2008-11-17
        if ( givenCharacter > 96 && givenCharacter < 123 )
            givenCharacter -= 32;
        
        encodingTypeCharacter = UString::EncodingTypeString[encodingTypeIndex][stringIndex];

        if (givenCharacter == 0)
        {
            // If given string is fully matched to the certain EncodingTypeString, set true.
            if (encodingTypeCharacter == 0)
                isEncodingTypeFound = true;
            break;
        } // end - if

        if ( givenCharacter == encodingTypeCharacter ) {
            stringIndex++;
            continue;
        } // end - if

        // Given string is not matched to this encodingTypeString. Compare it to another encodingTypeString.
        stringIndex = 0;
        encodingTypeIndex++;

        // Stop compare processing. It is the end of encodingTypeString list.
        if (encodingTypeIndex == 10) // total number of encodingTypeStrings
            break;
    } // end - while()

    // If given string is fully matched, return its type value
    if (isEncodingTypeFound)
        return EncodingType(encodingTypeIndex);

    // If it isn't matched, return 0.
    return UNKNOWN;

} // end - convertEncodingTypeFromStr



// -----------------------------[ Constructor & Destructor ]

UString::UString(void) : bufferSize_(0), length_(0), data_(0), isLengthChanged_(false)
{
    // Allocate default size to the data_ of UString.
    bufferSize_ = UString::DEFAULT_SIZE;

    try {
        //data_ = new UCS2Char[bufferSize_];
        //data_[0] = static_cast<UCS2Char>(NULL);
        data_ = static_cast<UCS2Char*>( malloc( bufferSize_ * sizeof(UCS2Char) ) );
        data_[0] = static_cast<UCS2Char>(NULL);
    }
    catch(UStringException ue)
    {
        ue.displayMessage();
    }
    catch(...)
    {
        cerr << "Unexpected Exception occurs" << endl;
        throw;
    }
}

UString::UString(const UString& initString) 
    : bufferSize_(0), length_(0), data_(0), isLengthChanged_(false)
{
    *this = initString;
}

UString::UString(const string&  initString, EncodingType encodingType)
    : bufferSize_(0), length_(0), data_(0), isLengthChanged_(false)

{
    assign(initString, encodingType);
}

UString::UString(const char*  initString, EncodingType encodingType)
    : bufferSize_(0), length_(0), data_(0), isLengthChanged_(false) 

{
    string init(initString);
    assign(init, encodingType);
}

UString::~UString()
{
    if (data_ != 0) {

        // Delete data area of UString.
        try {
            // Modified by TuanQuang Nguyen, Nov 02 2008
            // old code: delete data_;
            //
            // delete [] data_;
            free( data_ );
        }
        catch(UStringException ue)
        {
            ue.displayMessage();
        }
        catch(...)
        {
            cerr << "Unexpected Exception occurs in UString" << endl;
            throw;
        }
    }
}


// ---------------------------------------------[ assign() ]

UString& UString::assign(const string& inputString, EncodingType encodingType)
{
    // If the buffer size of UString is smaller than given string length, reallocate it.
    if ( bufferSize_ <= inputString.length() ) {

        //  if (data_ != 0) 
        //      //Modified by TuanQuang Nguyen, Nov 02 2008
        //      //old code: delete data_;
        //      //delete [] data_;
        //      free(data_);
        
        // Allocate flexible size according to the length of initString.
        bufferSize_ = inputString.length() + FLEXIBLE_FREE_SPACE;

        try {
            //data_ = new UCS2Char[bufferSize_];
            //data_[0] = static_cast<UCS2Char>(NULL);
            data_ = static_cast<UCS2Char*>( realloc(data_, bufferSize_ * sizeof(UCS2Char) ) );
            data_[0] = static_cast<UCS2Char>(NULL);
        }
        catch(UStringException ue)
        {
            ue.displayMessage();
        }
        catch(...)
        {
            cerr << "Unexpected Exception occurs" << endl;
            throw;
        }
    } // end - if

    // copy with converting the given string into data_
    setUString(inputString, encodingType);

    return *this;
} // end - assign()


UString& UString::assign(const char* inputString, EncodingType encodingType)
{
    string init(inputString);
    return assign( init, encodingType );
}



// ---------------------------------------------[ setUString() ]

void UString::setUString(const string& inputString, EncodingType encodingType)
{
    isLengthChanged_ = false;

    // Terminate converting if encoding type is not right value.
    if (encodingType == UString::UNKNOWN || encodingType == UString::TOTAL_ENCODING_TYPE_NO) {
        return;
    } // end - if

    int inputStringIndex  = 0;
    int inputStringLength = inputString.size(); // Assign the length

    int outputStringIndex = 0;
    int outputBufferSize  = bufferSize_;
    unsigned int outputUCSCharacter;

    int returnValue;



    // Coninue converting characters one by one while there is no space in output buffer or there is no character to read.
    while( outputBufferSize > 0 && inputStringLength > 0 )
    {

        // Using converting functions offered by iconv library. It will return byte size of converting character.
        returnValue = ConvertFunctionList[encodingType].convertToUCS(&outputUCSCharacter, (unsigned char*)&inputString[inputStringIndex], inputStringLength);

        // If there is no matched character in converting table, insert space character to the output.
        if (returnValue < 0) 
        {
            switch(returnValue)
            {
            case RET_ILSEQ:
                data_[outputStringIndex] = (UCS2Char)0x20; // insert space character to the UCS4 String
                inputStringLength--; inputStringIndex++;
                break;

            default: // Illigal return value. Stop converting.
                inputStringLength = 0;
                break;
            } // end - switch

        } // end - if

        // If there is matched character in converting table, insert right converting character to the output. 
        else
        {
            // Move to the next position in the input string.
            inputStringIndex  += returnValue;
            inputStringLength -= returnValue;

            // Convert ucs4 to ucs2
            returnValue = ucs2_wctomb(&data_[outputStringIndex], outputUCSCharacter, bufferSize_);

            if (returnValue < 0 )
                data_[outputStringIndex] = (UCS2Char)0x20;

            outputStringIndex++;
            outputBufferSize--;
        } // end - else

    } // end - while

    // If there is no characters which needs to be convert, insert NULL into the output.
    if (outputBufferSize)
        data_[outputStringIndex] = static_cast<UCS2Char>(NULL);

    length_ = outputStringIndex;
    /* 2008.07.31
     * no need to use dataString_ because Sunday support ustring directly.
     */
    // Set dataString_ 
    //convertString(dataString_, encodingType);

} // end - setUString()


// ---------------------------------------------[ convertString() ]

void UString::convertString(string& outputString, EncodingType encodingType) const
{
    outputString.clear();

    // Terminate converting if encoding type is not right value.
    if (encodingType == UString::UNKNOWN || encodingType == UString::TOTAL_ENCODING_TYPE_NO) {
        outputString = "";
        return;
    } // end - if

    int inputStringIndex  = 0;
    int inputStringLength = length();
    int outputStringIndex = 0;
	
    // Log 2009.02.10
    // Previous code
    // -----------------------------------------------------------
    // int outputBufferSize  = sizeof(UCS2Char) * bufferSize_ + 1;
    //
    // New code
    // -----------------------------------------------------------
    int outputBufferSize  = 3 * bufferSize_ + 1; // UTF-8 consumes a maximum of 3 bytes.

    //char* outputStringBuffer = new char[outputBufferSize + 1];
    char* outputStringBuffer = static_cast<char*>( malloc( (outputBufferSize+1) * sizeof(char) ) );

    unsigned int inputUCSCharacter;

    int returnValue;

    // Coninue converting characters one by one while there is no space in output buffer or there is no character to read.
    
    while (outputBufferSize > 0 && inputStringLength > 0)
    {
        // Convert ucs2 -> ucs4
        returnValue = ucs2_mbtowc(&inputUCSCharacter, &data_[inputStringIndex], inputStringLength);

        // If there is no matched character in converting table, insert space character to the output.
        if (returnValue < 0)
        {
            outputStringBuffer[outputStringIndex] = 0x20;
            outputStringIndex++;
            outputBufferSize--;
        } // end - if

        // If there is matched character in converting table, insert right converting character to the output. 
        else
        {
            // Using converting functions offered by iconv library. It will return byte size of converting character.
            returnValue = ConvertFunctionList[encodingType].convertFromUCS((unsigned char*)&outputStringBuffer[outputStringIndex], inputUCSCharacter, outputBufferSize);

            // If there is no matched character in converting table, insert space character to the output.
            switch (returnValue)
            {
            case RET_ILUNI:
                outputStringBuffer[outputStringIndex] = 0x20; // space character
                outputStringIndex++;
                outputBufferSize--;
                break;

            case RET_TOOSMALL: // If input string is shorter than output, stop converting.
                inputStringLength = 0;
                break;

            default:
                outputStringIndex += returnValue;
                outputBufferSize  -= returnValue;
            } // end - switch()

        } // end - else

        inputStringIndex++;
        inputStringLength--;

    } // end - while()
    
    // If there's a space to insert NULL charactor, insert NULL.
    if (outputBufferSize > 0)
        outputStringBuffer[outputStringIndex] = 0;
    
    outputString = (const char*)outputStringBuffer;

    free(outputStringBuffer);

} // end - convertString

// ---------------------------------------------[ subString() ]

UString& UString::subString(UString& outputUString, size_t pos) const
{
    unsigned int lengthOfOrigin = length();

    if ( outputUString.bufferSize_ < lengthOfOrigin -  pos + 1 )
    {
        outputUString.bufferSize_ = lengthOfOrigin -  pos + FLEXIBLE_FREE_SPACE;
        outputUString.data_ = static_cast<UCS2Char*>( 
            realloc( outputUString.data_, outputUString.bufferSize_ * sizeof(UCS2Char) ) );
        outputUString.data_[0] = static_cast<UCS2Char>(NULL);
    }

    outputUString.isLengthChanged_ = false;
    outputUString.length_ = lengthOfOrigin - pos;
    memcpy(outputUString.data_, &data_[pos], sizeof(UCS2Char)*(outputUString.length_));

    outputUString.data_[lengthOfOrigin - pos] = static_cast<UCS2Char>(NULL); // insert null

    return outputUString;
}

UString& UString::subString(UString& outputUString, size_t pos, size_t nChar) const
{
    unsigned int nCopy;

    unsigned int originLength = length();

    if ( (originLength - pos) < nChar)
    {
        nCopy = originLength - pos;
    }
    else
    {
        nCopy = nChar;
    }

    if ( outputUString.bufferSize_ < nCopy + 1 )
    {
        outputUString.bufferSize_ = nCopy + FLEXIBLE_FREE_SPACE; 
        outputUString.data_ = static_cast<UCS2Char*>( 
            realloc( outputUString.data_, outputUString.bufferSize_ * sizeof(UCS2Char) ) );
        outputUString.data_[0] = static_cast<UCS2Char>(NULL);
    }

    outputUString.isLengthChanged_ = false;
    outputUString.length_ = nCopy;
    memcpy(outputUString.data_, &data_[pos], sizeof(UCS2Char) * nCopy);
    outputUString.data_[nCopy] = static_cast<UCS2Char>(NULL); // insert null

    return outputUString;
}

bool UString::isGraphChar(int index) const
{
    return isThisGraphChar(data_[index]);
} // end - isGraphChar()

bool UString::isThisGraphChar(UCS2Char ucs2Char) 
{
    if ( UCS2_CHAR_GRAPH_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisGraphChar()

bool UString::isSpaceChar(int index) const
{
    return isThisSpaceChar(data_[index]);
} // end - isSpaceChar()

bool UString::isThisSpaceChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_SPACE_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisSpaceChar()

bool UString::isControlChar(int index) const
{
    return isThisControlChar(data_[index]);
} // end - isControlChar()

bool UString::isThisControlChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_CNTRL_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisControlChar()

bool UString::isPunctuationChar(int index) const
{
    return isThisPunctuationChar(data_[index]);
} // end - isPunctuationChar()

bool UString::isThisPunctuationChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_PUNCT_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisPunctuationChar()

bool UString::isAlphaChar(int index) const
{
    return isThisAlphaChar(data_[index]);
} // end - isAlphaChar()

bool UString::isThisAlphaChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_ALPHA_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisAlphaChar()

bool UString::isUpperChar(int index) const
{
    return isThisUpperChar(data_[index]);
} // end - isUpperChar()

bool UString::isThisUpperChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_UPPER_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisUpperChar()

bool UString::isLowerChar(int index) const
{
    return isThisLowerChar(data_[index]);
} // end - isLowerChar()

bool UString::isThisLowerChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_LOWER_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisLowerChar()


bool UString::isDigitChar(int index) const
{
    return isThisDigitChar(data_[index]);
} // end - isDigitChar()

bool UString::isThisDigitChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_DIGIT_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisDigitChar()

bool UString::isAlnumChar(int index) const
{
    return isThisAlnumChar(data_[index]);
} // end - isAlnumChar()

bool UString::isThisAlnumChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_ALNUM_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisAlnumChar()

bool UString::isXdigitChar(int index) const
{
    return isThisXdigitChar(data_[index]);
} // end - isXdigitChar()

bool UString::isThisXdigitChar(UCS2Char ucs2Char)
{
    if ( UCS2_CHAR_XDIGIT_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))) )
        return true;
    return false;
} // end - isThisXdigitChar()

UCS2CharType UString::charType(int index) const
{
    UCS2Char thisChar = data_[index];

    if (thisChar == 0)
        return UCS2_NULL;
    else if (thisChar >= (UCS2Char)0x0030 && thisChar <= (UCS2Char)0x0039) // DIGIT
        return UCS2_DIGIT;
    else if (thisChar >= (UCS2Char)0x0041 && thisChar <= (UCS2Char)0x005A) // ALPHA-UPPER
        return UCS2_ALPHA;
    else if (thisChar >= (UCS2Char)0x0061 && thisChar <= (UCS2Char)0x007A) // ALPHA-LOWER
        return UCS2_ALPHA;
    else if (thisChar >= (UCS2Char)0xAC00 && thisChar <= (UCS2Char)0xD7A3) // KOREAN
        return UCS2_KOREAN;

    return UCS2_UNDEF; // OTHERS
}

UCS2Char UString::toUpperChar(int index)
{

    UCS2Char thisChar = data_[index];

    if (thisChar >= 0x0061 && thisChar <= 0x007a) 
        thisChar -= 32;

    else if (thisChar == 0x00b5) 
        thisChar = 0x039c;

    else if (thisChar >= 0x00e0 && thisChar <= 0x00ff)
    {
        if (thisChar == 0x00ff) 
            thisChar = 0x0178;
        else if (thisChar != 0x00f7) 
            thisChar -= 32;
    }

    else if (thisChar >= 0x0561 && thisChar <= 0x0586) 
        thisChar -= 48;

    else if (thisChar >= 0x2170 && thisChar <= 0x217f) 
        thisChar -= 16;

    else if (thisChar >= 0x24d0 && thisChar <= 0x24e9) 
        thisChar -= 26;

    else if (thisChar >= 0xff41 && thisChar <= 0xff5a) 
        thisChar -= 32;

    else if (thisChar >= 0x0100 && thisChar <= 0x01ff)
        thisChar = UCS2_CHAR_LOWER2UPPER_PAGE01[thisChar-0x0100];

    else if (thisChar >= 0x0200 && thisChar <= 0x0292)
        thisChar = UCS2_CHAR_LOWER2UPPER_PAGE02[thisChar-0x0200];

    else if (thisChar >= 0x0300 && thisChar <= 0x03fb)
        thisChar = UCS2_CHAR_LOWER2UPPER_PAGE03[thisChar-0x0300];

    else if (thisChar >= 0x0400 && thisChar <= 0x050f)
        thisChar = UCS2_CHAR_LOWER2UPPER_PAGE04[thisChar-0x0400];

    else if (thisChar >= 0x1e00 && thisChar <= 0x1ff3)
        thisChar = UCS2_CHAR_LOWER2UPPER_PAGE1e[thisChar-0x1e00];

    data_[index] = thisChar; 

    return thisChar;
}

UCS2Char UString::toLowerChar(int index)
{

   UCS2Char thisChar = data_[index];
    
    if (thisChar >= 0x0041 && thisChar <= 0x005a) 
        thisChar += 32;

    else if (thisChar >= 0x00c0 && thisChar <= 0x00de)
    {
        if (thisChar != 0x00d7) thisChar +=32;
    }

    else if (thisChar >= 0x0531 && thisChar <= 0x0556) 
        thisChar += 48;

    else if (thisChar >= 0x2160 && thisChar <= 0x216f) 
        thisChar += 16;

    else if (thisChar >= 0x24b6 && thisChar <= 0x24cf) 
        thisChar += 26;

    else if (thisChar >= 0xff21 && thisChar <= 0xff3a) 
        thisChar += 32;

    else if (thisChar >= 0x0100 && thisChar <= 0x0232)
        thisChar = UCS2_CHAR_UPPER2LOWER_PAGE01[thisChar-0x0100];

    else if (thisChar>= 0x0300 && thisChar<= 0x03fa)
        thisChar = UCS2_CHAR_UPPER2LOWER_PAGE03[thisChar-0x0300];

    else if (thisChar>= 0x0400 && thisChar<= 0x050e)
        thisChar = UCS2_CHAR_UPPER2LOWER_PAGE04[thisChar-0x0400];

    else if (thisChar >= 0x1e00 && thisChar <= 0x1ffc)
        thisChar = UCS2_CHAR_UPPER2LOWER_PAGE1e[thisChar-0x1e00];

    else if (thisChar >= 0x2100 && thisChar<= 0x212b)
        thisChar= UCS2_CHAR_UPPER2LOWER_PAGE21[thisChar-0x2100];

    data_[index] = thisChar; 
    return thisChar;

} // end - toLowerChar()

void UString::toLowerString(void)
{
    for(unsigned int i = 0; i < length(); i++)
        toLowerChar(i);
}

// ---------------------------------------------[ find() ]

/******************************************************************************************
Added by Tuan-Quang, Nguyen, July 31st, 2008
Description: this function uses Sunday algorithm to find the occurrence of a pattern in the
string.
******************************************************************************************/
unsigned int UString::find(const UString&   patternString, unsigned int startOffset) const
{


	// implement Sunday algorithm
	// pattern length
	unsigned int patternLen = patternString.length();

    // If the size of the pattern is 0, then return NOT_FOUND
    if (patternLen == 0)
        return NOT_FOUND;
    
	// test length
	unsigned int textLen = length();
	unsigned int n, pos, j;
	std::map<UCS2Char, unsigned int> skipArray;

	if(patternLen > textLen)
		return NOT_FOUND;

	// generate skip array for Sunday algorithm
	skipArray.clear();
	for(n = 0; n < patternLen; n++)
	{
		skipArray[patternString[n]] = patternLen - n;
	}

	// searching for the pattern 
	pos = startOffset;

	// Due to the unsigned int value, we have to check two condition
	while(pos + patternLen < textLen && textLen - patternLen > pos)
	{
		j = 0;
		while( j < patternLen && data_[pos + j] == patternString[j])
		j++;

		if(j == patternLen)
			// pattern is found
			return pos;

		std::map<UCS2Char, unsigned int>::const_iterator it;
		it = skipArray.find(data_[pos + patternLen]);
		if(it == skipArray.end())
			pos += patternLen + 1;
		else
			pos += skipArray[data_[pos + patternLen]];
	}

	// for the last occurrence
	if(pos + patternLen == textLen)
	{
		j = 0;
		while( j < patternLen && data_[pos + j] == patternString[j])
			j++;

		if(j == patternLen)
			return pos;
	}

	return NOT_FOUND;
}


unsigned int UString::find(const string&   patternString, unsigned int startOffset, EncodingType encodingType) const
{
    UString patternUString(patternString, encodingType);

    return find(patternUString, startOffset);
} // end - find()


unsigned int UString::find(UCS2Char c, unsigned int startPosition) const
{
    for(unsigned int i = startPosition; i < length_; i++)
        if ( data_[i] == c ) 
            return i;

    // character isn't found.
    return NOT_FOUND; 
} // end - find()


// ---------------------------------------------[ format() ]

void UString::format(EncodingType encodingType, const char* formatString, ...)
{
    va_list argumentList; // argumentList contains ...
    va_start(argumentList, formatString);
    formatProcess(encodingType, 1024, formatString, argumentList);

} // end - format()


void UString::format(EncodingType encodingType, int maxBufferSize, const char* formatString, ...)
{
    va_list argumentList; // argumentList contains ...
    va_start(argumentList, formatString);
    formatProcess(encodingType, maxBufferSize, formatString, argumentList);
} // end - format()



// private function
void UString::formatProcess(EncodingType encodingType, int maxBufferSize, const char* formatString, va_list argumentList)
{
    //char* buffer = new char[maxBufferSize + 1]; // buffer contains result of the format. + 1 is a flexible number according to the OS.
    char* buffer = static_cast<char*>( malloc( (maxBufferSize + 1) * sizeof(char) ) );

    vsnprintf(buffer, maxBufferSize, formatString, argumentList);

    string bufferContainer; // bufferContainer is used for encoding UString.
    bufferContainer = buffer;

    //delete[] buffer;
    free(buffer);

    // If the length of string is longer than buffer size, increase the buffersize.
    if ( bufferContainer.length() >= bufferSize_ )
    {
        bufferSize_ = bufferContainer.length() + FLEXIBLE_FREE_SPACE;
        //free(data_);
        data_ = static_cast<UCS2Char*>( realloc( data_, bufferSize_ * sizeof(UCS2Char) ) );
    }

    setUString(bufferContainer, encodingType);
}
// ---------------------------------------------[ print() ]

void UString::displayStringValue(EncodingType encodingType, ostream& outputStream) const
{
    string output;
    convertString(output, encodingType);
    outputStream << output;
} // end - displayStringValue()


void UString::displayStringInfo(EncodingType encodingType, ostream& outputStream) const
{
    string output;
    convertString(output, encodingType);
    outputStream << endl << "data_ : (" << output << ")" << endl;
    outputStream << "NumOfChar_ : "<< length() << endl;
    outputStream << "bufferSize_ : " << bufferSize_ <<endl;
} // end - displayStringInfo()



// ---------------------------------------------[ clear() ]

void UString::clear(void)
{
    isLengthChanged_ = false;

    // Delete data area of UString.
    try {
        length_ = 0;
        bufferSize_ = UString::DEFAULT_SIZE;

        data_ = static_cast<UCS2Char*>( realloc( data_, bufferSize_ * sizeof(UCS2Char) ) );
        data_[0] = static_cast<UCS2Char>(NULL);
    }
    catch(UStringException ue)
    {
        ue.displayMessage();
    }
    catch(...)
    {
        cerr << "Unexpected Exception occurs" << endl;
        throw;
    }

}



// ---------------------------------------------[ empty() ]

bool UString::empty(void) const
{
    if(length() == 0) 
        return true;
    return false;
} // end - empty()

size_t UString:: size() const
{ 
    return (length() * sizeof(UCS2Char)); 
} // end - size()

unsigned int UString::bufferSize() const 
{ 
    return bufferSize_; 
} // end - bufferSize()

// ---------------------------------------------[ length() ]

unsigned int UString::length(void) const
{
    if (isLengthChanged_ == false)
    {
        return length_;
    }

    // Recalculate length
    length_ = 0;
    UCS2Char endOfString(0);
    while(data_[length_++] != endOfString);
    length_--;

    isLengthChanged_ = false;

    return length_;
} // end - length()


const char* UString::c_str() const 
{ 
    return reinterpret_cast<const char*>(data_); 
} // end - c_str()
// ---------------------------------------------[ convert Functions ]

void UString::copyStringFrom(const UString& sourceString)
{

    length_ = sourceString.length();

    // If the buffer size of this class is smaller than source string, increase memory allocation amount.
    if (bufferSize_ <= length_)
    {
        try {
            bufferSize_ = sourceString.length() + FLEXIBLE_FREE_SPACE; 
            data_ = static_cast<UCS2Char*>( realloc( data_, bufferSize_ * sizeof(UCS2Char) ) );
            data_[0] = static_cast<UCS2Char>(NULL);
        }
        catch (UStringException ue)
        {
            ue.displayMessage();
        }
        catch (...)
        {
            cerr << "Unexpected Exception occurs" << endl;
            throw;
        }

    } // end - if


    // Copy Source String into this class
    memcpy(data_, sourceString.data_, sizeof(UCS2Char) * length_);

    // Added by JunHui Hur, Nov 07 2008
    data_[length_] = static_cast<UCS2Char>(NULL);

} // end - copyStringFrom()


// ---------------------------------------------[ operators ]

int UString::compare (const UString& compareString) const
{
    if ( (*this) < compareString )
        return -1;
    else if ( (*this) == compareString )
        return 0;
    return 1;
} // end - compare()

const UCS2Char UString::at(size_t pos) const
{
    try {

        if( pos > bufferSize_ )
        {
            throw UStringException("Out of Bound");
        }
        return data_[pos];
    } 
    catch(UStringException ue) {
        ue.displayMessage();
        return -1;
    }
    catch(...) {
        cerr << "Unexpected Exception occurs" << endl;
        throw;
    }
} // end - at()
const UCS2Char& UString::operator[] (size_t pos) const
{
    if ( (pos) > bufferSize_ )
    {
        bufferSize_ = pos + FLEXIBLE_FREE_SPACE;
        data_ = static_cast<UCS2Char*>( realloc( data_, bufferSize_ * sizeof(UCS2Char) ) );
        data_[pos] = static_cast<UCS2Char>(NULL);
    } // end - if 

    return data_[pos];   
} // end - operator[] const

UCS2Char& UString::operator[] (size_t pos)
{
    isLengthChanged_ = true;
    return const_cast<UCS2Char&>( static_cast<const UString&>(*this)[pos] );
} // end - operator[]

UString& UString::operator=  (const UString& assignString)
{
    if (this == &assignString) 
        return *this;
    copyStringFrom(assignString);
    return *this;
}

UString& UString::operator+= (const UString& appendString)
{
    unsigned int appendStringLength = appendString.length();
    unsigned int thisLength = length();
    unsigned int newLength = appendStringLength + thisLength;

    // If free space is not enough to append given string, reallocate data_ area.
    if ( bufferSize_ <= newLength )
    {
        while( bufferSize_ <= newLength )
        {
            if ( bufferSize_ < 65536 )
            {
                bufferSize_ = (bufferSize_ * bufferSize_) + FLEXIBLE_FREE_SPACE;
                continue;
            } // end - if
            else 
            {
                bufferSize_ =  MAXIMUM_INDEX_SIZE;
                newLength = bufferSize_ - 1;
                appendStringLength = newLength - thisLength;
                break;
            } // end - else
        } // end - while()

        data_ = static_cast<UCS2Char*>( realloc( data_, bufferSize_ * sizeof(UCS2Char) ) );

    } // end - if

    // copy appendString into the back of UString.
    
    // Modified 2008-12-30
    // -------------------
    //for(unsigned int i = 0; i < appendStringLength; i++)
    //{
    //    data_[i + thisLength] = appendString[i];
    //}
    memcpy(&data_[thisLength], appendString.data_, appendStringLength * sizeof(UCS2Char)); 

    data_[newLength] = static_cast<UCS2Char>(NULL); // append NULL to the end of string
    length_ = newLength;
    isLengthChanged_ = false;

    return *this;

} // end - operator +=

UString& UString::operator+= (const UCS2Char& appendChar)
{
    unsigned int thisLength = length();

    // If free space is not enough to append given string, reallocate data_ area.
    if ( (bufferSize_ == thisLength + 1)) // one byte for NULL
    {
        // Modified by dohyun 2008-11-18
        // ------------------------------
        //
        //bufferSize_ += FLEXIBLE_FREE_SPACE;
        if ( bufferSize_ < 65536 )
            bufferSize_ = (bufferSize_ * bufferSize_) + FLEXIBLE_FREE_SPACE;
        else
            bufferSize_ =  MAXIMUM_INDEX_SIZE;

        //UCS2Char *tmpData_ = new UCS2Char [bufferSize_];
        //UCS2Char *tmpData_ = static_cast<UCS2Char*>( malloc( bufferSize_ * sizeof(UCS2Char) ) );

        //memcpy(tmpData_, data_, sizeof(UCS2Char) * length());
        //tmpData_[thisLength] = appendChar;

		// Modified by TuanQuang Nguyen, Nov 03 2008
        // old code: delete data_;
        //
        //delete [] data_;
        //free(data_);

        data_ = static_cast<UCS2Char*>( realloc( data_, bufferSize_ * sizeof(UCS2Char) ) );
    } // end - if

    // copy appendString into the back of UString.
    data_[thisLength] = appendChar;

    data_[thisLength + 1] = static_cast<UCS2Char>(NULL); // append NULL to the end of string

    length_ = thisLength + 1;
    isLengthChanged_ = false;

    return *this;
} // end - operator +=

bool UString::operator== (const UString& compareString) const
{

    if (length() != compareString.length()) {
        return false;
    }

    for(unsigned int i = 0; i < length_; i++)
    {
        if (data_[i] != compareString[i]) 
        {
            return false;
        }
    }

    return true;
} // end - operator==

bool UString::operator!= (const UString& compareString) const
{
    return !( (*this) ==compareString );
} // end - operator !=

bool UString::operator< (const UString& compareString) const
{
    unsigned int thisLength, compareLength;
    thisLength = (*this).length();
    compareLength = compareString.length();
    
    if ( thisLength < compareLength )
        return true;
    else if ( thisLength > compareLength )
        return false;
    else
    { // if ( thisLength == compareLength )

        // Compare characters one by one.
        for(unsigned int i = 0; i < thisLength; i++)
        {
            if ( (*this)[i] < compareString[i] )
                return true;
            else if ( (*this)[i] > compareString[i] )
                return false;
        } // end - for

    } // end - else

    return false; // Exact match
} // end - operator<


// UString Tools
///////////////////////////////////////////////////////////

namespace ustring_tool {

    //
    // @brief This function reads text from file and stores line by line according to the
    // encoding type.
    //
    // @param encodingType  Encoding type of the file.
    // @param filename      file name string with it's path.
    // @param lines         output UString which stores lines of given file.
    //
    void getUStringLinesFromFile( UString::EncodingType  encodingType, 
            const std::string& filename, std::vector<UString>& lines )
    {
        // clear output data
        lines.clear();

        // open file and check if it is opened
        std::ifstream fpin( filename.c_str() );
        if ( !fpin.is_open() )
        {
            cerr << "Cannot open " << filename << endl;
            return;
        } // end - if

        char ch;
        string buffer;
        while( fpin.good() )
        {
            fpin.get( ch );
            if ( ch == '\n' ) // get one line
            {
                if ( buffer.size() == 0 ) break;

                UString insertUString( buffer, encodingType );
                lines.push_back( insertUString );

                buffer.clear();
            } // end - if
            else if( ch == '\r' )
                continue;
            else 
                buffer.push_back(ch);
        } // end - while

    } // end - getUStringLinesFromFile()

    // 
    // @brief This interface tokenizes string into several tokens.
    //
    // @param encodingType  Encoding type of delimiter.
    // @param delimiter     delimiter which is used to tokenize UString.
    // @param srcUString    source UString which is tokenized.
    // @param tokens        output which contains tokens.
    // 
    void getTokensFromUString( UString::EncodingType encodingType, const char delimiter, 
            const UString& srcUString, std::vector<UString>& tokens )
    {
        // clear output data
        tokens.clear();

        string delimiterString; delimiterString.push_back( delimiter );
        UString delimiterUString(delimiterString, encodingType);
        UCS2Char delChar = delimiterUString[0];

        unsigned int start = 0;
        unsigned int num = 0;
        unsigned int i;

        for( i = 0; i < srcUString.length(); i++)
        {
            if ( srcUString[i] == delChar )
            {
                num = i - start;
                UString token;
                srcUString.subString( token, start, num );
                tokens.push_back( token );
                start = i + 1;
            } // end - if
        } // end - for

        if ( start != i )
        {
            num = i - start;
            UString token;
            srcUString.subString( token, start, num );
            tokens.push_back( token );
            start = i + 1;
        } // end - if

    } // end - getTokensFromUString()

} // end - namespace ustring_stuff

} // end - namespace sf1lib
