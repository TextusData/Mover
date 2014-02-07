target=persist.a,
cc_sources=[PersistantObject.cc, ProtoBufObject.cc],
deps=[textus/base/Base, textus/logging/Logging, textus/event/Event, textus/file/File],
unittests=[{
	target=ProtoBufObjectTest,
	proto_sources=[ TestProtoMessage.proto ],
	cc_sources=[ProtoBufObjectTest.cc],
	deps=[ textus/testing/Testing ],
}]
