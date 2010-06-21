///
/// @brief Header file of UString tool
/// @author Dohyun Yun
/// @date 2009-12-16
/// 

#ifndef _USTRTOOL_H_
#define _USTRTOOL_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "stdio.h"
#include "algo.hpp"
#include "ustr_types.h"

namespace ustring_tool {

    /**
     * @brief a function which gets text from file and stores line by line into vector<UString>
     *
     * @param encodingType  an encodingtype of file.
     * @param filename      filename with path.
     * @param lines         an output which stores each line of text.
     */
    template<class StringT>
        void getUStringLinesFromFile ( typename StringT::EncodingType encodingType,
                const std::string& filename, std::vector<StringT>& lines )

        {
            //return Algorithm<StringT>::read_from_file(StringT::EncodingTypeString[encodingType],
            //                                         filename.c_str(), lines);
            // clear output data
            lines.clear();

            // open file and check if it is opened
            std::ifstream fpin( filename.c_str() );
            if ( !fpin.is_open() )
            {
                std::cerr << "Cannot open " << filename << std::endl;
                return;
            } // end - if

            char ch;
            std::string buffer;
            while( fpin.good() )
            {
                fpin.get( ch );
                if ( ch == '\n' ) // get one line
                {
                    if ( buffer.size() == 0 ) continue;

                    StringT insertUString( buffer, encodingType );
                    lines.push_back( insertUString );

                    buffer.clear();
                } // end - if
                else if( ch == '\r' )
                    continue;
                else
                    buffer += ch;

                //std::cout<<buffer<<endl;
            } // end - while

        } // end - getUStringLinesFromFile()

    /**
     * @brief an interface to generate tokens by delimiter.
     *
     * @param encodingType  an encoding type string of delimiter.
     * @param delimiter     a delimiter character.
     * @param tokens        a vector of UString which stores tokens.
     */

    template<class StringT>
        void getTokensFromUString ( typename StringT::EncodingType encodingType, const char delimiter,
                const StringT & srcUString, std::vector<StringT>& tokens )
        {
            // clear output data
            tokens.clear();

            std::string delimiterString(1, delimiter);

            StringT delimiterUString(delimiterString, encodingType);

            return ::izenelib::util::Algorithm<StringT>::make_tokens_with_delimiter(srcUString, delimiterUString, tokens);
        } // end - getTokensFromUString()

    /**
     * @brief an interface to convert Korean compatibility jamo to Korean Jamo.
     * @param[srcChar]    Source Korean Compatibility jamo.
     * @param[tarChar]    Target Korean Jamo.
     */
    template<class StringT>
        inline void cvtCJamoToJamo ( const izenelib::util::UCS2Char& srcChar, izenelib::util::UCS2Char& tarChar )
        {
            assert ( StringT::isThisModernCJamo(srcChar) );
            izenelib::util::UCS2Char tmp = srcChar - 0x3131 + StringT::UCS2_CJAMO_CVT_TABLE[ srcChar - 0x3131 ] + 0x1100;
            tarChar = tmp;
        } // end - cvtCJamoToJamo()

    /**
     * @brief an interface to convert Korean compatibility jamo to Korean Jamo.
     * @param[srcChar]    Source Korean Compatibility jamo.
     * @param[tarChar]    Target Korean Jamo.
     * @return true       Success to convert character.
     * @return false      Fail to convert character.
     */
    template<class StringT>
        inline bool cvtCJamoToJamoWithCharCheck ( const izenelib::util::UCS2Char& srcChar, izenelib::util::UCS2Char& tarChar )
        {
            if ( !StringT::isThisModernCJamo(srcChar) )
                return false;
            cvtCJamoToJamo<StringT>( srcChar, tarChar );
            return true;
        } // end - cvtCJamoToJamo()

    /**
     * @brief an interface to convert Korean Jamo to Korean compatibility Jamo.
     * @param[srcChar]    Source Korean Jamo.
     * @param[tarChar]    Target Korean Compatibility Jamo.
     */
    template<class StringT>
        inline void cvtJamoToCJamo ( const izenelib::util::UCS2Char& srcChar, izenelib::util::UCS2Char& tarChar )
        {
            assert ( StringT::isThisJamo(srcChar) );
            izenelib::util::UCS2Char tmp = srcChar - 0x1100 - StringT::UCS2_CJAMO_CVT_TABLE[ srcChar - 0x1100 ] + 0x3131;
            tarChar = tmp;
        } // end - cvtJamoToCJamo()

    /**
     * @brief an interface to convert Korean compatibility jamo to Korean Jamo.
     * @param[srcChar]    Source Korean Jamo.
     * @param[tarChar]    Target Korean Compatibility Jamo.
     * @return true       Success to convert character.
     * @return false      Fail to convert character.
     */
    template<class StringT>
        inline bool cvtJamoToCJamoWithCharCheck ( const izenelib::util::UCS2Char& srcChar, izenelib::util::UCS2Char& tarChar )
        {
            if ( !StringT::isThisJamo(srcChar) )
                return false;
            cvtJamoToCJamo<StringT>( srcChar, tarChar );
            return true;
        } // end - cvtJamoToCJamo()

    /**
     * @brief an interafce to decompose korean string(korean character -> group of korean jamo).
     * This interface doesn't include korean character checking for the performance.
     *
     * NOTICE 1 : This interface cannot check if the input character is korean or not.
     *            Make sure srcUString is korean character string before using this interface.
     *
     * NOTICE 2 : This interface cannot decompose "compatibility jamo". 
     *            Use processKoreanDecomposerWithCharacterCheck() if that is needed.
     *
     * @param srcUString    a source UString which is used for decomposing process.
     * @param tarUString    a target UString which stores the result of process.
     *
     * @return true if more than one korean character is in srcUString, or false.
     */
    template<class StringT>
        bool processKoreanDecomposer(const StringT& srcUString, StringT& tarUString)
        {
            tarUString.clear();
            size_t srcLength = srcUString.length();
            uint16_t indexValue, cho, jung, jong;
            for(size_t i = 0; i < srcLength; i++)
            {
                indexValue = srcUString.at(i) - 0xAC00;
                jong = indexValue % 28;
                jung = ((indexValue - jong) / 28 ) % 21;
                cho  = ((indexValue - jung) / 28 ) / 21;

                // For correct matching of UCS2 table with cho, jung and jong, we need to add proper values for each jamo.
                cho  += 0x1100;
                tarUString += cho;

                jung += 0x1161;
                tarUString += jung;

                if (jong != 0) // jong seung can exist or not in the korean character.
                {
                    jong += 0x11A7;
                    tarUString += jong;
                }

                // DEBUG
                // cout << "Cho  : " << cho  << endl;
                // cout << "Jung : " << jung << endl;
                // cout << "Jong : " << jong << endl;

            } // end - for

            return true;

        } // end - processKoreanDecomposer()


    /**
     * @brief an interface to decompose korean string(korean character -> group of korean jamo).
     * This interface will check if each character is korean or not during the process.
     *
     * NOTICE : This interface can decompose "Korean Compatibility Jamo".
     *          For example, the UTF-8 character ㄱ or ㅏ can be transformed to Korean Jamo.
     *
     * @param srcUString    a source UString which is used for decomposing process. The
     * @param tarUString    a target UString which stores the result of process.
     *
     * @return true if more than one decomposed korean character is in srcUString, or false.
     */
    template<class StringT>
        bool processKoreanDecomposerWithCharacterCheck(const StringT& srcUString, StringT& tarUString)
        {
            bool ret = false;
            StringT tmpUString;

            size_t srcLength = srcUString.length();
            uint16_t indexValue, cho, jung, jong;
            for(size_t i = 0; i < srcLength; i++)
            {

                indexValue = srcUString.at(i);

                if ( StringT::isThisModernCJamo(indexValue) )
                {
                    ret = true;
                    cvtCJamoToJamo<StringT>(indexValue, cho);
                    if ( StringT::isThisJongseong( cho ) ) //ignore jongseong converting.
                        tmpUString += indexValue;
                    else
                        tmpUString += cho;
                    continue;
                }
                else if ( !StringT::isThisKoreanChar(indexValue) )
                {
                    tmpUString += indexValue;
                    continue;
                }

                ret = true;

                indexValue = srcUString.at(i) - 0xAC00;
                jong = indexValue % 28;
                jung = ((indexValue - jong) / 28 ) % 21;
                cho  = ((indexValue - jung) / 28 ) / 21;

                // For correct matching of UCS2 table with cho, jung and jong, we need to add proper values for each jamo.
                cho  += 0x1100;
                tmpUString += cho;

                jung += 0x1161;
                tmpUString += jung;

                if (jong != 0) // jong seung can exist or not in the korean character.
                {
                    jong += 0x11A7;
                    tmpUString += jong;
                }

                // DEBUG
                // cout << "Cho  : " << cho  << endl;
                // cout << "Jung : " << jung << endl;
                // cout << "Jong : " << jong << endl;

            } // end - for

            tarUString.swap( tmpUString );
            return ret;

        } // end - processKoreanDecomposer() */

    /**
     * @brief an interface to compose three korean jamos into one korean character.
     * @author Dohyun Yun
     * @param cho     chosung  jamo
     * @param jung    jungsung jamo
     * @param jong    jongsung jamo
     * @return        Composed one korean character. 0 for invalid parameter values.
     */
    template<class CHAR_TYPE>
        CHAR_TYPE composeOneKoreanChar(unsigned int cho, unsigned int jung, unsigned int jong)
        {
            cho  -= 0x1100;
            jung -= 0x1161;
            jong -= 0x11A7;

            CHAR_TYPE comChar = ((cho*21) + jung) * 28 + jong + 0xAC00;
            return comChar;
        }

    /**
     * @brief an interface to compose korean chracter using group of korean jamo.
     * - NOTICE : Restriction
     *      - Two inputs("가" & "ㄱㅏ") are decomposed to ("ㄱㅏ"). So this cannot
     *      generate composed result of "ㄱㅏ". That means there's no Compatibility
     *      jamo result when the string is composed.
     * @author Dohyun Yun
     * @details
     * This interface can manage both korean or not-korean character in the string.
     * @param srcUString    a source UString which contains the korean jamos.
     * @param tarUString    a target UString which stores the result of this process.
     */
    template<class StringT>
        bool processKoreanComposer(const StringT& srcUString, StringT& tarUString)
        {

            bool ret = false;
            StringT tmpUString;

            izenelib::util::UCS2Char cho, jung, jong, curChar, finalCharacter;
            size_t i = 0, srcLength = srcUString.length();

            while(i < srcLength)
            {
                curChar = srcUString.at(i++);
                if( StringT::isThisChoseong(curChar) ) // if current char is cho-seung
                {
                    ret = true;
                    cho = curChar;

                    if ( i == srcLength ) // if only independent cho-seung is appeared.
                    {
                        if ( curChar < 0x1113 )
                        {
                            cvtJamoToCJamo<StringT>(curChar, finalCharacter);
                            tmpUString += finalCharacter;
                            tarUString.swap(tmpUString);
                            return true;
                        }
                        else {
                            std::cerr << "[UString Tool] Error : Invalid sequence while processKoreanComposing" << std::endl;
                            return false;
                        }
                    }

                    curChar = srcUString.at(i++);
                    if ( !StringT::isThisJungseong(curChar) )
                    {
                        cvtJamoToCJamo<StringT>(curChar, finalCharacter);
                        tmpUString += finalCharacter;
                        continue;
                    } // end - if

                    jung = curChar;

                    // If the next char is out of jongsung area or it reaches to the end of the string.
                    if ( i == srcLength )
                        jong = 0x11A7;
                    else
                    {
                        if ( !srcUString.isJongseong(i) )
                            jong = 0x11A7;
                        else
                        {
                            jong = srcUString.at(i);
                            i++;
                        }
                    }

                    finalCharacter = composeOneKoreanChar<uint16_t>(cho, jung, jong);
                    if ( finalCharacter < 1 )
                        return false;

                    tmpUString += finalCharacter;

                    //cout << "Added Korean char : " << finalCharacter << endl;

                }
                else if ( StringT::isThisJungseong(curChar) ) 
                {
                    cvtJamoToCJamo<StringT>(curChar, finalCharacter);
                    tmpUString += finalCharacter;
                    continue;
                }
                else
                {
                    //cout << "Added Other char : " << srcUString.at(i) << endl;
                    tmpUString += curChar;
                }
            } // end - while()

            tarUString.swap(tmpUString);
            return ret;

        } // end - processKoreanComposer()

} // end - namespace ustring_stuff

#endif // _USTRTOOL_H_
