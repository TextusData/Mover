# Base.mod Copyright(c) 2012-2013 Ross Biro
cc_sources = [ Base.cc, Thread.cc, ThreadID.cc, ThreadLocal.cc,
	       Time.cc, OneTimeUse.cc, UUID.cc,
	       LeakyBucket.cc, ThreadCallBack.cc, SignalHandler.cc ],
proto_sources = [ UUID.proto ],
target=base.a,
deps=[ textus/base/init/Init.h, textus/base/locks/Lock.h,
       textus/base/locks/Mutex.h, textus/base/locks/Condition.h,
       textus/base/logic/Bool.h, textus/logging/Log.h ],

unittests=[{
	name=BaseTest,
	target=BaseTest,
	cc_sources=[ BaseTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=CacheTest,
	target=CacheTest,
	cc_sources=[ CacheTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=ThreadLocalTest,
	target=ThreadLocalTest,
	cc_sources=[ ThreadLocalTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=OneTimeUseTest,
	target=OneTimeUseTest,
	cc_sources=[ OneTimeUseTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=PerThreadTest,
	target=PerThreadTest,
	cc_sources=[ PerThreadTest.cc ],
	deps=[ textus/testing/Testing ],
},

]
