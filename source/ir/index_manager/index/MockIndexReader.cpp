#include <ir/index_manager/index/MockIndexReader.h>

NS_IZENELIB_IR_BEGIN

namespace indexmanager {

    /**************************************************************
     *                   MockIndexReaderWriter
     **************************************************************/

    MockIndexReaderWriter::MockIndexReaderWriter() : reader_(NULL) {}

    MockIndexReaderWriter::~MockIndexReaderWriter(){
        if(reader_) delete reader_;
    }

    freq_t MockIndexReaderWriter::docFreq(collectionid_t colID, Term* term) {
        return getTermReader(colID)->docFreq(term);
    }

    TermInfo* MockIndexReaderWriter::termInfo(collectionid_t colID, Term* term) {
        return getTermReader(colID)->termInfo(term);
    }

    /**
     * @param colID ignored
     */
    TermReader* MockIndexReaderWriter::getTermReader(collectionid_t colID) {
        return (TermReader*) new MockTermReader(this);
    }

//    /// Not Implemented yet
//    MockForwardIndexReader* getForwardIndexReader(){
//        return NULL;
//    }

    /**************************************************************
     *                   MockTermReader
     **************************************************************/

    MockTermReader::MockTermReader(MockIndexReaderWriter* index)
        : index_(index), term_(NULL), termInfo_(NULL) {}

    MockTermReader::~MockTermReader(void)
    {
        if(term_) delete term_;
        if(termInfo_) delete termInfo_;
    }

    TermIterator* MockTermReader::termIterator(const char* field)
    {
        return (TermIterator*)new MockTermIterator(index_, std::string(field) );
    }

    TermDocFreqs*	MockTermReader::termDocFreqs()
    {
        return (TermDocFreqs*)new MockTermDocFreqs(index_, *term_);
    }

    TermPositions*	MockTermReader::termPositions()
    {
        return (TermPositions*)new MockTermPositions(index_, *term_);
    }

    TermInfo* MockTermReader::termInfo(Term* term)
    {
        typedef std::map<std::pair<std::string, termid_t>, MockPostings>::iterator Iter;
        Iter it = index_->inverted_.find(std::make_pair(std::string(term->getField()), term->getValue()));
        if(it == index_->inverted_.end()) 
        {
            return NULL;
        }
        if(termInfo_ == NULL)
            termInfo_ = new TermInfo;
        termInfo_->set(it->second.size(), it->second.size(),0, -1,-1,0,-1,0,-1,0);
        return termInfo_;
    };

    /**************************************************************
     *                   MockTermIterator
     **************************************************************/

    MockTermIterator::MockTermIterator(MockIndexReaderWriter* index,
            std::string property)
        : index_(index), property_(property), term_("")
    {
        cursor_ = index_->inverted_.begin();
    }

    MockTermIterator::~MockTermIterator() {}

    bool MockTermIterator::next() {
        cursor_++;
        while( cursor_ != index_->inverted_.end() ) {
            if(cursor_->first.first == property_)
                return true;
            cursor_++;
        }
        return false;
    }

    /**************************************************************
     *                   MockTermDocFreqs
     **************************************************************/
    MockTermDocFreqs::MockTermDocFreqs(MockIndexReaderWriter* index, Term& term)
        : index_(index), property_(term.getField()), termid_(term.getValue())
    {
        postings_ = index_->inverted_.find(std::pair<std::string, termid_t>(property_, termid_))->second;
        cursor_ = -1;

        ctf_ = 0;
        for(size_t i=0; i<postings_.size(); i++) {
            ctf_ += boost::get<3>(postings_[i]).size();
        }
    }

    /**************************************************************
     *                   MockTermPositions
     **************************************************************/

}

NS_IZENELIB_IR_END
