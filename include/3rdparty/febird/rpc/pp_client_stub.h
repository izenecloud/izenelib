/* vim: set tabstop=4 : */
#define ArgCount BOOST_PP_ITERATION()
#define ArgDeclare(z, n, d)   Arg##n a##n d
#define PP_ArgListRef(z, n, d)  typename ArgVal<Arg##n>::xref_t a##n d
#define PP_ArgListVal(z, n, d)  typename ArgVal<Arg##n>::val_t a##n d

#define PP_ArgListDeRef(z, n, d)  ArgVal<Arg##n>::deref(d##n)
//#define PP_ArgListGetVal(z, n, d)  ArgVal<Arg##n>::val(d##n)

template<class Client, class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class client_packet<Client,
	 arglist_ref<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>,
				 rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
	: public client_packet_io<Client,
			rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg)),
void (ThisType::*)(const client_packet_base&BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg))
>{
public:
	typedef arglist_ref<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))> aref_t;
	aref_t  refs;

	client_packet(ThisType* self BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListRef, BOOST_PP_EMPTY()))
		: refs(*self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a))
	{}

	explicit client_packet(aref_t& argrefs) : refs(argrefs) { }

	rpc_ret_t call_byid(rpc_client_basebase* client)
	{
		return this->t_call_byid(client, refs);
	}
	rpc_ret_t call_byname(rpc_client_basebase* client)
	{
		return this->t_call_byname(client, refs);
	}
	void send_id_args(rpc_client_basebase* client)
	{
		return this->t_send_id_args(client, refs);
	}
	void send_nm_args(rpc_client_basebase* client)
	{
		return this->t_send_nm_args(client, refs);
	}
	void read_args(rpc_client_basebase* client)
	{
		return this->t_read_args(client, refs);
	}
	virtual void on_return()
	{
		assert(0 != this->on_ret);
		rpc_client_basebase* client = (rpc_client_basebase*)refs.self->get_ext_ptr();
		this->t_read_args(client, refs);
		(refs.self.get()->*this->on_ret)(*this BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListDeRef, refs.a));
	}
};

template<class Client, class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class client_packet<Client,
	 arglist_ref<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>*,
				 rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
	: public client_packet_io<Client,
			rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg)),
void (ThisType::*)(const client_packet_base&BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg))
>{
public:
	typedef arglist_ref<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))> aref_t;
	aref_t* refs;

	explicit client_packet(void* refs) : refs((aref_t*)refs) {}

	rpc_ret_t call_byid(rpc_client_basebase* client)
	{
		return this->t_call_byid(client, *refs);
	}
	rpc_ret_t call_byname(rpc_client_basebase* client)
	{
		return this->t_call_byname(client, *refs);
	}
	void send_id_args(rpc_client_basebase* client)
	{
		return this->t_send_id_args(client, *refs);
	}
	void send_nm_args(rpc_client_basebase* client)
	{
		return this->t_send_nm_args(client, *refs);
	}
	void read_args(rpc_client_basebase* client)
	{
		return this->t_read_args(client, *refs);
	}
	virtual void on_return()
	{
		assert(this->on_ret);
		rpc_client_basebase* client = (rpc_client_basebase*)refs->self->get_ext_ptr();
		this->t_read_args(client, *refs);
		(refs->self.get()->*this->on_ret)(*this BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListDeRef, refs->a));
	}
};

// hold args references
template<class Client, class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class client_packet<Client,
	 arglist_val<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>,
				 rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
	: public client_packet_io<Client,
				 rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg)),
void (ThisType::*)(const client_packet_base&BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg))
>{
public:
	typedef arglist_ref<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))> aref_t;
	typedef arglist_val<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))> aval_t;
	aval_t  vals;

	client_packet(ThisType* self BOOST_PP_ENUM_TRAILING(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
		: vals(*self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a))
	{}
	explicit client_packet(aref_t& argrefs)
		: vals(boost::mpl::true_(), argrefs)
	{
	}

	rpc_ret_t call_byid(rpc_client_basebase* client)
	{
		aref_t refs(boost::mpl::true_(), vals);
		return this->t_call_byid(client, refs);
	}
	rpc_ret_t call_byname(rpc_client_basebase* client)
	{
		aref_t refs(boost::mpl::true_(), vals);
		return this->t_call_byname(client, refs);
	}
	void send_id_args(rpc_client_basebase* client)
	{
		aref_t refs(boost::mpl::true_(), vals);
		return this->t_send_id_args(client, refs);
	}
	void send_nm_args(rpc_client_basebase* client)
	{
		aref_t refs(boost::mpl::true_(), vals);
		return this->t_send_nm_args(client, refs);
	}
	void read_args(rpc_client_basebase* client)
	{
		aref_t refs(boost::mpl::true_(), vals);
		return this->t_read_args(client, refs);
	}
	virtual void on_return()
	{
		assert(this->on_ret);
		aref_t refs(boost::mpl::true_(), vals);
		rpc_client_basebase* client = (rpc_client_basebase*)vals.self->get_ext_ptr();
		this->t_read_args(client, refs);
		(refs.self.get()->*this->on_ret)(*this BOOST_PP_ENUM_TRAILING(ArgCount, PP_ArgListDeRef, refs.a));
	}
};

//#undef PP_ArgListGetVal
#undef PP_ArgListDeRef

template<class ThisType BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, class Arg)>
class client_stub_ref<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))>
{
	typedef void (ThisType::*myfun_t)(const client_packet_base&BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg));
	typedef arglist_ref<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))> aref_t;
	typedef arglist_val<rpc_ret_t (ThisType::*)(BOOST_PP_ENUM_PARAMS(ArgCount, Arg))> aval_t;
	typedef client_packet_fun<myfun_t> packet_fun_t;
	myfun_t on_return;
	ThisType* m_self;
	client_stub_i* m_meta;

public:
	client_stub_ref() : m_meta(0), m_self(0), on_return(0) {}

	template<class ThisDerived>
	void set_async_callback(void (ThisDerived::*pf)(const client_packet_base&
							BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, Arg)))
	{
		if (false)
		{	// only do compile-time check, this sentence will be eliminated in obj-code
			ThisType* pbase = (ThisDerived*)NULL;
		}
		on_return = static_cast<myfun_t>(pf);
	}

	void bind(ThisType* self, client_stub_i* impl)
	{
		m_meta = impl;
		m_self = self;
	}

	rpc_ret_t operator()(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
	{
		aref_t argrefs(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		rpc_client_basebase* client = (rpc_client_basebase*)m_self->get_ext_ptr();
		if (m_self->test_flags(rpc_call_byid))
			return m_meta->call_byid(client, &argrefs);
		else
			return m_meta->call_byname(client, &argrefs);
	}

	rpc_ret_t byid(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
	{
		aref_t argrefs(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		rpc_client_basebase* client = (rpc_client_basebase*)m_self->get_ext_ptr();
		return m_meta->call_byid(client, &argrefs);
	}
	rpc_ret_t byname(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
	{
		aref_t argrefs(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		rpc_client_basebase* client = (rpc_client_basebase*)m_self->get_ext_ptr();
		return m_meta->call_byname(client, &argrefs);
	}

	void async(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
	{
		aref_t args(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		rpc_client_basebase* client = (rpc_client_basebase*)m_self->get_ext_ptr();
		boost::intrusive_ptr<client_packet_base> packet = m_meta->valpacket_create(&args);
		packet_fun_t* p = (packet_fun_t*)(packet.get());
		assert(on_return);
		p->on_ret = on_return;
		packet->stub = this->m_meta;
		packet->how_call = rpc_call_asynch_ordered;
		if (m_self->test_flags(rpc_call_byid))
			m_meta->send_id_args(client, packet.get());
		else
			m_meta->send_nm_args(client, packet.get());
		client->put_packet(packet);
	}
	void async_byid(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
	{
		aref_t args(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		boost::intrusive_ptr<client_packet_base> packet = m_meta->valpacket_create(&args);
		rpc_client_basebase* client = (rpc_client_basebase*)m_self->get_ext_ptr();
		assert(on_return);
		packet_fun_t* p = (packet_fun_t*)(packet.get());
		p->on_ret = on_return;
		packet->stub = this->m_meta;
		packet->how_call = rpc_call_asynch_ordered;
		m_meta->send_id_args(client, packet.get());
		client->put_packet(packet);
	}
	void async_byname(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
	{
		aref_t args(m_self BOOST_PP_ENUM_TRAILING_PARAMS(ArgCount, a));
		boost::intrusive_ptr<client_packet_base> packet = m_meta->valpacket_create(&args);
		rpc_client_basebase* client = (rpc_client_basebase*)m_self->get_ext_ptr();
		packet_fun_t* p = (packet_fun_t*)(packet.get());
		assert(on_return);
		p->on_ret = on_return;
		packet->stub = this->m_meta;
		packet->how_call = rpc_call_asynch_ordered;
		m_meta->send_nm_args(client, packet.get());
		client->put_packet(packet);
	}

	rpc_ret_t reap(BOOST_PP_ENUM(ArgCount, ArgDeclare, BOOST_PP_EMPTY()))
	{
		return this->retv;
	}
};

#undef PP_ArgListVal
#undef PP_ArgListRef
#undef ArgDeclare
#undef ArgCount

