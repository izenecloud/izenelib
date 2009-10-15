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
    MockTermReader* MockIndexReaderWriter::getTermReader(collectionid_t colID) {
        if(reader_ == NULL)
            reader_ = new MockTermReader(this);
        return reader_;
    }

    /**
     * @param terms write space separated termid_t list
     */
    bool MockIndexReaderWriter::insertDoc(docid_t docid , const std::string& property,
            const std::string& terms)
    {
        if ( terms.size() == 0 ) return false;

        std::vector<termid_t> tl;
        termid_t t;

        std::stringstream ss(terms);
        while(!ss.eof()) {
            ss >> t;
            tl.push_back(t);
        }
        return insertDoc(docid, property, tl);
    }

    bool MockIndexReaderWriter::insertDoc(docid_t docid , const std::string& property,
            std::vector<termid_t>& terms)
    {
        if( !forward_.insert( std::make_pair(docid, std::make_pair(property, terms)) ).second )
            return false;

        for(size_t i  =0; i< terms.size(); i++ ) {
            // get postings
            std::pair<std::string, termid_t> term = std::make_pair(property, terms[i]);

            // find posting
            MockPostings& postings = inverted_[term];

            MockPostings::iterator it;
            for(it = postings.begin(); it < postings.end(); it++ ) {
                if( boost::get<0>(*it) <= docid )
                    break;
            }

            // update posting
            if( it != postings.end() && boost::get<0>(*it) == docid ) {
                boost::get<1>(*it) ++;
                boost::get<3>(*it).push_back(i);
            } else {
                // insert new posting
                MockPosting posting;
                boost::get<0>(posting) = docid;
                boost::get<1>(posting) = 1;
                boost::get<2>(posting) = terms.size();
                boost::get<3>(posting).push_back(i);
                postings.insert(it, posting);
            }
        }
        return true;
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

    MockTermIterator* MockTermReader::termIterator(const char* field)
    {
        return new MockTermIterator(index_, std::string(field) );
    }

    MockTermDocFreqs*	MockTermReader::termDocFreqs()
    {
        return new MockTermDocFreqs(index_, *term_);
    }

    MockTermPositions*	MockTermReader::termPositions()
    {
        return new MockTermPositions(index_, *term_);
    }

    TermInfo* MockTermReader::termInfo(Term* term)
    {
        typedef std::map<std::pair<std::string, termid_t>, MockPostings>::iterator Iter;
        Iter it = index_->inverted_.find(std::make_pair(std::string(term->getField()), term->getValue()));
        if(it == index_->inverted_.end()) return NULL;

        if(termInfo_ == NULL)
            termInfo_ = new TermInfo;
        termInfo_->set(it->second.size(), -1);
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
        cursor_ = 0;

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
