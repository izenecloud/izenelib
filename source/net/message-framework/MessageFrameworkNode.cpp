#include <net/message-framework/MessageFrameworkNode.h>

#include <boost/functional/hash.hpp>


namespace messageframework
{

        std::size_t MessageFrameworkNode::hash() const
        {
            std::size_t seed = 0;
            boost::hash_combine( seed, nodeIP_ );
            boost::hash_combine( seed, nodePort_ );
            boost::hash_combine( seed, nodeName_ );
            return seed;
        }
}
