#include <iostream>
#include <sstream>
#include <3rdparty/rdf/rdf_parser.h>

using namespace rdf;

RdfParser::RdfParser()
{
}

RdfParser::~RdfParser()
{
}

void RdfParser::ParseContent(const std::string& content)
{
//   raptor_world *world = NULL;
//   raptor_parser* rdf_parser = NULL;
//   world = raptor_new_world();
// 
//   rdf_parser = raptor_new_parser(world, "rdfxml");
// 
//   raptor_parser_set_statement_handler(rdf_parser, NULL, RdfParser::print_triple);
// 
// //   uri_string = raptor_uri_filename_to_uri_string(file.c_str());
// //   uri = raptor_new_uri(world, uri_string);
// //   base_uri = raptor_uri_copy(uri);
//   raptor_iostream *iostr = new raptor_iostream();
//   raptor_iostream_string_write(content.c_str(), iostr);
//   raptor_parser_parse_iostream(rdf_parser, iostr, NULL);
//   delete iostr;
//   raptor_free_parser(rdf_parser);
// 
//   raptor_free_world(world);
}

void RdfParser::ParseFile(const std::string& file)
{
  raptor_world *world = NULL;
  raptor_parser* rdf_parser = NULL;
  unsigned char *uri_string;
  raptor_uri *uri, *base_uri;

  world = raptor_new_world();

  rdf_parser = raptor_new_parser(world, "rdfxml");

  raptor_parser_set_statement_handler(rdf_parser, NULL, RdfParser::print_triple);

  uri_string = raptor_uri_filename_to_uri_string(file.c_str());
  uri = raptor_new_uri(world, uri_string);
  base_uri = raptor_uri_copy(uri);

  raptor_parser_parse_file(rdf_parser, uri, base_uri);

  raptor_free_parser(rdf_parser);

  raptor_free_uri(base_uri);
  raptor_free_uri(uri);
  raptor_free_memory(uri_string);

  raptor_free_world(world);
}
