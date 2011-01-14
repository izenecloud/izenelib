#ifndef IZENE_3RDPARTY_RDF_RDFPARSER_H_
#define IZENE_3RDPARTY_RDF_RDFPARSER_H_

#include <stdio.h>
#include <types.h>

#include <string>

#include <boost/function.hpp>
#include <3rdparty/rdf/raptor.h>
#include <3rdparty/rdf/raptor_internal.h>
namespace rdf{

class RdfParser
{
public:
//   typedef struct raptor_iostream_s raptor_iostream;
  RdfParser();
  ~RdfParser();
  void ParseContent(const std::string& content);
  void ParseFile(const std::string& file);
  
  static void print_triple(void* user_data, raptor_statement* triple) 
  {
    raptor_statement_print_as_ntriples(triple, stdout);
    fputc('\n', stdout);
  }

};

}
#endif  // IZENE_3RDPARTY_RDF_RDFPARSER_H_
