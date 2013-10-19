/**
 * This is code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */

#ifndef CODECFACTORY_H_
#define CODECFACTORY_H_

#include "common.h"
#include "codecs.h"
#include "util.h"
#include "fastpfor.h"
#include "simdfastpfor.h"
#include "variablebyte.h"
#include "compositecodec.h"
#include "blockpacking.h"
#include "simdbinarypacking.h"
#include <boost/shared_ptr.hpp>
using namespace std;

class CODECFactory
{
public:
    map<string, boost::shared_ptr<IntegerCODEC> > scodecmap;

    CODECFactory()
    {
        scodecmap["fastpfor"]= boost::shared_ptr<IntegerCODEC> (new CompositeCodec<FastPFor , VariableByte> ());
        scodecmap["simdfastpfor"]= boost::shared_ptr<IntegerCODEC> (new CompositeCodec<SIMDFastPFor , VariableByte> ());
        scodecmap["simdbinarypacking"]=boost::shared_ptr<IntegerCODEC>(new CompositeCodec<SIMDBinaryPacking,VariableByte>());
    }

    vector<boost::shared_ptr<IntegerCODEC> > allSchemes()
    {
        vector < boost::shared_ptr<IntegerCODEC> > ans;
        map<string, boost::shared_ptr<IntegerCODEC> >::iterator i;
        for (i = scodecmap.begin(); i != scodecmap.end(); ++i)
        {
            ans.push_back(i->second);
        }
        return ans;
    }

    vector<string> allNames()
    {
        vector < string > ans;
        map<string, boost::shared_ptr<IntegerCODEC> >::iterator i;
        for (i = scodecmap.begin(); i != scodecmap.end(); ++i)
        {
            ans.push_back(i->first);
        }
        return ans;
    }

    boost::shared_ptr<IntegerCODEC> & getFromName(string name)
    {
        if (scodecmap.find(name) == scodecmap.end())
        {
            cerr << "name " << name << " does not refer to a CODEC." << endl;
            cerr << "possible choices:" << endl;
            map<string, boost::shared_ptr<IntegerCODEC> >::iterator i;
            for (i = scodecmap.begin(); i != scodecmap.end(); ++i)
            {
                cerr << static_cast<string> (i->first) << endl;// useless cast, but just to be clear
            }
            cerr << "for now, I'm going to just return 'copy'" << endl;
            return scodecmap["copy"];
        }
        return scodecmap[name];
    }

};

#endif /* CODECFACTORY_H_ */
