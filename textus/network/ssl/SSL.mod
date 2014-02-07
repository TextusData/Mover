cc_sources=[ SecureSocket.cc, Certificate.cc ],
target=ssl.a,
deps=[ textus/base/Base, textus/network/Network ],
unittests=[{
	name=SecureSocketTest,
	target=SecureSocketTest,
	cc_sources=[SecureSocketTest.cc],
	deps=[ textus/testing/Testing ],
},
{
	name=SecureMessageTest,
	target=SecureMessageTest,
	cc_sources=[SecureMessageTest.cc],
	deps=[ textus/testing/Testing ],
}]
