#include <boost/memory.hpp>
#include <boost/detail/performance_counter.hpp>

#if defined(__GNUG__)
#include <ext/mt_allocator.h>
#endif

#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>
#include <string>

// -------------------------------------------------------------------------

enum { Total = 1000000 };

template <class LogT, class Type=int>
class TestAllocatorPerformance
{
private:
	NS_BOOST_DETAIL::accumulator m_acc;
	static Type* p[Total];

public:
	void doNewDelete(LogT& log, int NAlloc, int PerAlloc)
	{
		int i;
		NS_BOOST_DETAIL::performance_counter counter;
		{
			for (int j = 0; j < NAlloc; ++j)
			{
				for (i = 0; i < PerAlloc; ++i)
				{
					p[i] = new Type;
				}
// 				for (i = 0; i < PerAlloc; ++i)
// 				{
// 					delete p[i];
//				}
			}
		}
		m_acc.accumulate(counter.trace(log));
	}

#if defined(__GNUG__)
	void doMtAllocator(LogT& log, int NAlloc, int PerAlloc)
	{
		typedef __gnu_cxx::__mt_alloc<Type> allocator_type;
		typedef __gnu_cxx::__pool_base::_Tune tune_type;
		//tune_type tune(16, 5120, 32, 5120, 20, 10, false);

		int i;
		NS_BOOST_DETAIL::performance_counter counter;
		{
			for (int j = 0; j < NAlloc; ++j)
			{
				allocator_type alloc;
				//alloc._M_set_options(tune);
				for (i = 0; i < PerAlloc; ++i)
				{
					p[i] = new(alloc.allocate(1)) Type;
				}
				for (i = 0; i < PerAlloc; ++i)
				{
					p[i]->~Type();
					alloc.deallocate(p[i], 1);
				}
			}
		}
		m_acc.accumulate(counter.trace(log));
	}
#endif

	void doBoostPool(LogT& log, int NAlloc, int PerAlloc)
	{
		NS_BOOST_DETAIL::performance_counter counter;
		{
			for (int j = 0; j < NAlloc; ++j)
			{
				boost::pool<> alloc(sizeof(Type));
				for (int i = 0; i < PerAlloc; ++i)
				{
					Type* p = new(alloc.malloc()) Type;
					// need to call the destructor of Type!
				}
			}
		}
		m_acc.accumulate(counter.trace(log));
	}

	void doBoostObjectPool(LogT& log, int NAlloc, int PerAlloc)
	{
		NS_BOOST_DETAIL::performance_counter counter;
		{
			for (int j = 0; j < NAlloc; ++j)
			{
				boost::object_pool<Type> alloc;
				for (int i = 0; i < PerAlloc; ++i)
				{
					Type* p = alloc.construct();
				}
			}
		}
		m_acc.accumulate(counter.trace(log));
	}

	template <class LogT2>
	void doAutoAlloc(LogT2& log, int NAlloc, int PerAlloc)
	{
		NS_BOOST_DETAIL::performance_counter counter;
		{
			for (int j = 0; j < NAlloc; ++j)
			{
				boost::auto_alloc alloc;
				for (int i = 0; i < PerAlloc; ++i)
				{
					Type* p = BOOST_NEW(alloc, Type);
				}
			}
		}
		m_acc.accumulate(counter.trace(log));
	}

	template <class LogT2>
	void doTlsScopedAlloc(LogT2& log, int NAlloc, int PerAlloc)
	{
		NS_BOOST_DETAIL::performance_counter counter;
		{
			for (int j = 0; j < NAlloc; ++j)
			{
				boost::scoped_alloc alloc;
				for (int i = 0; i < PerAlloc; ++i)
				{
					Type* p = BOOST_NEW(alloc, Type);
				}
			}
		}
		m_acc.accumulate(counter.trace(log));
	}

	template <class LogT2>
	void doScopedAlloc(LogT2& log, int NAlloc, int PerAlloc)
	{
		NS_BOOST_MEMORY::block_pool& recycle = NS_BOOST_MEMORY::tls_block_pool::instance();
		NS_BOOST_DETAIL::performance_counter counter;
		{
			for (int j = 0; j < NAlloc; ++j)
			{
				boost::scoped_alloc alloc(recycle);
				for (int i = 0; i < PerAlloc; ++i)
				{
					Type* p = BOOST_NEW(alloc, Type);
				}
			}
		}
		m_acc.accumulate(counter.trace(log));
	}


	void doComparison(LogT& log, int NAlloc)
	{
		int i;
		const int Count = 16;
		const int PerAlloc = Total / NAlloc;

		m_acc.start();
		log.trace("\n===== boost::auto_alloc(%d) =====\n", PerAlloc);
		for (i = 0; i < Count; ++i)
			doAutoAlloc(log, NAlloc, PerAlloc);
		m_acc.trace_avg(log);


		m_acc.start();
		log.trace("\n===== NewDelete(%d) =====\n", PerAlloc);
		for (i = 0; i < Count; ++i)
			doNewDelete(log, NAlloc, PerAlloc);
		m_acc.trace_avg(log);

		m_acc.start();
		log.trace("\n===== TLS boost::scoped_alloc(%d) =====\n", PerAlloc);
		for (i = 0; i < Count; ++i)
			doTlsScopedAlloc(log, NAlloc, PerAlloc);
		m_acc.trace_avg(log);

		m_acc.start();
		log.trace("\n===== boost::scoped_alloc(%d) =====\n", PerAlloc);
		for (i = 0; i < Count; ++i)
			doScopedAlloc(log, NAlloc, PerAlloc);
		m_acc.trace_avg(log);


		m_acc.start();
		log.trace("\n===== BoostPool(%d) =====\n", PerAlloc);
		for (i = 0; i < Count; ++i)
			doBoostPool(log, NAlloc, PerAlloc);
		m_acc.trace_avg(log);

		m_acc.start();
		log.trace("\n===== BoostObjectPool(%d) =====\n", PerAlloc);
		for (i = 0; i < Count; ++i)
			doBoostObjectPool(log, NAlloc, PerAlloc);
		m_acc.trace_avg(log);

#if defined(__GNUG__)
		m_acc.start();
		log.trace("\n===== MtAllocator(%d) =====\n", PerAlloc);
		for (i = 0; i < Count; ++i)
			doMtAllocator(log, NAlloc, PerAlloc);
		m_acc.trace_avg(log);
#endif

	}

	void testComparison(LogT& log)
	{
		NS_BOOST_DETAIL::null_log nullLog;

		doAutoAlloc(nullLog, 1, Total);
		doTlsScopedAlloc(nullLog, 1, Total);

		doComparison(log, Total);
		doComparison(log, 1000);
		doComparison(log, 1);
	}
};

template <class LogT, class Type>
Type* TestAllocatorPerformance<LogT, Type>::p[Total];

// -------------------------------------------------------------------------

void testPerformance()
{
	typedef NS_BOOST_DETAIL::stdout_log LogT;
	LogT log;
	TestAllocatorPerformance<LogT, std::string> test;
	test.testComparison(log);
}

// -------------------------------------------------------------------------
// $Log: performance.cpp,v $
//

int main()
{
    testPerformance();
    return 0;
}

