		//////////////////////////////////////////////////////////////////////////
		// 使用这种方式的原因是，当把 ar 作为一个引用参数传递时，
		// 编译器会禁止一些优化，比如把 ar 本身放入寄存器，
		// 而这种优化导致的结果差异可能很巨大，已发现的是在 VC2008 中，达到 2 倍以上
		//
		//  expanded as below, 'ar' will be put in register
		//  不手工展开 test_serialize, 编译器不能将 ar 放进寄存器，速度会慢一倍(d.d4 大一倍)
		//	test_serialize(ar, d);
		//------------------------------------------------------------------------
		int64_t c0, c1, c2, c3, c4, c5, c6, c7;

		QueryPerformanceCounter(&c0);
		ar & v0;
		QueryPerformanceCounter(&c1);
		ar & v1;
		QueryPerformanceCounter(&c2);
		ar & v2;
		QueryPerformanceCounter(&c3);
		ar & v3;
		QueryPerformanceCounter(&c4);

		for (int i=0, n=v0.size(); i < n; ++i)
		{
			MyData1 md1;
			ar & md1;
		}
		QueryPerformanceCounter(&c5);

		for (int i=0, n=v0.size(); i < n; ++i)
		{
			VarIntD vid;
			ar & vid;
		}
		QueryPerformanceCounter(&c6);

		ar & v4;
		QueryPerformanceCounter(&c7);

		d.d0 += c1 - c0;
		d.d1 += c2 - c1;
		d.d2 += c3 - c2;
		d.d3 += c4 - c3;
		d.d4 += c5 - c4;
		d.d5 += c6 - c5;
		d.d6 += c7 - c6;
		//////////////////////////////////////////////////////////////////////////
