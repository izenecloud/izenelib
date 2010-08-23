#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include <ir/index_manager/utility/PriorityQueue.h>

using namespace std;
using namespace boost;

boost::mt19937 engine ;
boost::uniform_real<float> distribution(0.0, 1.0);
boost::variate_generator<boost::mt19937&, boost::uniform_real<float> > gen(engine, distribution);

struct ScoreDoc
{
    unsigned int id;
    float score;
};

class ScoreDocQueue: public izenelib::ir::indexmanager::PriorityQueue<ScoreDoc*>{
public:
	ScoreDocQueue(int32_t count)
	{ 
		initialize(count,false);
	}
protected:
	bool lessThan(ScoreDoc* a, ScoreDoc* b) {
		return (a->score < b->score);
	}
};

BOOST_AUTO_TEST_SUITE( t_priorityqueue )

BOOST_AUTO_TEST_CASE(prorityqueue)
{
    size_t count = 1000;
    ScoreDocQueue queue(count);

    for (size_t i = 0; i < count; i++) 
    {
        ScoreDoc* doc = new ScoreDoc;
        doc->id = i;
        doc->score = gen();
        queue.put( doc );
    }
    float last = -1.0F;
    for (size_t j = 0; j < count; j++) {
        ScoreDoc* doc = queue.pop();
        float next = doc->score;
        delete doc;
        BOOST_CHECK(next>=last);
        last = next;
    }

}

BOOST_AUTO_TEST_SUITE_END()

