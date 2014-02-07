# Util.mod Copyright(c) 2013-2014 Ross Biro
cc_sources = [ Base64.cc, Hex.cc ],
#proto_sources = [ UUID.proto ],
target=util.a,
#deps=[ ],

unittests=[{
	name=RegexpTest,
	target=RegexpTest,
	cc_sources=[ RegexpTest.cc ],
	deps=[ textus/testing/Testing ],
},
]
