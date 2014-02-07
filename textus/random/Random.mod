cc_sources = [ PRNG.cc, Random.cc, BBS.cc, Mixer.cc, SysRand.cc, RDRand.cc ],
#proto_sources = [ ],
target=random.a,
deps=[ textus/base/Base, textus/system/System ],
unittests=[{
	name=RandomTest,
	target=RandomTest,
	cc_sources=[ RandomTest.cc ],
	deps=[ textus/testing/Testing ],
},
]
