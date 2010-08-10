#ifndef COMPRESSOREXCEPTION_H_
#define COMPRESSOREXCEPTION_H_

#include<string>
using namespace std;

namespace izenelib {
namespace util{
namespace compression {


/**
 *  \brief CompressorException class
 */
class CompressorException {
private:
	string reason_; /* why this exception was thrown */

public:
	/**
	 * \brief Construct a new CompressorException object.
	 *
	 * \param r A C style string.
	 */
	CompressorException(const char* r) {
		this->reason_ = r;
	}

	/**
	 * \brief Construct a new CompressorException object.
	 *
	 * \param r A C++ style string.
	 */
	CompressorException(string r) {
		this->reason_ = r;
	}

	/**
	 * \brief Returns the reason of this exception.
	 *
	 * \return A string telling the user what has gone wrong.
	 */
	string get_reason() {
		return reason_;
	}
}; // CompressorException

}
}
}
#endif /*COMPRESSOREXCEPTION_H_*/
