#include <string>
#include <fstream>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <am/graphchi/graphchi_basic_includes.hpp>
#include <am/graphchi/engine/dynamic_graphs/graphchi_dynamicgraph_engine.hpp>
#include <am/graphchi/api/functional/functional_api.hpp>

using namespace graphchi;
namespace bfs = boost::filesystem;

#define DIR_PREFIX "./tmp/am_graphchi_"

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program. 
 */
typedef vid_t VertexDataType;
typedef vid_t EdgeDataType;

/**
 * Smoke test. On every iteration, each vertex sets its id to be
 * id + iteration number. Vertices check whether their neighbors were
 * set correctly. This assumes that the vertices are executed in round-robin order.
 */
struct SmokeTestProgram : public GraphChiProgram<VertexDataType, EdgeDataType> {
    /**
     *  Vertex update function.
     */
    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext) {
        if (gcontext.iteration == 0) {
            for(int i=0; i < vertex.num_outedges(); i++) {
                vertex.outedge(i)->set_data(vertex.id());        
            }
        } else {
            for(int i=0; i < vertex.num_inedges(); i++) {
                graphchi_edge<vid_t> * edge = vertex.inedge(i);
                vid_t inedgedata = edge->get_data();
                vid_t expected = edge->vertex_id() + gcontext.iteration - (edge->vertex_id() > vertex.id());
                if (inedgedata != expected) {
                    assert(false);
                }
            }
            for(int i=0; i < vertex.num_outedges(); i++) {
                vertex.outedge(i)->set_data(vertex.id() + gcontext.iteration);
            }
        }
        vertex.set_data(gcontext.iteration + 1);
    }
    
    /**
     * Called before an iteration starts.
     */
    void before_iteration(int iteration, graphchi_context &gcontext) {
    }
    
    /**
     * Called after an iteration has finished.
     */
    void after_iteration(int iteration, graphchi_context &gcontext) {
    }
    
    /**
     * Called before an execution interval is started.
     */
    void before_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext) {        
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext) {        
    }
    
};

/**
  * Vertex callback that checks the vertex data is ok.
  */
class VertexDataChecker : public VCallback<VertexDataType> {
    int iters;
public:
    size_t total;

    VertexDataChecker(int iters) : iters(iters), total(0) {}
    void callback(vid_t vertex_id, VertexDataType &vecvalue) {
        assert(vecvalue == (VertexDataType)iters);
        total += (size_t) iters;
    }
};


struct BulksyncSmokeTestProgram : public functional_kernel<int, int> {
    
    /* Initial value - on first iteration */
    int initial_value(graphchi_context &info, vertex_info& myvertex) {
        return 0;
    }
    
    /* Called before first "gather" */
    int reset() {
        return 0;
    }
    
    // Note: Unweighted version, edge value should also be passed
    // "Gather"
    int op_neighborval(graphchi_context &info, vertex_info& myvertex, vid_t nbid, int nbval) {
        assert(nbval == (int) info.iteration - 1);
        return nbval;
    }
    
    // "Sum"
    int plus(int curval, int toadd) {
        assert(curval == 0 || toadd == curval);
        return toadd;
    }
    
    // "Apply"
    int compute_vertexvalue(graphchi_context &ginfo, vertex_info& myvertex, int nbvalsum) {
        return ginfo.iteration;
    }
    
    // "Scatter
    int value_to_neighbor(graphchi_context &info, vertex_info& myvertex, vid_t nbid, int myval) {
        assert(myval == (int) info.iteration);
        return myval;
    }
    
}; 



BOOST_AUTO_TEST_SUITE( graphchi_smoke_suite )

BOOST_AUTO_TEST_CASE(graphchi_simple)
{
    metrics m("smoketest");
    
    /* Basic arguments for application */
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string filename = db_dir.string() + "/graphchi_smoke";
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
    
    /* Detect the number of shards or preprocess an input to creae them */
    int nshards = convert_if_notexists<EdgeDataType>(filename, 
                                                              get_option_string("nshards", "auto"));
        
    /* Run */
    SmokeTestProgram program;
    graphchi_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m); 
    engine.run(program, niters);
        
    /* Check also the vertex data is ok */
    VertexDataChecker vchecker(niters);
    foreach_vertices(filename, 0, engine.num_vertices(), vchecker);
    assert(vchecker.total == engine.num_vertices() * niters);
    
    /* Report execution metrics */
    metrics_report(m);
    
    logstream(LOG_INFO) << "Smoketest passed successfully! Your system is working!" << std::endl;
}


BOOST_AUTO_TEST_CASE(graphchi_dynamic_engine)
{
    metrics m("smoketest-dynamic-engine");
    
    /* Basic arguments for application */
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string filename = db_dir.string() + "/graphchi_dynamic_smoke";
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
    
    /* Detect the number of shards or preprocess an input to creae them */
    int nshards = convert_if_notexists<EdgeDataType>(filename, 
                                                              get_option_string("nshards", "auto"));
        
    /* Run */
    SmokeTestProgram program;
    graphchi_dynamicgraph_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m); 
    engine.run(program, niters);
        
    /* Check also the vertex data is ok */
    VertexDataChecker vchecker(niters);
    foreach_vertices(filename, 0, engine.num_vertices(), vchecker);
    assert(vchecker.total == engine.num_vertices() * niters);
    
    /* Report execution metrics */
    metrics_report(m);
    
    logstream(LOG_INFO) << "Dynamic Engine Smoketest passed successfully! Your system is working!" << std::endl;
}


BOOST_AUTO_TEST_CASE(graphchi_bulksync)
{
    metrics m("smoketest-functional");
    
    /* Basic arguments for application */
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string filename = db_dir.string() + "/graphchi_bulksync_smoke";
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
    std::string mode = get_option_string("mode", "semisync");
    
    logstream(LOG_INFO) << "Running bulk sync smoke test." << std::endl;
    run_functional_unweighted_synchronous<BulksyncSmokeTestProgram>(filename, niters, m);
    logstream(LOG_INFO) << "Smoketest passed successfully! Your system is working!" << std::endl;
}


BOOST_AUTO_TEST_SUITE_END()

