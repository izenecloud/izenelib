#ifndef MEMMAP_H_
#define MEMMAP_H_


NS_IZENELIB_AM_BEGIN

namespace util{

const size_t BLOCK_SIZE = 1;

class MemBlock {
private:
	long startingPoint_;	
	size_t pageSize_;
	char *pBuf_;
	//size_t size_;
	
public:
	bool isLoaded;
	bool isDirty;

public:	
	MemBlock(long startingPoint, size_t pageSize) :
		startingPoint_(startingPoint), pageSize_(pageSize){		
		isLoaded = false;
		pBuf_ = new char[pageSize_*BLOCK_SIZE];
		isDirty =true;
	}
	virtual ~ MemBlock() {
		delete [] pBuf_;
	}
	bool load(FILE* f) {			
		if( isLoaded )return true;
		if (0 != fseek(f, startingPoint_, SEEK_SET)) {
			return false;
		}
		if (1 != fread(pBuf_, pageSize_, BLOCK_SIZE, f)) {
			return false;
		}
		isLoaded = true;
		isDirty = false;
		return true;
	}

	bool flush(FILE* f) {		
		if( !isDirty )return true;
		
		if (0 != fseek(f, startingPoint_, SEEK_SET)) {
			return false;
		}
		if (1 != fwrite(pBuf_, pageSize_, BLOCK_SIZE, f)) {
			return false;
		}
		delete []pBuf_;
		pBuf_ = 0;
		isLoaded = false;
		isDirty = false;
		return true;
	}
	char* getP(long fpos) {
		if( !pBuf_ )
			return NULL;
		//cout<<"startingPoint: "<< startingPoint_<<endl;
		//cout<<"fpos: "<<fpos<<endl;
		//cout<<( fpos-sizeof(SlfHeader) )/pageSize_<<endl;
		assert(fpos >= startingPoint_ && fpos <= startingPoint_ + long(pageSize_*BLOCK_SIZE) );
		size_t i = (fpos - startingPoint_)/pageSize_;
		//cout<<i<<endl;
		assert( i>=0 && i<BLOCK_SIZE );			
		return pBuf_ + pageSize_*i*sizeof(char);
	}

};

//extern vector< boost::shared_ptr<MemBlock> > mbList;
vector< MemBlock* > mbList;

}

NS_IZENELIB_AM_END

#endif /*MEMMAP_H_*/
