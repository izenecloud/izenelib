#include <string>
#include <fstream>
#include <cmath>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <am/graphchi/graphchi_basic_includes.hpp>
#include <am/graphchi/engine/dynamic_graphs/graphchi_dynamicgraph_engine.hpp>
#include <am/graphchi/api/functional/functional_api.hpp>
#include <am/graphchi/util/toplist.hpp>

using namespace graphchi;
namespace bfs = boost::filesystem;

#define DIR_PREFIX "./tmp/am_graphchi_"

#define THRESHOLD 1e-1    
#define RANDOMRESETPROB 0.15


typedef float VertexDataType;
typedef float EdgeDataType;

struct PagerankProgram : public GraphChiProgram<VertexDataType, EdgeDataType> {
    
    /**
      * Called before an iteration starts. Not implemented.
      */
    void before_iteration(int iteration, graphchi_context &info) {
    }
    
    /**
      * Called after an iteration has finished. Not implemented.
      */
    void after_iteration(int iteration, graphchi_context &ginfo) {
    }
    
    /**
      * Called before an execution interval is started. Not implemented.
      */
    void before_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &ginfo) {        
    }
    
    
    /**
      * Pagerank update function.
      */
    void update(graphchi_vertex<VertexDataType, EdgeDataType> &v, graphchi_context &ginfo) {
        float sum=0;
        if (ginfo.iteration == 0) {
            /* On first iteration, initialize vertex and out-edges. 
               The initialization is important,
               because on every run, GraphChi will modify the data in the edges on disk. 
             */
            for(int i=0; i < v.num_outedges(); i++) {
                graphchi_edge<float> * edge = v.outedge(i);
                edge->set_data(1.0 / v.num_outedges());
            }
            v.set_data(RANDOMRESETPROB); 
        } else {
            /* Compute the sum of neighbors' weighted pageranks by
               reading from the in-edges. */
            for(int i=0; i < v.num_inedges(); i++) {
                float val = v.inedge(i)->get_data();
                sum += val;                    
            }
            
            /* Compute my pagerank */
            float pagerank = RANDOMRESETPROB + (1 - RANDOMRESETPROB) * sum;
            
            /* Write my pagerank divided by the number of out-edges to
               each of my out-edges. */
            if (v.num_outedges() > 0) {
                float pagerankcont = pagerank / v.num_outedges();
                for(int i=0; i < v.num_outedges(); i++) {
                    graphchi_edge<float> * edge = v.outedge(i);
                    edge->set_data(pagerankcont);
                }
            }
                
            /* Keep track of the progression of the computation.
               GraphChi engine writes a file filename.deltalog. */
            ginfo.log_change(std::abs(pagerank - v.get_data()));
            
            /* Set my new pagerank as the vertex value */
            v.set_data(pagerank); 
        }
    }
    
};

struct pagerank_kernel : public functional_kernel<float, float> {
    
    /* Initial value - on first iteration */
    float initial_value(graphchi_context &info, vertex_info& myvertex) {
        return 1.0;
    }
    
    /* Called before first "gather" */
    float reset() {
        return 0.0;
    }
    
    // Note: Unweighted version, edge value should also be passed
    // "Gather"
    float op_neighborval(graphchi_context &info, vertex_info& myvertex, vid_t nbid, float nbval) {
        return nbval;
    }
    
    // "Sum"
    float plus(float curval, float toadd) {
        return curval + toadd;
    }
    
    // "Apply"
    float compute_vertexvalue(graphchi_context &ginfo, vertex_info& myvertex, float nbvalsum) {
        assert(ginfo.nvertices > 0);
        return RANDOMRESETPROB / ginfo.nvertices + (1 - RANDOMRESETPROB) * nbvalsum;
    }
    
    // "Scatter
    float value_to_neighbor(graphchi_context &info, vertex_info& myvertex, vid_t nbid, float myval) {
        assert(myvertex.outdegree > 0);
        return myval / myvertex.outdegree; 
    }
    
}; 



BOOST_AUTO_TEST_SUITE( graphchi_app_pagerank_suite )

BOOST_AUTO_TEST_CASE(graphchi_pagerank)
{
    metrics m("pagerank");
    
    /* Basic arguments for application */
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string filename = db_dir.string() + "/graphchi_pagerank";
    ///generate fake inputs
    std::ofstream of(filename.c_str());
    const unsigned int size = 8;
    const unsigned int src[size] = {1,2,3,4,5,6,7,8};
    const unsigned int dest[size] = {4,2,3,1,3,2,8,1};	
    const float weight[size] = {0.1,0.2,0.3,0.1,0.5,0.6,0.8,1};
    for(unsigned int i = 0; i < size; ++i)
        of<<src[i]<<" "<<dest[i]<<" "<<weight[i]<<std::endl;
    of.close();
    
    int niters = get_option_int("niters", 4); // Number of iterations
    bool scheduler = false;                       // Whether to use selective scheduling

    int ntop = get_option_int("top", 20);
    
    /* Detect the number of shards or preprocess an input to creae them */
    int nshards = convert_if_notexists<EdgeDataType>(filename, 
                                                              get_option_string("nshards", "auto"));

    /* Run */
    graphchi_engine<float, float> engine(filename, nshards, scheduler, m); 
    engine.set_modifies_inedges(false); // Improves I/O performance.
    PagerankProgram program;
    engine.run(program, niters);
        
    /* Output top ranked vertices */
    std::vector< vertex_value<float> > top = get_top_vertices<float>(filename, ntop);
    std::cout << "Print top " << ntop << " vertices:" << std::endl;
    for(int i=0; i < (int)top.size(); i++) {
        std::cout << (i+1) << ". " << top[i].vertex << "\t" << top[i].value << std::endl;
    }
    metrics_report(m);
    
    logstream(LOG_INFO) << "Pagerank executed successfully!" << std::endl;
}

BOOST_AUTO_TEST_CASE(graphchi_pagerank_functional)
{
    metrics m("pagerank-functional");
    
    /* Basic arguments for application */
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string filename = db_dir.string() + "/graphchi_pagerank-functional";
    ///generate fake inputs
    std::ofstream of(filename.c_str());
    const unsigned int size = 8;
    const unsigned int src[size] = {1,2,3,4,5,6,7,8};
    const unsigned int dest[size] = {4,2,3,1,3,2,8,1};	
    const float weight[size] = {0.1,0.2,0.3,0.1,0.5,0.6,0.8,1};
    for(unsigned int i = 0; i < size; ++i)
        of<<src[i]<<" "<<dest[i]<<" "<<weight[i]<<std::endl;
    of.close();
    
    int niters = get_option_int("niters", 4);
    int ntop = get_option_int("top", 20);
    std::string mode = get_option_string("mode", "semisync");
	
    logstream(LOG_INFO) << "Running pagerank functional." << std::endl;
    run_functional_unweighted_synchronous<pagerank_kernel>(filename, niters, m);
    logstream(LOG_INFO) << "Pagerank functional passed successfully! Your system is working!" << std::endl;
    /* Write Top 20 */
    std::vector< vertex_value<float> > top = get_top_vertices<float>(filename, ntop);
    std::cout << "Print top 20 vertices: " << std::endl;
    for(int i=0; i < (int) top.size(); i++) {
        std::cout << (i+1) << ". " << top[i].vertex << "\t" << top[i].value << std::endl;
    }
	
}

BOOST_AUTO_TEST_SUITE_END()

