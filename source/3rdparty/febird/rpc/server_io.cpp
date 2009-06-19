/* vim: set tabstop=4 : */
#include <febird/rpc/rpc_basic.h>

namespace febird { namespace rpc {

void FEBIRD_DLL_EXPORT
incompitible_class_cast(remote_object* y, const char* szRequestClassName)
{
	std::ostringstream oss;
	oss << "object[id=" << y->getID() << ", type=" << y->getClassName()
		<< "] is not " << szRequestClassName;
	throw rpc_exception(oss.str());
}


} } // namespace::febird::rpc
