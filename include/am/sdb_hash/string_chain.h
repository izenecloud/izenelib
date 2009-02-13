#ifndef STRING_CHAIN_H_
#define STRING_CHAIN_H_


class string_chain {	
	size_t bucketSize_;
public:
	char *str;
	long fpos;
	int num;
	string_chain* next;
	bool isLoaded;
	bool isDirty;
	
	
	int level;
	
	//It indicates how many active string_chains in memory.
	//It increases when allocateBlock() called, or reading from disk,
	//and decreases when unload() called. 
	static size_t activeNum;

public:
	string_chain(size_t bucketSize) :
	bucketSize_(bucketSize) {
		str = NULL;
		num = 0;
		next = 0;
		isLoaded = false;
		isDirty = true;
		fpos = 0;
		
		level = 0;		
		
		//++activeNum;
	}

	virtual ~string_chain() {
		if( str ) delete str;
		str = 0;
		next = 0;	
		
		//--activeNum;
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

		size_t blockSize = bucketSize_ - sizeof(int) - sizeof(long);
			
		if (1 != fwrite(str, blockSize, 1, f) ) {
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
		size_t blockSize = bucketSize_ - sizeof(int) - sizeof(long);
		
		if ( !str )
		{
			str = new char[blockSize];		
     	    memset(str, 0, blockSize);
		}
 
		//cout<<"read blocksize="<<blockSize<<endl;
		if (1 != fread(str, blockSize, 1, f) ) {
			return false;
		}

		long nextfpos = 0;

		if (1 != fread(&nextfpos, sizeof(long), 1, f) ) {
			return false;
		}

		//cout<<"read next fpos="<<nextfpos<<endl;
		if (nextfpos !=0) {
			if( !next )next = new string_chain(bucketSize_);
			next->fpos = nextfpos;
		}
		isLoaded = true;
		isDirty = false;		
				
		++activeNum;
		
		return true;
	}

	string_chain* loadNext(FILE* f) {
		//cout<<"loadNext"<<endl;	
		//if(next)cout<<next->fpos<<endl;		
		if (next && !next->isLoaded) {			
			next->read(f);			
			//cout<<"load from file"<<endl;
			//cout<<"activeNode: "<<activeNum<<endl;	
		}
		//
		if( next )next->level = level+1;
		return next;
	}
	
	void unload(){			
		if(str){
			delete str;
			str = 0;			
			--activeNum;			
		}	
		isLoaded = false;
		
		//cout<<"unload fpos="<<fpos<<endl;
		//cout<<"activeNode: "<<activeNum<<endl;
	}

	void display(std::ostream& os = std::cout) {		
		os<<"(level: "<<level;
		os<<"  isLoaded: "<<isLoaded;
		os<<"  bucketSize: "<<bucketSize_;
		os<<"  numitems: "<<num;
		os<<"  fpos: "<<fpos;
		if(next)os<<"  nextfpos: "<<next->fpos;
		//os<<"str: "<<str;
		os<<")- > ";		
		if (next)
			next->display(os);
	}
};

size_t string_chain::activeNum;


#endif /*STRING_CHAIN_H_*/
