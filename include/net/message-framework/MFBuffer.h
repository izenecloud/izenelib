#ifndef MFBUFFER_H_
#define MFBUFFER_H_

#include <iostream>

using namespace std;

namespace messageframework{

class MFBuffer
{
public:
	/**
	 * \brief Default constructor
	 */
	MFBuffer(void) {
		data = 0;
		size = 0;
	}

	/**		 
	 *  \brief Constructor taking a pointer and a size. Make a copy of the _data.
	 */
	MFBuffer(void* pd, size_t sz) {
		size = sz;
		if (size != 0) {
			data = new char[size];
			memcpy(data, pd, size);
		}
	}


	/**
	 *  \brief Constructor taking a (possibly) null terminated string.
	 */
	MFBuffer(const char* ps, size_t sz = 0) {
		size = (0 == sz) ? strlen(ps) : sz;
		data = new char[size];
		memcpy(data, ps, size);
	}

	
	/**
	 *  \brief Copy constructor. Call the assignment operator.
	 */
	MFBuffer(MFBuffer& obj) {
		data = 0;
		size = 0;
		operator=(obj);
	}

	/**
	 * \brief Destructor. Delete the pointer if the size is non-zero.
	 */
	~MFBuffer() {		
		if (size != 0) {
			delete[] data;
			data = 0;
			size = 0;
		}
	}

	/**		 
	 *  \brief Assignment operator. Make a copy of the other object.
	 */
	MFBuffer& operator=(MFBuffer& obj) {
		if (size != 0) {
			delete[] data;
		}
		if (obj.size > 0) {
			size = obj.size;
			data = new char[size];
			memcpy(data, obj.data, size);
		}
		return *this;
	}

	bool operator <=(MFBuffer& obj) {
		return memcmp(data, obj.getData(), min(size, obj.getSize()));

	}

public:

	const void* getData() const {
		return data;
	}
	
	size_t getSize() {
		return size;
	}
	
	/*
	 * 	we can set any type of data. By this, we can deal with different KeyType and DataType.
	 * 
	 */

	void setData(const void* pd, size_t sz) {
		if (size) {
			delete[] data;
		}
		size = sz;
		data = new char[size];
		memcpy(data, pd, size);
	}

	/*
	 * 	for debug.
	 */

	void display() {
		cout<<"MFBuffer display.\n";
		cout<<"size "<<size<<endl;
		//cout<<"_data "<<(char*)_data<<endl;
	}


public:
	char* data;
	size_t size;
};

typedef boost::shared_ptr<MFBuffer> MFBufferPtr;

}


#endif /*MFBUFFER_H_*/
