/**
 * @brief A set of mock objects for replacing IndexManager in unitests.
 * @author Wei Cao
 * @date 2009-10-14
 */

#ifndef _MOCK_INDEXREADER_H_
#define _MOCK_INDEXREADER_H

#include <string>
#include <vector>
#include <map>

#include <boost/tuple/tuple.hpp>

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/Term.h>
#include <ir/index_manager/index/TermInfo.h>
#include <ir/index_manager/index/AbsTermReader.h>
#include <ir/index_manager/index/AbsTermIterator.h>


NS_IZENELIB_IR_BEGIN

namespace indexmanager {

class MockTermReader;
class MockTermIterator;
class MockTermDocFreqs;
class MockTermPositions;

/**
 * Mock object for IndexReader
 */
class MockIndexReaderWriter
{
friend class MockTermReader;
friend class MockTermIterator;
friend class MockTermDocFreqs;
friend class MockTermPositions;

public:

    /// docid, tf, docLen, positions
    typedef boost::tuple<docid_t, freq_t, count_t, std::vector<loc_t> > MockPosting;

    typedef std::vector<MockPosting> MockPostings;

    MockIndexReaderWriter();

    ~MockIndexReaderWriter();

    count_t numDocs() { return forward_.size(); }

    count_t maxDoc() { return forward_.rbegin()->first; }

    freq_t docFreq(collectionid_t colID, Term* term);

    TermInfo* termInfo(collectionid_t colID, Term* term);

    bool isDirty() {return false;}

    void reopen() {}

    void setPropertiesAndIDs(std::vector<std::string> virtualPropreties, std::vector<unsigned int> propertyIdList)
    {
        if (virtualPropreties.size() != propertyIdList.size())
            return;
        for (uint32_t i = 0; i < virtualPropreties.size(); ++i)
        {
            field_string_map_.insert(std::make_pair(propertyIdList[i], virtualPropreties[i]));
        }
    }

    size_t docLength(docid_t docId, fieldid_t fieldId) 
    {
        std::string property;
        std::map<fieldid_t, std::string>::iterator iter1;
        iter1 = field_string_map_.find(fieldId);
        if (iter1 != field_string_map_.end())
        {
            property = iter1->second;
        }
        else
            return 0;
        std::map<std::pair<docid_t, std::string>, uint32_t>::iterator iter = 
        fieldLen_.find(std::make_pair(docId, property));
        if ( iter != fieldLen_.end())
        {
            return iter->second;
        }
        else
            return 0;
    }
    /**
     * @param colID ignored
     */
    TermReader* getTermReader(collectionid_t colID);

//    /// Not Implemented yet
//    MockForwardIndexReader* getForwardIndexReader(){
//        return NULL;
//    }

    /**
     * @param terms write space separated termid_t list
     */
    bool insertDoc(docid_t docid , const std::string& property,
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

    bool insertDoc(docid_t docid , const std::string& property,
            std::vector<termid_t>& terms)
    {
        /**
        Here should allowed each docs has different properties;
        */
        forward_.insert( std::make_pair(docid, std::make_pair(property, terms)));
        
        fieldLen_.insert(std::make_pair( std::make_pair(docid, property), terms.size()));

        for(size_t i  =0; i< terms.size(); i++ ) {
            // get postings
            std::pair<std::string, termid_t> term = std::make_pair(property, terms[i]);

            // find posting
            MockPostings& postings = inverted_[term];

            MockPostings::iterator it;
            for(it = postings.begin(); it < postings.end(); it++ ) {
                if( boost::get<0>(*it) >= docid )
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

private:

    std::map<docid_t, std::pair<std::string, std::vector<termid_t> > > forward_;

    std::map<std::pair<std::string, termid_t>, MockPostings> inverted_;

    MockTermReader* reader_;

    std::map<std::pair<docid_t, std::string>, uint32_t> fieldLen_;

    std::map<fieldid_t, std::string> field_string_map_;
};

typedef MockIndexReaderWriter::MockPosting MockPosting;
typedef MockIndexReaderWriter::MockPostings MockPostings;

class MockTermReader : public TermReader {

public:

    MockTermReader(MockIndexReaderWriter* index);

    ~MockTermReader(void);

    /// Nothing to do
    void open(Directory* pDirectory,const char* barrelname,FieldInfo* pFieldInfo){}

    void reopen(){}

    /// Nothing to do
    void close(){}

    TermIterator* termIterator(const char* field);
    /**
    * find the term in the vocabulary,return false if not found
    */
    bool seek(Term* pTerm) {
        if(!term_ ) {
            term_ = new Term(*pTerm);
        } else if(term_->compare(pTerm)) {
            term_->setField(pTerm->getField());
            term_->setValue(pTerm->getValue());
        }
        if( termInfo(term_) ) return true;
        return false;
    }

    TermDocFreqs*	termDocFreqs();

    TermPositions*	termPositions();

    freq_t docFreq(Term* term) {
        TermInfo* ti = termInfo(term);
        if(ti) return ti->docFreq();
        return 0;
    }

    TermReader*	clone() {
        return (TermReader*) (new MockTermReader(index_));
    }

    TermInfo* termInfo(Term* term);

private:

    MockIndexReaderWriter* index_;

    Term* term_;
    TermInfo* termInfo_;
};

/// Mock object for TermIterator
class MockTermIterator : public TermIterator {
public:
    MockTermIterator(MockIndexReaderWriter* index, std::string property);

    ~MockTermIterator();

public:
    bool next();

    const Term* term() {
        term_.setField(cursor_->first.first.c_str());
        term_.setValue(cursor_->first.second);
        return &term_;
    }

    const TermInfo* termInfo() {
        termInfo_.set(cursor_->second.size(), cursor_->second.size(), 0,-1,-1,0,-1,0,-1,0);
        return &termInfo_;
    };

    PostingReader* termPosting() {
        return NULL;
    }

private:
    MockIndexReaderWriter* index_;
    std::string property_;

    std::map<std::pair<std::string, termid_t>, MockPostings> ::iterator cursor_;

    Term term_;
    TermInfo termInfo_;
};

/// Mock object for TermDocFreqs
class MockTermDocFreqs : public TermDocFreqs {
public:
    MockTermDocFreqs(MockIndexReaderWriter* index, Term& term);

    virtual ~MockTermDocFreqs() {}

    freq_t	docFreq() { return postings_.size(); }

    int64_t	getCTF() { return ctf_;}

    virtual count_t next(docid_t*& docs, count_t*& freqs) {
        if(cursor_ == postings_.size()) return 0;
        docs = &boost::get<0>(postings_[cursor_]);
        freqs = &boost::get<1>(postings_[cursor_]);
        cursor_++;
        return 1;
    }

    virtual bool next() {
        if(cursor_ == postings_.size() -1 ) return false;
        cursor_++;
        return true;
    }

    virtual docid_t skipTo(docid_t target)
    {
        docid_t currDoc;
        do
        {
            if(!next())
                return 0xFFFFFFFF;
            currDoc = doc();
        } while(target > currDoc);

        return currDoc;
    }

    docid_t doc() { return boost::get<0>(postings_[cursor_]); }

    count_t freq() { return boost::get<1>(postings_[cursor_]); }

    freq_t docLength() { return boost::get<2>(postings_[cursor_]); }

    virtual void close(){ postings_.clear(); cursor_ = 0; }

protected:
    MockIndexReaderWriter* index_;
    std::string property_;
    termid_t termid_;

    MockPostings postings_;
    int64_t ctf_;
    size_t cursor_;
};

/// Mock object of TermPostions
class MockTermPositions : public MockTermDocFreqs , public TermPositions {
public:
    MockTermPositions(MockIndexReaderWriter* index, Term& term) : MockTermDocFreqs(index, term) {};

    ~MockTermPositions(){}

    freq_t	docFreq() { return MockTermDocFreqs::docFreq(); }

    int64_t	getCTF() { return MockTermDocFreqs::getCTF();}

    docid_t doc() { return MockTermDocFreqs::doc(); }

    count_t freq() { return MockTermDocFreqs::freq(); }

    freq_t docLength() { return MockTermDocFreqs::docLength(); }

    virtual void close(){ MockTermDocFreqs::close(); }

    count_t next(docid_t*& docs, count_t*& freqs) {
        posCursor_ = 0;
        return MockTermDocFreqs::next(docs, freqs);
    }

    bool next() {
        posCursor_ = 0;
        return MockTermDocFreqs::next();
    }

    virtual docid_t skipTo(docid_t target)
    {
        posCursor_ = 0;
        return MockTermDocFreqs::skipTo(target);
    }

    loc_t nextPosition()
    {
        loc_t pos = BAD_POSITION;
        if( posCursor_ < boost::get<3>(postings_[cursor_]).size() ) {
            pos = boost::get<3>(postings_[cursor_])[posCursor_];
            posCursor_++;
        }
        return pos;
    }

    int32_t nextPositions(loc_t*& positions)
    {
        if( posCursor_ < boost::get<3>(postings_[cursor_]).size() ) {
            positions = &boost::get<3>(postings_[cursor_])[posCursor_];
            posCursor_++;
            return 1;
        }
        return 0;
    }

protected:
    size_t posCursor_;
};

}

NS_IZENELIB_IR_END

#endif
