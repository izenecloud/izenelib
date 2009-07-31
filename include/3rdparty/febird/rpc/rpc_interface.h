BEGIN_RPC_ADD_MF_EX(my_self_t, "ServerMain")
	RPC_ADD_MF(createSessionObject)
	RPC_ADD_MF(createGlobaleObject)
	RPC_ADD_MF(createNamedGlobaleObject)
	RPC_ADD_MF(createNamedSessionObject)
	RPC_ADD_MF(retrieveGlobaleObject)
	RPC_ADD_MF(retrieveSessionObject)
	RPC_ADD_MF(removeSessionObject)
END_RPC_ADD_MF()

//! create an servant, and return its proxy. proxy.m_id will be got from server
RPC_DECLARE_MF_EX(createGlobaleObject, (var_uint32_t* servantID, const std::string& className), ;)
RPC_DECLARE_MF_EX(createSessionObject, (var_uint32_t* servantID, const std::string& className), ;)
RPC_DECLARE_MF_EX(createNamedSessionObject, (var_uint32_t* servantID, std::string& className, const std::string& instanceName), ;)

//! if failed, className take the existed className of the existed object
RPC_DECLARE_MF_EX(createNamedGlobaleObject, (var_uint32_t* servantID, std::string& className, const std::string& instanceName), ;)

RPC_DECLARE_MF_EX(retrieveGlobaleObject, (var_uint32_t* servantID, const std::string& instanceName), ;)
RPC_DECLARE_MF_EX(retrieveSessionObject, (var_uint32_t* servantID, const std::string& instanceName), ;)

RPC_DECLARE_MF_EX(removeSessionObject, (var_uint32_t servantID), ;)
