# Template.mod Copyright(c) 2013 Ross Biro
cc_sources = [ Template.cc ],
#proto_sources = [],
target=template.a,
deps=[ textus/base/Base ],

unittests=[{
	name=TemplateTest,
	target=TemplateTest,
	cc_sources=[ TemplateTest.cc ],
	deps=[ textus/testing/Testing ],
},
]
