#ifndef STRING_CHAIN_H_
#define STRING_CHAIN_H_


class string_chain {	
	size_t blockSize_;
public:
	char *str;
	long fpos;
	int num;
	string_chain* next;
	bool isLoaded;
	bool isDirty;
	
	static int activeNum;

public:
	string_chain(size_t blockSize) :
	blockSize_(blockSize) {
		str = NULL;
		num = 0;
		next = 0;
		isLoaded = false;
		isDirty = true;
		fpos = 0;
		
		++activeNum;
	}

	~string_chain() {
		delete str;
		str = 0;
		next = 0;
		num = 0;
		isLoaded = false;
		
		--activeNum;
	}

	bool write(FILE* f) {
		if (!isDirty) {
			return true;
		}

		if (!isLoaded) {
			return true;
		}

		if (!f) {
			return false;
		}

		//cout<<"write fpos="<<fpos<<endl;
		if ( 0 != fseek(f, fpos, SEEK_SET) )
		return false;

		if (1 != fwrite(&num, sizeof(int), 1, f) ) {
			return false;
		}
		//cout<<"write num="<<num<<endl;

		if (1 != fwrite(str, blockSize_, 1, f) ) {
			return false;
		}

		long nextfpos = 0;

		if (next)
		nextfpos = next->fpos;

		//cout<<"write nextfpos = "<< nextfpos<<endl;

		if (1 != fwrite(&nextfpos, sizeof(long), 1, f) ) {
			return false;
		}

		isDirty = false;
		isLoaded = true;

		return true;
	}
	bool read(FILE* f) {
		if (!f) {
			return false;
		}

		//cout<<"read from "<<fpos<<endl;
		if ( 0 != fseek(f, fpos, SEEK_SET) )
		return false;

		if (1 != fread(&num, sizeof(int), 1, f) ) {
			return false;
		}
		//cout<<"read num="<<num<<endl;

		if ( !str)
		str = new char[blockSize_];
	    memset(str, 0, blockSize_);

		//cout<<"write blocksize="<<blockSize_<<endl;

		if (1 != fread(str, blockSize_, 1, f) ) {
			return false;
		}

		long nextfpos = 0;

		if (1 != fread(&nextfpos, sizeof(long), 1, f) ) {
			return false;
		}

		//cout<<"read next fpos="<<nextfpos<<endl;
		if (nextfpos !=0) {
			next = new string_chain(blockSize_);
			next->fpos = nextfpos;
		}
		isLoaded = true;
		isDirty = false;

		return true;
	}

	string_chain* loadNext(FILE* f) {
		if (next && !next->isLoaded) {
			//cout<<"load from file"<<endl;
			next->read(f);
		}
		return next;
	}

	void display(std::ostream& os = std::cout) {
		cout<<"\nstring_chain display...."<<endl;
		os<<num<<endl;
		os<<fpos<<endl;
		os<<str<<endl;
		if (next)
		next->display(os);
	}

};

int string_chain::activeNum;


#endif /*STRING_CHAIN_H_*/
