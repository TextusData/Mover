# Config.mod Copyright(c) 2013 Ross Biro
cc_sources = [ Config.cc ]
#proto_sources = [ ],
target=config.a,
deps=[ textus/file/File, textus/base/Base ]

unittests=[
{
	name=ConfigTest,
	target=ConfigTest,
	cc_sources=[ ConfigTest.cc ],
	deps=[ textus/testing/Testing ],
},
]
