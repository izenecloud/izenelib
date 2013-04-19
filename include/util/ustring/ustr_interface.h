/**
   @file ustr_interface.h
   @author Kevin Hu
   @date 2009.11.24
 */

/**
 * @brief an enumerator which offers encoding type of the string.
 */
enum EncodingType
{
    UNKNOWN = 0,
    UTF_8, // unicode
    EUC_KR, CP949, // korean
    EUC_JP, SJIS, // japanese
    GB2312, BIG5, // chinese
    ISO8859_15, // latin
    TOTAL_ENCODING_TYPE_NO // end of charset
};

/// a string which offers encoding type of the string.
static const char EncodingTypeString[10][30];
// Unicode character information table : Each value contains 8 characters' information(Because its data type is unsigned char - 8bits) according to the table name.
// For example, UCS2_CHAR_GRAPH_TABLE determines if this unicode character is a graphcal character or not. So UCS2_CHAR_GRAPH_TABLE[0] covers unocode characters from
// 0x0000 to 0x000f. To use this table, you should divide unicode character value into 8 finding block in table. After getting the value of the table, do AND operation
// between the result of the table and (1 << (7 - (unicode character value % 8))) which indicates the exact character in block. These tables make it possible to reduce

static const char          UCS2_CJAMO_CVT_TABLE[52];

static const unsigned char UCS2_CHAR_GRAPH_TABLE [8192]; ///< UCS2 character information table which determines if it is the graphcal     character or not.
static const unsigned char UCS2_CHAR_SPACE_TABLE [8192]; ///< UCS2 character information table which determines if it is the spacial      character or not.
static const unsigned char UCS2_CHAR_CNTRL_TABLE [8192]; ///< UCS2 character information table which determines if it is the control      character or not.
static const unsigned char UCS2_CHAR_PUNCT_TABLE [8192]; ///< UCS2 character information table which determines if it is the punctuation  character or not.
static const unsigned char UCS2_CHAR_LANG_TABLE [8192];  ///< UCS2 character information table which determines if it is the language     character or not.
static const unsigned char UCS2_CHAR_UPPER_TABLE [8192]; ///< UCS2 character information table which determines if it is the upper        character or not.
static const unsigned char UCS2_CHAR_LOWER_TABLE [8192]; ///< UCS2 character information table which determines if it is the lower        character or not.
static const unsigned char UCS2_CHAR_DIGIT_TABLE [8192]; ///< UCS2 character information table which determines if it is the digital      character or not.
static const unsigned char UCS2_CHAR_ALNUM_TABLE [8192]; ///< UCS2 character information table which determines if it is the alpha or num character or not.
static const unsigned char UCS2_CHAR_XDIGIT_TABLE[8192]; ///< UCS2 character information table which determines if it is the hexadecimal  character or not.

// Unicode alphabet character converting tables
static const UCS2Char UCS2_CHAR_UPPER2LOWER_PAGE01[307]; ///< UCS2 alphabet converting table from upper to lower which covers UCS2 characters from 0x0100 to 0x0232.
static const UCS2Char UCS2_CHAR_UPPER2LOWER_PAGE03[251]; ///< UCS2 alphabet converting table from upper to lower which covers UCS2 characters from 0x0300 to 0x03fa.
static const UCS2Char UCS2_CHAR_UPPER2LOWER_PAGE04[271]; ///< UCS2 alphabet converting table from upper to lower which covers UCS2 characters from 0x0400 to 0x050e.
static const UCS2Char UCS2_CHAR_UPPER2LOWER_PAGE1e[509]; ///< UCS2 alphabet converting table from upper to lower which covers UCS2 characters from 0x1e00 to 0x1ffc.
static const UCS2Char UCS2_CHAR_UPPER2LOWER_PAGE21[44]; ///< UCS2 alphabet converting table from upper to lower which covers UCS2 characters from 0x2100 to 0x212b.

static const UCS2Char UCS2_CHAR_LOWER2UPPER_PAGE01[256]; ///< UCS2 alphabet converting table from lower to upper which covers UCS2 characters from 0x0100 to 0x01ff.
static const UCS2Char UCS2_CHAR_LOWER2UPPER_PAGE02[147]; ///< UCS2 alphabet converting table from lower to upper which covers UCS2 characters from 0x0200 to 0x0292.
static const UCS2Char UCS2_CHAR_LOWER2UPPER_PAGE03[252]; ///< UCS2 alphabet converting table from lower to upper which covers UCS2 characters from 0x0300 to 0x03fb.
static const UCS2Char UCS2_CHAR_LOWER2UPPER_PAGE04[272]; ///< UCS2 alphabet converting table from lower to upper which covers UCS2 characters from 0x0400 to 0x050f.
static const UCS2Char UCS2_CHAR_LOWER2UPPER_PAGE1e[500]; ///< UCS2 alphabet converting table from lower to upper which covers UCS2 characters from 0x1e00 to 0x1ff3.

static const ConvertFunction
ConvertFunctionList[vector_string::TOTAL_ENCODING_TYPE_NO];
/**
 * @brief a static converting function which convert encoding type string into encoding type enumerator.
 *
 * @param encodingTypeString a string which indicates the encoding type.
 * @return enumerated value of encoding type.
 */
//static EncodingType convertEncodingTypeFromStringToEnum(
//const char* encodingTypeString);
typedef Algorithm<SelfT> algo;

public:

static const int DEFAULT_SIZE = 10; ///< a default size of memory.
static const int FLEXIBLE_FREE_SPACE = 10; ///< a default size of memory.
static const size_t NOT_FOUND = -1; ///< a return value of find() when the pattern is not found.
static const size_t MAXIMUM_INDEX_SIZE = -2; ///< Maximum size of index. It is used

protected:
EncodingType systemEncodingType_; // Log : 2009.07.23
public:
/**
 * @brief a constructor which initializes string with encoded string class.
 * @param initString    a string class to initialize UString.
 * @param encodingType  an enumerated type which indicates encoding type.
 */
inline vector_string(const std::string& initString, EncodingType encodingType)
    :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false), systemEncodingType_(UNKNOWN) // Log : 2009.07.23
{
    assign(initString, encodingType);
} // end - UString()

/**
 * @brief a constructor which initializes string with encoded string class.
 *
 * @param initString    a const char array to initialize UString.
 * @param encodingType  an enumerated type which indicates encoding type.
 */
inline vector_string(const char* initString, EncodingType encodingType)
    :length_(0), p_(NULL), str_(NULL), max_size_(0), is_attached_(false),systemEncodingType_(UNKNOWN) // Log : 2009.07.23
{
    assign(initString, encodingType);
} // end - UString()



//----------------------------------------[ Member Functions ]

public:

/**
 * @brief an interface function which determines if indexed UCS2 character is a
 * graphical character. A graphical character is a character which is visible on the screen.
 *
 * @param index     an index of UString data.
 * @return true     This character is a graphical character.
 * @return false    This character is not a graphical character.
 */
inline bool isGraphChar(size_t index) const
{
    return isThisGraphChar(str_[index]);
} // end - isGraphChar()

/**
 * @brief an static interface function which determines if given UCS2 character is a
 * graphical character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a graphical character.
 * @return false    This character is not a graphical character.
 */
static inline  bool isThisGraphChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_GRAPH_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisGraphChar()

/**
 * @brief an interface function which determines indexed UCS2 character is a spatial character.
 * Space, Tab, Newline, and, other spatial characters are included in it.
 *
 * @param index     an index of UString data.
 * @return true     This character is a spatial character.
 * @return false    This character is not a spatial character.
 */
inline bool isSpaceChar(int index) const
{
    assert((size_t)index<length_);
    return isThisSpaceChar(str_[index]);
} // end - isSpaceChar()


/**
 * @brief an static interface function which determines given UCS2 character is a spatial character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a spatial character.
 * @return false    This character is not a spatial character.
 */
static inline bool isThisSpaceChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_SPACE_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisSpaceChar()

/**
 * @brief an interface function which determines indexed UCS2 character is a control character. ^Z, ^T, and other control characters are included in it.
 *
 * @param index     an index of UString data.
 * @return true     This character is a control character.
 * @return false    This character is not a control character.
 */
inline bool isControlChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisControlChar(str_[index]);
} // end - isControlChar()

/**
 * @brief an static interface function which determines given UCS2 character is a control character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a control character.
 * @return false    This character is not a control character.
 */
static inline bool isThisControlChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_CNTRL_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisControlChar()


/**
 * @brief an interface function which determines indexed UCS2 character is a punctuation character. All graphical characters except for Alphabet & Numeric characters are punctuation character. For example, ( ! + = < ? and many other punctuation characters are included in it.
 *
 * @param index     an index of UString data.
 * @return true     This character is a punctuation mark.
 * @return false    This character is not a punctuation mark.
 */
inline bool isPunctuationChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisPunctuationChar(str_[index]);
} // end - isPunctuationChar()


/**
 * @brief an static interface function which determines given UCS2 character is a punctuation character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a punctuation character.
 * @return false    This character is not a punctuation character.
 */
static inline bool isThisPunctuationChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_PUNCT_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisPunctuationChar()

/**
 * @brief an interface to determine given character is korean character.
 */
bool isKoreanChar(int index) const
{
    assert((size_t)index<length_);
    return isThisKoreanChar(str_[index]);
} // end - isKoreanChar()

static bool isThisKoreanChar(UCS2Char ucs2Char)
{
    if ( ucs2Char < 0x1100 )
        return false;
    if ( ucs2Char > 0xD7AF )
        return false;
    if ( 0x11ff < ucs2Char && ucs2Char < 0xAC00 )
        return false;

    return true;
} // end - isThisKoreanChar()

/**
 * @brief an interface to determine given character is modern compatibility jamo of korean.
 * @author Dohyun Yun
 * @param[index]    An index which represent the location in str_.
 * @return true     if given character is modern compatibility java.
 * @return false    else.
 */
bool isModernCJamo(int index) const   // CJamo = Compatibility Jamo
{
    assert((size_t)index<length_);
    return isThisModernCJamo(str_[index]);
} // end - isModernCJamo()

/**
 * @brief an static interface to determine given character is modern compatibility jamo of korean.
 * @author Dohyun Yun
 * @param[ucs2Char] A UCS2 character value.
 * @return true     if given character is modern compatibility java.
 * @return false    else.
 */
static bool isThisModernCJamo(UCS2Char ucs2Char)
{
    if ( ucs2Char < 0x3131 )
        return false;
    if ( ucs2Char > 0x3163 )
        return false;
    return true;
} // end - isThisModernCJamo()

/**
 * @brief an interface to determine given character is korean jamo.
 * @author Dohyun Yun
 */
bool isJamo(int index) const
{
    assert((size_t)index<length_);
    return isThisJamo(str_[index]);
} // end - isJamo()

/**
 * @brief a static interface to determine given character is korean jamo.
 * @author Dohyun Yun
 */
static bool isThisJamo(UCS2Char ucs2Char)
{
    if ( ucs2Char < 0x1100 || ucs2Char > 0x11FF )
        return false;
    return true;
} // end - isThisJamo()

/**
 * @brief an interface to determine given character is choseung of korean jamo.
 * @author Dohyun Yun
 */
bool isChoseong(int index) const
{
    assert((size_t)index<length_);
    return isThisChoseong(str_[index]);
} // end - isChoseong()

/**
 * @brief a static interface to determine given character is choseung of korean jamo.
 * @author Dohyun Yun
 */
static bool isThisChoseong(UCS2Char ucs2Char)
{
    if ( ucs2Char < 0x1100  || ucs2Char > 0x115F )
        return false;
    return true;
} // end - isThisChoseong();

/**
 * @brief an interface to determine given character is jungseung of korean jamo.
 * @author Dohyun Yun
 */
bool isJungseong(int index) const
{
    assert((size_t)index<length_);
    return isThisJungseong(str_[index]);
} // end - isJungseong()

/**
 * @brief a static interface to determine given character is jungseung of korean jamo.
 * @author Dohyun Yun
 */
static bool isThisJungseong(UCS2Char ucs2Char)
{
    if ( ucs2Char < 0x1161  || ucs2Char > 0x11A7 )
        return false;
    return true;
} // end - isThisJungseong();

/**
 * @brief an interface to determine given character is jongseung of korean jamo.
 * @author Dohyun Yun
 */
bool isJongseong(int index) const
{
    assert((size_t)index<length_);
    return isThisJongseong(str_[index]);
} // end - isJongseong()

/**
 * @brief a static interface to determine given character is jongseung of korean jamo.
 * @author Dohyun Yun
 */
static bool isThisJongseong(UCS2Char ucs2Char)
{
    if ( ucs2Char < 0x11A8  || ucs2Char > 0x11FF )
        return false;
    return true;
} // end - isThisJongseong();


/**
 * @brief an interface to determine given character is Chinese character.
 */
bool isChineseChar(int index) const
{
    assert((size_t)index<length_);
    return isThisChineseChar(str_[index]);
} // end - isKoreanChar()

//@author wang qian
bool isAllChineseChar() const
{
    int index=0;
    while((size_t)index<length_)
    {

        if(!isThisChineseChar(str_[index]))
        {
            return false;
        }
        index++;
    }
    return true;
}

bool includeChineseChar() const
{
    int index=0;
    while((size_t)index<length_)
    {

        if(isThisChineseChar(str_[index]))
        {
            return true;
        }
        index++;
    }
    return false;
}
//....
static bool isThisChineseChar(UCS2Char ucs2Char)
{
    if ((ucs2Char>=0x2E80 && ucs2Char<=0x2EF3)
            || (ucs2Char>=0x2F00 && ucs2Char<=0x2FD5)
            || (ucs2Char>=0x3400 && ucs2Char<=0x4DB5)
            || (ucs2Char>=0x4E00 && ucs2Char<=0x9FC3)
            || (ucs2Char>=0xF900 && ucs2Char<=0xFAD9))
        return true;

    return false;
} // end - isThisChineseChar()

bool includeChar(UCS2Char ucs2Char) const
{
    //有bug
    int index=0;
    while((size_t)index<length_)
    {
        if(isThisChineseChar(str_[index]))
        {
            if(str_[index]==ucs2Char)
                return true;
        }
        index++;
    }
    return false;
}

bool match(const SelfT &compareUString) const
{
    int index=0;
    while((size_t)index<length())
    {
        if(isChineseChar(index))
        {
            if(!compareUString.includeChar(str_[index]))
                return false;
        }
        index++;
    }
    return true;
}

bool MatchDegree(const SelfT &compareUString) const
{
    int index=0;
    int rightNum=0,wrongNum=0;
    while((size_t)index<length())
    {
        if(isChineseChar(index))
        {
            if(!compareUString.includeChar(str_[index]))
            {
                wrongNum++;
            }
            else
            {
                rightNum++;
            }
        }
        index++;
    }
    if(rightNum>=2*wrongNum)
        return true;
    else
        return false;
}


bool filter(std::vector<std::pair<SelfT,uint32_t> >& filterList) const
{
    //TODO
    //int index=0;
    if(filterList.empty()) return false;
    else
    {
        typename std::vector<std::pair<SelfT,uint32_t> >::iterator iter=filterList.begin();
        // int k=0;
        //for (uint32_t j = 0; j < filterList.size(); j++,k++)
        for (iter=filterList.begin(); iter!=filterList.end(); )
        {
            if( !match((*iter).first))
            {
                iter=filterList.erase(iter);
                // k=j-1;

            }
            else
                iter++;
        }
        //    if( !match((*(filterList.end())).first))
        //     {filterList.erase(iter);
        //    }
        //free(iter);
        return true;
    }
}

typedef boost::tuple <SelfT,uint32_t,uint32_t> Tuple;

struct Myclasscmp
{
    static bool Compare (const Tuple& Tuple1,const Tuple& Tuple2)
    {
        return Tuple1.template get<2>() < Tuple2.template get<2>();
    }

};

struct Myclassequal
{
    static bool Compare (const Tuple& Tuple1,const Tuple& Tuple2)
    {
        return Tuple1.template get<0>() == Tuple2.template get<0>();
    }
};

void KeepOrderDuplicateFilter(std::vector<std::pair<SelfT,uint32_t> >& filterList) const
{

    std::vector<Tuple> list;
    //list.resize(filterList);
    for (uint32_t j = 0; j < filterList.size(); j++)
    {
        Tuple tempTuple(filterList[j].first,filterList[j].second,j);
        list.push_back(tempTuple);
    }

    sort(list.begin(), list.end()) ;
    typename std::vector<Tuple>::iterator pos;
    pos = std::unique(list.begin(), list.end(), Myclassequal::Compare);
    list.erase(pos, list.end());
    filterList.clear();
    std::sort(list.begin(), list.end(),Myclasscmp::Compare) ;

    for (uint32_t j = 0; j < list.size(); j++)
    {
        std::pair<SelfT,uint32_t> tempPair(list[j].template get<0>(),list[j].template get<1>());
        filterList.push_back(tempPair);
    }

}

bool FuzzyFilter(std::vector<std::pair<SelfT,uint32_t> >& filterList) const //TODO
{
// int index=0;
    if(filterList.empty())
    {
        return false;
    }
    else
    {
        typename std::vector<std::pair<SelfT,uint32_t> >::iterator iter=filterList.begin();
        // int k=0;
        //for (uint32_t j = 0; j < filterList.size(); j++,k++)
        for (iter=filterList.begin(); iter!=filterList.end(); )
        {
            if( !MatchDegree((*iter).first))
            {
                iter=filterList.erase(iter);
                // k=j-1;

            }
            else
                iter++;
        }
        //    if( !match((*(filterList.end())).first))
        //     {filterList.erase(iter);
        //    }
        //free(iter);
        return true;
    }

}



bool filter(std::vector<SelfT>& filterList) const //TODO
{
    int index=0;
    if(filterList.empty())
    {
        return false;
    }
    else
    {
        typename std::vector<SelfT>::iterator iter=filterList.begin();
        int k=0;
        //for (uint32_t j = 0; j < filterList.size(); j++,k++)
        for (iter=filterList.begin(); iter!=filterList.end(); )
        {
            if( !match((*iter)))
            {
                iter=filterList.erase(iter);
                // k=j-1;

            }
            else
                iter++;
        }
        //    if( !match((*(filterList.end())).first))
        //     {filterList.erase(iter);
        //    }
        //free(iter);
        return true;
    }

}
//........
/**
 * @brief an interface to determine given character is Chinese character.
 */
bool isJapaneseChar(int index) const
{
    assert((size_t)index<length_);
    return isThisJapaneseChar(str_[index]);
} // end - isKoreanChar()

static bool isThisJapaneseChar(UCS2Char ucs2Char)
{
    if ((ucs2Char>=0x3041 && ucs2Char<=0x309F)
            || (ucs2Char>=0x30A1 && ucs2Char<=0x30FF)
            || (ucs2Char>=0x31F0 && ucs2Char<=0x31FF)
            || (ucs2Char>=0xFF66 && ucs2Char<=0xFF9F))
        return true;

    return false;
} // end - isThisJapaneseChar()

/**
 * @brief an interface function which determines indexed UCS2 character is a numeric character.
 *
 * @param index     an index of UString data.
 * @return true     This character is a numeric character.
 * @return false    This character is not a numeric character.
 */
inline bool isNumericChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisNumericChar(str_[index]);
} // end - isNumericChar()

/**
 * @brief an static interface function which determines given UCS2 character is a numeric character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a numeric character.
 * @return false    This character is not a numeric character.
 */
static inline bool isThisNumericChar(UCS2Char ucs2Char)
{
    static const UCS2Char zero('0'), nine('9');
    if ( zero <= ucs2Char && ucs2Char <= nine )
        return true;
    return false;
} // end - isThisNumericChar()

/**
 * @brief an interface function which determines indexed UCS2 character is a alphabet character.
 *
 * @param index     an index of UString data.
 * @return true     This character is a alphabet character.
 * @return false    This character is not a alphabet character.
 */
inline bool isAlphaChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisAlphaChar(str_[index]);
} // end - isAlphaChar()

/**
 * @brief an static interface function which determines given UCS2 character is a alphabet character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a alphabet character.
 * @return false    This character is not a alphabet character.
 */
static inline bool isThisAlphaChar(UCS2Char ucs2Char)
{
    static const UCS2Char a('a'), z('z'), A('A'), Z('Z');
    if ( ( a <= ucs2Char && ucs2Char <= z ) || ( A <= ucs2Char && ucs2Char <= Z ) )
        return true;
    return false;
} // end - isThisAlphaChar()

/**
 * @brief an interface function which determines indexed UCS2 character is a language character.
 *
 * @param index     an index of UString data.
 * @return true     This character is a language character.
 * @return false    This character is not a language character.
 */
inline bool isLanguageChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisLanguageChar(str_[index]);
} // end - isLanguageChar()


/**
 * @brief an static interface function which determines given UCS2 character is a language character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a language character.
 * @return false    This character is not a language character.
 */
static inline bool isThisLanguageChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_LANG_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisLanguageChar()


/**
 * @brief an interface function which determines indexed UCS2 character is a upper alphabet character.
 *
 * @param index     an index of UString data.
 * @return true     This character is a upper alphabet character.
 * @return false    This character is not a upper alphabet character.
 */
inline bool isUpperChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisUpperChar(str_[index]);
} // end - isUpperChar()


/**
 * @brief an static interface function which determines given UCS2 character is a upper alphabet character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a upper alphabet character.
 * @return false    This character is not a upper alphabet character.
 */
static inline bool isThisUpperChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_UPPER_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisUpperChar()


/**
 * @brief an interface function which determines indexed UCS2 character is a lower alphabet character.
 *
 * @param index     an index of UString data.
 * @return true     This character is a lower alphabet character.
 * @return false    This character is not a lower alphabet character.
 */
inline bool isLowerChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisLowerChar(str_[index]);
} // end - isLowerChar()


/**
 * @brief an static interface function which determines given UCS2 character is a lower alphabet character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a lower alphabet character.
 * @return false    This character is not a lower alphabet character.
 */
static inline bool isThisLowerChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_LOWER_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisLowerChar()


/**
 * @brief an interface function which determines indexed UCS2 character is a digit character.
 *
 * @param index     an index of UString data.
 * @return true     This character is a digit character.
 * @return false    This character is not a digit character.
 */
inline bool isDigitChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisDigitChar(str_[index]);
} // end - isDigitChar()


/**
 * @brief an static interface function which determines given UCS2 character is a digit character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a digit character.
 * @return false    This character is not a digit character.
 */
static inline bool isThisDigitChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_DIGIT_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisDigitChar()


/**
 * @brief an interface function which determines indexed UCS2 character is a alphabet or digit character.
 *
 * @param index     an index of UString data.
 * @return true     This character is a alphabet or digit character.
 * @return false    This character is not a alphabet or digit character.
 */
inline bool isAlnumChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisAlnumChar(str_[index]);
} // end - isAlnumChar()


/**
 * @brief an static interface function which determines given UCS2 character is a alphabet or digit character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a alphabet or digit character.
 * @return false    This character is not a alphabet or digit character.
 */
static inline bool isThisAlnumChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_ALNUM_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisAlnumChar()

/**
 * @brief an interface function which determines indexed UCS2 character is a hexa-decimal character.
 *
 * @param index     an index of UString data.
 * @return true     This character is a hexa-decimal character.
 * @return false    This character is not a hexa-decimal character.
 */
inline bool isXdigitChar(size_t index) const
{
    assert((size_t)index<length_);
    return isThisXdigitChar(str_[index]);
} // end - isXdigitChar()

/**
 * @brief an static interface function which determines given UCS2 character is a hexa-decimal character.
 * You can refer comments of UCS2Table.h to get the information of the table.
 *
 * @param ucs2Char  an UCS2 type character.
 * @return true     This character is a hexa-decimal character.
 * @return false    This character is not a hexa-decimal character.
 */
static inline bool isThisXdigitChar(UCS2Char ucs2Char)
{
    //ucs2Char = ucs2Char>>8;
    if (UCS2_CHAR_XDIGIT_TABLE[ int(ucs2Char/8) ] & (1 << (7 - (ucs2Char % 8))))
        return true;
    return false;
} // end - isThisXdigitChar()

/**
 * @brief an interface function which determines type of given character.
 *
 * @param  index        an index of UString data.
 * @return UCS2_NULL    NULL character.
 * @return UCS2_UNDEF   Undefined character.
 * @return UCS2_DIGIT   Numeric character.
 * @return UCS2_ALPHA   Alphabet character.
 * @return UCS2_KOREAN  Korean character.
 */
inline UCS2CharType charType(size_t index) const
{
    assert((size_t)index<length_);
    UCS2Char thisChar = str_[index];//>>8;

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
} // end - charType()

/**
 * @brief an interface function which changes lower alphabet into upper one.
 *
 * @param index     an index of UString data.
 * @return          the result value of converting.
 */
UCS2Char toUpperChar(size_t index)
{

    assert((size_t)index<length_);
    UCS2Char thisChar = str_[index];//>>8;

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

    //thisChar =  thisChar<<8;

    str_[index] = thisChar;

    return thisChar;
} // end - toUpperChar

/**
 * @brief an interface function which changes upper alphabet string into upper one.
 */
void toUpperString(void)
{
    for (unsigned int i = 0; i < length(); i++)
        toUpperChar(i);
} // end - toLowerString()

static inline UCS2Char toLowerChar( UCS2Char thisChar)
{
    if (thisChar >= 0x0041 && thisChar <= 0x005a)
        thisChar += 32;

    else if (thisChar >= 0x00c0 && thisChar <= 0x00de)
    {
        if (thisChar != 0x00d7)
            thisChar +=32;
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

    return thisChar;
}

/**
 * @brief an interface function which changes upper alphabet into lower one.
 *
 * @param index     an index of UString data.
 * @return          the result value of converting.
 */
UCS2Char toLowerChar(size_t index)
{

    assert((size_t)index<length_);
    str_[index] = vector_string::toLowerChar(str_[index]);
    return str_[index];

} // end - toLowerChar()

/**
 * @brief an interface function which changes upper alphabet string into lower one.
 * @return true if output string is different with input string.
 */
static bool toLowerString(const CharT* const inputString, const size_t inputStringLength,
                          CharT* const outputBuffer, const size_t outputBufferLimit)
{
    bool ret = false;
    for(size_t i = 0; i < inputStringLength && i < outputBufferLimit; i++ )
    {
        outputBuffer[i] = toLowerChar( inputString[i] );
        if(outputBuffer[i] != inputString[i]) ret = true;
    }
    if(outputBufferLimit < inputStringLength) ret = true;
    return ret;
}

/**
 * @brief an interface function which changes upper alphabet string into lower one.
 */
void toLowerString(void)
{
    for (unsigned int i = 0; i < length(); i++)
        toLowerChar(i);
} // end - toLowerString()


/**
 * @brief an interface function which offers the first location of matched pattern at the start position in UString.
 *
 * @param patternString a string which is a source to find the location in UString data.
 * @param startOffset   a position where the find operation will be started.
 * @param encodingType  an enumerated type which indicates encoding type.
 */
inline size_t find(const std::string& patternString,
                   unsigned int startOffset, EncodingType encodingType) const
{

    SelfT patternUString(patternString, encodingType);

    return find(patternUString, startOffset);
} // end - find()

/* /\** */
/*  * @brief an interface function which offers the first location of matched character at the start position in UString. */
/*  * */
/*  * @param c             a character which is a source to find the location in UString data. */
/*  * @param startPosition a position where the find operation will be started.   */
/*  *\/ */
/* size_t find(UCS2Char c, unsigned int startPosition = 0) const */
/* { */
/*   for (size_t i = startPosition; i < length_; i++) */
/*     if (str_[i] == c) */
/*       return i; */

/*   // character isn't found. */
/*   return NOT_FOUND; */
/* } // end - find() */

/**
 * @brief an interface function which inserts into internal string data with
 * given encoded string class.
 *
 * @param inputString   an encoded string class to assign to the UString.
 * @param encodingType  an enumerated type which indicates encoding type.
 * @return              Reference of assigned UString.
 */
inline SelfT& assign(const std::string& initString,
                     EncodingType encodingType)
{
    //algo::read_from_encode(EncodingTypeString[encodingType],initString.c_str(),initString.length(), *this);
    if (initString.length() == 0)
    {
        clear();
        return *this;
    }

    derefer();

    max_size_ = initString.length()+1;
    length_ = 0;
    is_attached_ = false;

    p_ = (char*)HLmemory::hlmalloc(get_total_size(max_size_-1));
    str_ = (CharT*)(p_+sizeof (ReferT));

    setUString(initString, encodingType);

    is_attached_ = false;

    clear_reference();

    return *this;
} // end - assign()

/**
 * @brief an interface function which inserts into internal string data with
 * given encoded string class.
 *
 * @param inputString   an encoded const char* to assign to the UString.
 * @param len           length of inputString.
 * @param encodingType  an enumerated type which indicates encoding type.
 * @return              Reference of assigned UString.
 */

inline SelfT& assign(const char* initString, size_t len, EncodingType encodingType)
{
    if (len == 0)
    {
        clear();
        return *this;
    }

    derefer();

    max_size_ = len +1;
    length_ = 0;
    is_attached_ = false;

    p_ = (char*)HLmemory::hlmalloc(get_total_size(max_size_-1));
    str_ = (CharT*)(p_+sizeof (ReferT));

    setUString(initString, len, encodingType);

    clear_reference();

    //algo::read_from_encode(EncodingTypeString[encodingType],initString, len, *this);
    return *this;
} // end - assign()

/**
 * @brief an interface function which inserts into internal string data with
 * given encoded string class.
 *
 * @param inputString   an encoded const char* to assign to the UString.
 * @param encodingType  an enumerated type which indicates encoding type.
 * @return              Reference of assigned UString.
 */

inline SelfT& assign(const char* initString, EncodingType encodingType)
{
    size_t len = 0;//getLen((CharT*)initString);
    while (initString[len]!='\0')
        len++;
    return assign(initString, len, encodingType);
} // end - assign()



static size_t convertString( EncodingType encodingType,
                             const CharT* const inputString,
                             const size_t inputStringLength,
                             char* const outputBuffer,
                             const size_t outputBufferLimit )
{
    if (inputStringLength == 0)
        return 0;

    // Terminate converting if encoding type is not right value.
    if (encodingType == SelfT::UNKNOWN || encodingType
            == SelfT::TOTAL_ENCODING_TYPE_NO)
    {
        return 0;
    } // end - if

//  boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();

    size_t inputStringIndex = 0;
    size_t inputStringLeftLength = inputStringLength;
    size_t outputStringIndex = 0;

    size_t outputBufferSize = outputBufferLimit;

    unsigned int inputUCSCharacter;

    int returnValue;

    // Coninue converting characters one by one while there is no space in output buffer or there is no character to read.

    while (outputBufferSize != 0 && inputStringLeftLength != 0)
    {
        // Convert ucs2 -> ucs4
        assert(inputStringIndex< inputStringLength);
        returnValue = ucs2_mbtowc(&inputUCSCharacter,
                                  &inputString[inputStringIndex], inputStringLeftLength);

        // If there is no matched character in converting table, insert space character to the output.
        if (returnValue < 0)
        {
            outputBuffer[outputStringIndex] = 0x20;
            outputStringIndex++;
            outputBufferSize--;
        } // end - if

        // If there is matched character in converting table, insert right converting character to the output.
        else
        {
            // Using converting functions offered by iconv library. It will return byte size of converting character.
            returnValue
            = ConvertFunctionList[encodingType].convertFromUCS(
                  (unsigned char*)&outputBuffer[outputStringIndex],
                  inputUCSCharacter, outputBufferSize);

            // If there is no matched character in converting table, insert space character to the output.
            switch (returnValue)
            {
            case RET_ILUNI:
                outputBuffer[outputStringIndex] = 0x20; // space character
                outputStringIndex++;
                outputBufferSize--;
                break;

            case RET_TOOSMALL: // If input string is shorter than output, stop converting.
                inputStringLeftLength = 0;
                break;

            default:
                outputStringIndex += returnValue;
                if (outputBufferSize > (size_t)returnValue)
                    outputBufferSize -= returnValue;
                else
                    outputBufferSize = 0;
            } // end - switch()

        } // end - else

        inputStringIndex++;
        inputStringLeftLength--;

    } // end - while()

    // If there's a space to insert NULL charactor, insert NULL.
    assert(outputBufferSize > 0);
    assert(outputStringIndex < outputBufferLimit);
    outputBuffer[outputStringIndex] = '\0';

    return outputStringIndex;
}

size_t convertString( EncodingType encodingType,
                      char* const outputBuffer,
                      const size_t outputBufferLimit) const
{
    return convertString(encodingType, str_, length(), outputBuffer, outputBufferLimit);
}

/**
 * @brief an interface function to serve a certain type of string class
 * which is encoded with given encoding type.
 *
 * @param outputString  a string class which is the result of this function.
 * @param encodingType  an enumerated type which indicates encoding type.
 */
void convertString(std::string& outputString,
                   EncodingType encodingType) const
{
    outputString.clear();
    if (length() == 0)
        return;

    // Terminate converting if encoding type is not right value.
    if (encodingType == SelfT::UNKNOWN || encodingType
            == SelfT::TOTAL_ENCODING_TYPE_NO)
    {
        outputString = "";
        return;
    } // end - if

    size_t outputBufferSize = 3 * length() + 1; // UTF-8 consumes a maximum of 3 bytes.


    //char* outputStringBuffer = new char[outputBufferSize + 1];
    //std::cout<<"malloc "<<outputBufferSize<<std::endl;
    char* outputStringBuffer = (char*)HLmemory::hlmalloc(outputBufferSize+1);
    //std::cout<<"malloced...";

    size_t length = convertString(encodingType, outputStringBuffer, outputBufferSize+1);

    outputString.assign(( char*)outputStringBuffer, length);

    HLmemory::hlfree(outputStringBuffer);

    /*   char* buf = NULL; */
    /*   size_t len = 0; */
    /*   algo::write_to_encode(EncodingTypeString[encodingType], &buf, len, *this); */
    /*   outputString.reserve(len); */
    /*   outputString.assign(buf, len); */
    /*   algo::release(buf); */
} // end - convertString


/**
 * @brief an interface function to transfrom characters of any encoding type into ucs2 characters.
 * @details This interface is extracted from setUString(), and is added to avoid UString objects allocation
 * and improve LA's performance, Wei, 2010/08/04.
 *
 * @param encodingType      an enumerated type which indicates encoding type of inputString.
 * @param inputString       an char* array which contains characters of a specific encoding type.
 * @param inputStringSize   the number of char contained in inputString.
 * @param outputBuffer      an CharT* array which is used for hold the output ucs2 encoding characters.
 * @param outputBufferLimit the maximum length of outputBuffer, counted in CharT.
 */
static size_t toUcs2( EncodingType encodingType,
                      const char* const inputString, const size_t inputStringLength,
                      CharT* const outputBuffer, const size_t outputBufferLimit)
{
    // Terminate converting if encoding type is not right value.
    if (encodingType == SelfT::UNKNOWN || encodingType
            == SelfT::TOTAL_ENCODING_TYPE_NO)
    {
        return 0;
    } // end - if

    size_t inputStringIndex = 0;
    size_t inputStringLeftLength = inputStringLength; // Assign the length

    size_t outputStringIndex = 0;
    size_t outputBufferSize = outputBufferLimit;//*sizeof(CharT);

    unsigned int outputUCSCharacter;

    int returnValue;

    // Coninue converting characters one by one while there is no space in output buffer or there is no character to read.
    while (outputBufferSize != 0 && inputStringLeftLength != 0)
    {

        // Using converting functions offered by iconv library. It will return byte size of converting character.
        assert(inputStringIndex < inputStringLength);
        returnValue
        = ConvertFunctionList[encodingType].convertToUCS(
              &outputUCSCharacter,
              (unsigned char*)&inputString[inputStringIndex],
              inputStringLeftLength);

        // If there is no matched character in converting table, insert space character to the output.
        if (returnValue < 0)
        {
            switch (returnValue)
            {
            case RET_ILSEQ:
                outputBuffer[outputStringIndex] = (UCS2Char)0x20; // insert space character to the UCS4 String
                inputStringLeftLength--;
                inputStringIndex++;
                break;

            default: // Illigal return value. Stop converting.
                inputStringLeftLength = 0;
                break;
            } // end - switch

        } // end - if

        // If there is matched character in converting table, insert right converting character to the output.
        else
        {
            // Move to the next position in the input string.
            inputStringIndex += returnValue;
            if (inputStringLeftLength > (size_t)returnValue)
                inputStringLeftLength -= returnValue;
            else
                inputStringLeftLength = 0;

            // Convert ucs4 to ucs2
            returnValue = ucs2_wctomb(&outputBuffer[outputStringIndex],
                                      outputUCSCharacter, outputBufferLimit);

            if (returnValue < 0)
                outputBuffer[outputStringIndex] = (UCS2Char)0x20;

            outputStringIndex++;
            outputBufferSize--;
        } // end - else

    } // end - while

    // If there is no characters which needs to be convert, insert NULL into the output.
    assert(outputBufferSize > 0);
    assert(outputStringIndex < outputBufferLimit);
    outputBuffer[outputStringIndex] = '\0';

    return outputStringIndex;
}

/**
 * @brief an interface function which serves sub string of original one.
 *
 * @param outputUString a UString class which is the result of this function.
 * @param pos           a position which indicates the start point of sub string.
 */

// Log : 2009.03.02
//_UString& subString(_UString<AllocatorManager_T2>& outputUString, size_t pos) const
inline SelfT substr(SelfT& outputUString, size_t pos) const
{
    outputUString = substr(pos);
    return outputUString;
} // end - substr()

/**
 * @brief a second interface function which serves sub string of original one.
 *
 * @param outputUString a UString class which is the result of this function.
 * @param pos           a position which indicates the start point of sub string.
 * @param nChar         the number of characters which is the length of sub string.
 */
// Log : 2009.03.02
//_UString& subString(_UString<AllocatorManager_T2>& outputUString, size_t pos, size_t nChar) const
SelfT substr(SelfT& outputUString, size_t pos, size_t nChar) const
{
    outputUString = substr(pos, nChar);
    return outputUString;
} // end - substr()


/**
 * @brief an default format interface function which offers convinient formatted string insertion like a sprintf().
 * It just calls formatProcess() functions. It uses default sized(1024 byte) buffer.
 *
 * @param encodingType  an enumerated type which indicates encoding type.
 * @param formatString  a string which contains format characters. For example, "%d percent of people will be happ"
 * @param ...           a parameter list which is the value of format string.
 */
void format(EncodingType encodingType, const char* formatString,
            ...)
{
    va_list argumentList; // argumentList contains ...
    va_start(argumentList, formatString);
    formatProcess(encodingType, 1024, formatString, argumentList);
} // end - format()


/**
 * @brief an interface which offers convinient formatted string insertion like a sprintf().
 * It also calls formatProcess() function to get the format string.
 *
 * @param encodingType  an enumerated type which indicates encoding type.
 * @param maxBufferSize a maximum size of buffer which will contains the result string.
 * @param formatString  a string which contains format characters. For example, "%d percent of people will be happ"
 * @param ...           a parameter list which is the value of format string.
 */
inline void format(EncodingType encodingType, int maxBufferSize,
                   const char* formatString, ...)
{
    va_list argumentList; // argumentList contains ...
    va_start(argumentList, formatString);
    formatProcess(encodingType, maxBufferSize, formatString,
                  argumentList);
} // end - format()


private:

/**
 * @brief an format process function which offers convinient formatted string insertion like a sprintf().
 * This function is a real processing function.
 *
 * @param encodingType  an enumerated type which indicates encoding type.
 * @param maxBufferSize a maximum size of buffer which will contains the result string.
 * @param formatString  a string which contains format characters. For example, "%d percent of people will be happ"
 * @param argumentList  a parameter list which is the value of format string.
 */
void formatProcess(EncodingType encodingType, int maxBufferSize,
                   const char* formatString, va_list argumentList)
{
    //char* buffer = new char[maxBufferSize + 1]; // buffer contains result of the format. + 1 is a flexible number according to the OS.
    char* buffer = (char*)HLmemory::hlmalloc( (maxBufferSize + 1)* sizeof(char));

    vsnprintf(buffer, maxBufferSize, formatString, argumentList);

    std::string bufferContainer; // bufferContainer is used for encoding UString.
    bufferContainer = buffer;

    assign(buffer, encodingType);
    //delete[] buffer;
    HLmemory::hlfree(buffer);
} // end - formatProcess()



/**
 * @brief an interface function to set data area of UString with certain encoded string.
 * @details
 * p.s This interface should be called after data_ allocation is finished.
 *
 * @param inputString   a string class which is the input of this function.
 * @param encodingType  an enumerated type which indicates encoding type.
 */
void setUString(const std::string& inputString,
                EncodingType encodingType)
{

    length_ = toUcs2(encodingType, inputString.c_str(), inputString.size(), str_, max_size_);

    /* 2008.07.31
     * no need to use dataString_ because Sunday support ustring directly.
     */
    // Set dataString_
    //convertString(dataString_, encodingType);

} // end - setUString()


/**
 * @brief an interface function to set data area of UString with certain encoded string.
 * @details
 * p.s This interface should be called after data_ allocation is finished.
 *
 * @param inputString   a string class which is the input of this function.
 * @param encodingType  an enumerated type which indicates encoding type.
 */
void setUString(const char* inputString, size_t len,
                EncodingType encodingType)
{

    length_ = toUcs2(encodingType, inputString, len, str_, max_size_);
} // end - setUString()


public:

/**
 * @brief an interface function which shows string value in certain encoding type.
 *
 * @param encodingType  an enumerated type which indicates encoding type.
 * @param outputStream  an output stream where the string message is shown. default value is cerr
 */
void displayStringValue(EncodingType encodingType,
                        std::ostream& outputStream = std::cout) const
{
    //algo::display(*this, EncodingTypeString[encodingType], outputStream);

    std::string output;
    convertString(output, encodingType);
    outputStream << output;
} // end - displayStringValue()


/**
 * @brief an interface function which shows class infomation of UString in certain encoding type.
 *
 * @param encodingType  an enumerated type which indicates encoding type.
 * @param outputStream  an output stream where the string message is shown. default value is cerr
 */
inline void displayStringInfo(EncodingType encodingType,
                              std::ostream& outputStream = std::cout) const
{
    displayStringValue(encodingType, outputStream);
} // end - displayStringInfo()

/**
 * @brief it displayes hexadecimal values of string.
 */
void displayHexaValue(std::ostream& outputStream = std::cout) const
{
    stringstream ss;
    for(size_t i = 0; i < length_; i++)
        ss << std::hex << str_[i] << " ";
    outputStream << ss.str();
}

/**
 * @brief   an interface function which offers buffer size of UString.
 *
 * @return  the byte size of buffer. It is the total size of data area of UString.
 */
inline size_t bufferSize() const
{
    return max_size_;
} // end - bufferSize()

public:

/// An comparison operator of UString
inline bool operator == (const SelfT& compareString) const
{
    return compare(compareString)==0;
} // end - operator==

/// An comparison operator of UString
inline bool operator != (const SelfT& compareString) const
{
    return compare(compareString)!=0;
} // end - operator !=

/// An comparison operator of UString
inline bool operator < (const SelfT& compareString) const
{
    return compare(compareString)<0;
} // end - operator<

/// This function returns current buffer size.
inline unsigned int getBufferSize()
{
    return max_size_*sizeof(CharT);
}

/// @brief This function sets buffer which is allocated from outside.
/// @details
///     - buffer should be filled with UCS2Char and contains NULL at the end of the string.
///     - Memory area of buffer must be managed inside the UString after using this interface.
/// @param length       The length of given buffer.
/// @param bufferSize   The size of given buffer.
/// @param buffer       UString data buffer which is filled from outside.
//
inline void setBuffer(size_t length, size_t bufferSize,
                      void* buffer)
{
    //std::cout<<length<<std::endl;
    assign(length, (CharT*)buffer);
    //std::cout<<"llllllllllll\n";
} // end - setBuffer

static typename SelfT::EncodingType convertEncodingTypeFromStringToEnum(const char* encodingTypeString)
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
        if ( givenCharacter> 96 && givenCharacter < 123 )
            givenCharacter -= 32;

        encodingTypeCharacter = SelfT::EncodingTypeString[encodingTypeIndex][stringIndex];

        if (givenCharacter == 0)
        {
            // If given string is fully matched to the certain EncodingTypeString, set true.
            if (encodingTypeCharacter == 0)
                isEncodingTypeFound = true;
            break;
        } // end - if

        if ( givenCharacter == encodingTypeCharacter )
        {
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


friend std::istream& operator >> ( std::istream& in , SelfT& ustringObj) // Log : 2009.07.23
{
    assert (ustringObj.systemEncodingType_ != SelfT::UNKNOWN );
    std::string inputString;
    in >> inputString;
    ustringObj.assign( inputString , ustringObj.systemEncodingType_ );

    return in;
}

public:
inline void setSystemEncodingType(EncodingType currentEncodingType)
{
    systemEncodingType_ = currentEncodingType;
} // Log : 2009.07.23
EncodingType getSystemEncodingType(void) const
{
    return systemEncodingType_;
} // Log : 2009.07.23
