cc_sources=[ Log.cc, LoggerBuf.cc],
target=logging.a,
deps=[ textus/base/Base.h, textus/base/ThreadLocal.h, textus/base/locks/Mutex.h, textus/base/init/Init.h, textus/base/Thread.h ],
unittests=[ 
{
	name=LogTest,
	target=LogTest,
	cc_sources=[LogTest.cc],
	deps=[ textus/testing/Testing ],
	timeallowed=600,
}
]
