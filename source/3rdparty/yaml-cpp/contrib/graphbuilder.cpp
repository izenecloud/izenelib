#include <3rdparty/yaml-cpp/parser.h>
#include <3rdparty/yaml-cpp/contrib/graphbuilder.h>
#include "graphbuilderadapter.h"

namespace YAML
{
  void *BuildGraphOfNextDocument(Parser& parser, GraphBuilderInterface& graphBuilder)
  {
    GraphBuilderAdapter eventHandler(graphBuilder);
    if (parser.HandleNextDocument(eventHandler)) {
      return eventHandler.RootNode();
    } else {
      return NULL;
    }
  }
}
