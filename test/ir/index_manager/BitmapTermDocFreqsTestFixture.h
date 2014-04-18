#ifndef BITMAP_TERM_DOC_FREQS_TEST_FIXTURE_H
#define BITMAP_TERM_DOC_FREQS_TEST_FIXTURE_H

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/utility/Bitset.h>
#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

template <class bitmap_t, class iter_t>
class BitmapTermDocFreqsTestFixture
{
public:
    typedef izenelib::ir::indexmanager::docid_t docid_t;

    BitmapTermDocFreqsTestFixture();

    void testDocFreq();

    void testNext();

    void testSkipTo();

private:
    std::vector<docid_t> docids_;
    boost::scoped_ptr<iter_t> iter_;
};

template <class bitmap_t, class iter_t>
BitmapTermDocFreqsTestFixture<bitmap_t, iter_t>::BitmapTermDocFreqsTestFixture()
{
    docid_t docids[] = {1, 3, 4, 10, 15, 200, 355, 489, 678, 1234};
    int num = sizeof(docids) / sizeof(docids[0]);
    boost::shared_ptr<bitmap_t> pBitMap(new bitmap_t);

    for (int i = 0; i < num; ++i)
    {
        docid_t docId = docids[i];

        docids_.push_back(docId);
        pBitMap->set(docId);
    }

    iter_.reset(new iter_t(pBitMap));
}

template <class bitmap_t, class iter_t>
void BitmapTermDocFreqsTestFixture<bitmap_t, iter_t>::testDocFreq()
{
    BOOST_CHECK_EQUAL(iter_->docFreq(), docids_.size());
}

template <class bitmap_t, class iter_t>
void BitmapTermDocFreqsTestFixture<bitmap_t, iter_t>::testNext()
{
    for (std::vector<docid_t>::const_iterator it = docids_.begin();
         it != docids_.end(); ++it)
    {
        BOOST_CHECK(iter_->next());
        BOOST_CHECK_EQUAL(iter_->doc(), *it);
    }

    BOOST_CHECK(iter_->next() == false);
    BOOST_CHECK_EQUAL(iter_->doc(), BAD_DOCID);
}

template <class bitmap_t, class iter_t>
void BitmapTermDocFreqsTestFixture<bitmap_t, iter_t>::testSkipTo()
{
    docid_t skipTestData[][2] = {
        {1, 1},       // hit
        {2, 3},       // skip
        {4, 4},       // hit
        {5, 10},      // skip
        {15, 15},     // hit
        {200, 200},   // hit
        {201, 355},   // skip
        {489, 489},   // hit
        {500, 678},   // skip
        {1000, 1234}, // skip
        {2000, BAD_DOCID}, // end
        {3000, BAD_DOCID}  // end
    };

    int num = sizeof(skipTestData) / sizeof(skipTestData[0]);
    for (int i = 0; i < num; ++i)
    {
        docid_t target = skipTestData[i][0];
        docid_t gold = skipTestData[i][1];
        BOOST_TEST_MESSAGE("target: " << target << ", gold: " << gold);

        BOOST_CHECK_EQUAL(iter_->skipTo(target), gold);
        BOOST_CHECK_EQUAL(iter_->doc(), gold);
    }

    BOOST_CHECK(iter_->next() == false);
    BOOST_CHECK_EQUAL(iter_->doc(), BAD_DOCID);
}

#endif // BITMAP_TERM_DOC_FREQS_TEST_FIXTURE_H
