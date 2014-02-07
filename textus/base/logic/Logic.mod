cc_sources=[ Bool.cc ],
proto_sources=[ Bool.proto ],
target=logic.a,
deps=[ textus/logging/Log.h ],
unittests=[{
	 name=BoolTest,
	 cc_sources=[ BoolTest.cc ],
	 target=BoolTest,
	 deps=[ textus/testing/Testing, textus/base/Base ]
}]
