
/**
 * @file
 * @author  Aapo Kyrola <akyrola@cs.cmu.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * Copyright [2012] [Aapo Kyrola, Guy Blelloch, Carlos Guestrin / Carnegie Mellon University]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

 *
 * @section DESCRIPTION
 *
 * Command line options.
 */

#ifndef GRAPHCHI_CMDOPTS_DEF
#define GRAPHCHI_CMDOPTS_DEF


#include <string>
#include <iostream>
#include <stdint.h>
#include <map>

#include <am/graphchi/api/chifilenames.hpp>
#include <util/singleton.h>

namespace graphchi
{

/** GNU COMPILER HACK TO PREVENT IT FOR COMPILING METHODS WHICH ARE NOT USED IN
    THE PARTICULAR APP BEING BUILT */
#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif

struct graphchiconfig
{
    graphchiconfig()
    {
        //conf["execthreads"]= "2";
        conf["loadthreads"] = "2";
        conf["niothreads"] = "2";
        conf["membudget_mb"] = "800";
        conf["preload.max_megabytes"] = "300";
        conf["max_edgebuffer_mb"] = "1000";
        conf["io.blocksize"] = "1048576";
        conf["filetype"] = "edgelist";  ///"adjlist" for adjacencylist format
        conf["metrics.reporter"] = "console,file,html";
    }
    static graphchiconfig* get()
    {
        return ::izenelib::util::Singleton<graphchiconfig>::get();
    }

    std::map<std::string, std::string> conf;
};

static std::string VARIABLE_IS_NOT_USED get_config_option_string(const char *option_name)
{
    if (graphchiconfig::get()->conf.find(option_name) != graphchiconfig::get()->conf.end())
    {
        return graphchiconfig::get()->conf[option_name];
    }
    else
    {
        std::cout << "ERROR: could not find option " << option_name << " from config.";
        assert(false);
    }
}

static std::string VARIABLE_IS_NOT_USED get_config_option_string(const char *option_name,
        std::string default_value)
{
    if (graphchiconfig::get()->conf.find(option_name) != graphchiconfig::get()->conf.end())
    {
        return graphchiconfig::get()->conf[option_name];
    }
    else
    {
        return default_value;
    }

}
static int VARIABLE_IS_NOT_USED get_config_option_int(const char *option_name, int default_value)
{
    if (graphchiconfig::get()->conf.find(option_name) != graphchiconfig::get()->conf.end())
    {
        return atoi(graphchiconfig::get()->conf[option_name].c_str());
    }
    else
    {
        return default_value;
    }
}

static uint64_t VARIABLE_IS_NOT_USED get_config_option_long(const char *option_name, uint64_t default_value)
{
    if (graphchiconfig::get()->conf.find(option_name) != graphchiconfig::get()->conf.end())
    {
        return atol(graphchiconfig::get()->conf[option_name].c_str());
    }
    else
    {
        return default_value;
    }
}
static double VARIABLE_IS_NOT_USED get_config_option_double(const char *option_name, double default_value)
{
    if (graphchiconfig::get()->conf.find(option_name) != graphchiconfig::get()->conf.end())
    {
        return atof(graphchiconfig::get()->conf[option_name].c_str());
    }
    else
    {
        return default_value;
    }
}

static std::string VARIABLE_IS_NOT_USED get_option_string(const char *option_name,
        std::string default_value)
{
    return get_config_option_string(option_name, default_value);
}

static int VARIABLE_IS_NOT_USED get_option_int(const char *option_name, int default_value)
{
    return get_config_option_int(option_name, default_value);
}

static uint64_t VARIABLE_IS_NOT_USED get_option_long(const char *option_name, uint64_t default_value)
{
    return get_config_option_long(option_name, default_value);
}

static float VARIABLE_IS_NOT_USED get_option_float(const char *option_name, float default_value)
{
    return (float) get_config_option_double(option_name, default_value);
}

} // End namespace


#endif


