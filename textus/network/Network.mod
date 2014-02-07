cc_sources=[ Socket.cc, NetworkAddress.cc, Protocol.cc, URL.cc ],
target=network.a,
deps=[ textus/base/Base, textus/file/File, textus/random/Random ],
unittests=[{
	name=SocketTest,
	target=SocketTest,
	cc_sources=[SocketTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=ResolverTest,
	target=ResolverTest,
	cc_sources=[ResolverTest.cc ],
	deps=[ textus/testing/Testing ],
}
]