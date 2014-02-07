cc_sources=[ SimpleServer.cc ],
target=server.a,
deps=[ textus/base/Base, textus/file/File, textus/network/Network ],
unittests=[{
	name=LineServerTest,
	target=LineServerTest,
	cc_sources=[LineServerTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=MessageServerTest,
	target=MessageServerTest,
	cc_sources=[MessageServerTest.cc ],
	deps=[ textus/testing/Testing ],
}
]