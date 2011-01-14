#include <stdio.h>
#include <3rdparty/rdf/rdf_parser.h>

int main(int argc, char *argv[])
{
  rdf::RdfParser parser;
  parser.ParseFile(argv[1]);

  return 0;
}