#include <boost/memory.hpp>

NS_BOOST_MEMORY_BEGIN
tls_block_pool_t g_tls_blockPool;
	
tls_block_pool_init g_tls_blockPoolInit;

tls_block_pool_t* _boost_TlsBlockPool(){return &g_tls_blockPool;}
NS_BOOST_MEMORY_END

