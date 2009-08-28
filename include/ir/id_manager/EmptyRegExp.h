#ifndef EMPTY_REGEXP_H_
#define EMPTY_REGEXP_H_

#include <types.h>

NS_IZENELIB_IR_BEGIN

namespace idmanager {

template<typename NameString> class EmptyRegExp {

public:

	EmptyRegExp(){}

	~EmptyRegExp(){}

	void open(){}

	void close(){}

	void insert(const NameString & word, const unsigned int id){}

	int num_items(){return 0;}

	bool findRegExp(const NameString& exp, std::vector<unsigned int> & results){return false;}


}; // end - class EmptyRegExp

} // end - namespace idmanager

NS_IZENELIB_IR_END

#endif
