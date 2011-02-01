/**
 * @file    ScdParser.h
 * @brief   
 * @author  MyungHyun Lee (Kent)
 * @date    Feb 02, 2009
 */

#ifndef _DUMMY_SCD_PARSER_H_
#define _DUMMY_SCD_PARSER_H_


#include <document-manager/Document.h>

#include <vector>
#include <string>

namespace sf1v5_dummy
{

    /**
     * @brief   Parses the document and saves them into a Document class
    */
    void parseScdFile( const char * filePath, std::vector<Document> & docList, int max=1000000 );

}

#endif  //_DUMMY_SCD_PARSER_H_
