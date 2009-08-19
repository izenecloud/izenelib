/**
 * @file    LAManager.h
 * @brief   Defines the dummy LAManager class
 * @author  MyungHyun Lee  (Kent)
 * @date    2009-01-30
 */

#ifndef _DUMMY_LA_MANAGER_H_
#define _DUMMY_LA_MANAGER_H_

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

namespace sf1v5_dummy {
/**
 * @brief   This clas analyzes the text.
 * @details
 * In the original the LAManager's job includes stemming, finding stopwords, and morphological analyzing. 
 * However, in this dummy version, to make life much more simple, it only parses terms (words) from a text.
 */
class LAManager {
public:
	LAManager();

	/**
	 * @brief   Given a text, the words within the text are parsed and returned
	 */
	void parseString(const std::string & text,
			std::vector<std::string> & termList);
};

typedef boost::shared_ptr<LAManager> LAManagerPtr;
}



#endif  //_DUMMY_LA_MANAGER_H_
