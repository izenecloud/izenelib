///
///  @file : MessageFrameworkNode.h
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


#if !defined(_MSGFRK_NODE_H)
#define _MSGFRK_NODE_H

#include <string>
#include <net/message-framework/MessageFrameworkErrorString.h>
#include <net/message-framework/MessageFrameworkException.h>
#include <boost/shared_ptr.hpp>

namespace messageframework
{
	/**
	 * @brief This class defines a node in a network. A node is represented by an IP and a port number.
	 */
    //TODO: MAY NEED A CONSTRUCTOR THAT GETS THE NAME, IP, AND PORT
	class MessageFrameworkNode {
	public:
		/**
		 * @brief Constructor of the node, initialize variables with default values
		 */
		MessageFrameworkNode(): nodeIP_(""), nodePort_(0), nodeName_(""){}

		/**
		 * @brief Construct the node with a given IP and a given port number.
		 */
		MessageFrameworkNode(const std::string& nodeIP, const unsigned int nodePort): nodeIP_(nodeIP), nodePort_(nodePort), nodeName_(""){}

		MessageFrameworkNode(const MessageFrameworkNode& inputNode):
			nodeIP_(inputNode.nodeIP_),
			nodePort_(inputNode.nodePort_),
			nodeName_(inputNode.nodeName_)
		{
		}

		MessageFrameworkNode& operator=(const MessageFrameworkNode& inputNode)
		{
			nodeIP_ = inputNode.nodeIP_;
			nodePort_ = inputNode.nodePort_;
			nodeName_ = inputNode.nodeName_;

			return *this;
		}
		
		bool operator==(const MessageFrameworkNode& inputNode) const
		{
			return ( nodeIP_ == inputNode.nodeIP_ ) && ( nodePort_ == inputNode.nodePort_ );
		}

		/**
		 * @brief Destructor of the node
		 */
		~MessageFrameworkNode(){			
			//std::cout<<" ~MessageFrameworkNode"<<std::endl;
		}


		/**
 		 * @brief clear data
 		 */
		void clear()
		{
			nodeIP_.clear(); nodePort_ = 0; nodeName_.clear();
		}

		template <typename Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & nodeIP_ & nodePort_ & nodeName_;
		}


        // @by MyungHyun - 2009-01-09
        /**
         * @brief   By using boost::hash the function returns the hash key value using the member variables
         */
        std::size_t hash() const;

        void display(std::ostream& os=std::cout) const {
        	os<<nodeName_<<" : ("<<nodeIP_<<" : "<<nodePort_<<")"<<std::endl;
        }

	public:

		/**
		 * @brief IP of the node
		 */
		std::string nodeIP_;

		/**
		 * @brief Port of the node
		 */
		unsigned int nodePort_;

		/**
		 * @brief Name of the node
		 */
		std::string nodeName_;
	};

	 inline std::ostream& operator<<(std::ostream& s, const MessageFrameworkNode& c)
	  {c.display(s); return s;}


	/**
 	 * @brief This class is used by std containers such as std::map to define the weak order
 	 * of MessageFrameworkNodes
 	 */
	class MessageFrameworkNode_less
	{
	public:
		/**
 	 	 * @brief This function is used by std containers such as std::map to define the weak order
 	     * of MessageFrameworkNodes
 	     * @param
 	     * node1 - the 1st node
 	     * @param
 	     * node2 - the 2nd node
 	     * @return
 	     * true if node1 < node2
 	     */
		bool operator()(const MessageFrameworkNode& node1, const MessageFrameworkNode& node2) const
		{
			if(node1.nodePort_ < node2.nodePort_)
				return true;
			else if(node1.nodePort_ > node2.nodePort_)
				return false;
			else if(node1.nodeIP_.compare(node2.nodeIP_) < 0)
				return true;

			return false;
		}
	};

	typedef boost::shared_ptr<MessageFrameworkNode> MessageFrameworkNodePtr;
}// end of messageframework





// @by MyungHyun - 2009-01-09
// Defines the method's namespace depending on the compiler
#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
namespace boost
#else
namespace messageframework
#endif
{
    inline std::size_t hash_value( const messageframework::MessageFrameworkNode & src )
    {
        return src.hash();
    }
}



#endif  // end of #define _MSGFRK_NODE_H
