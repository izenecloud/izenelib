#ifndef SDBEXCEPTION_H_
#define SDBEXCEPTION_H_

class SDBException {
public:
	SDBException(const string& error) :
		message(error) {
	}
	string what() {
		return message;
	}
private:
	string message;
};

#endif /*SBEXCEPTION_H_*/
