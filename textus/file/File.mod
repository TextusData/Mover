target=file.a,
cc_sources=[FileHandle.cc, SystemFile.cc, TextusFile.cc, Pipe.cc, Shell.cc, ProcessMonitor.cc, ProcessEventFactory.cc,
	   ProcessEvent.cc, LineReader.cc, LineReaderEvent.cc, LineReaderEventFactory.cc,
	   ProtoBufReader.cc, ProtoBufReaderEvent.cc, ProtoBufReaderEventFactory.cc,
	   Directory.cc, SystemDirectory.cc, Handle.cc, HashFileStore.cc,
	   FileBackedNumber.cc,
	   ], 
deps=[ textus/base/Base, textus/logging/Logging, textus/event/Event,
       textus/base/logic/Logic, textus/threads/Threads, textus/system/System ],
unittests=[{
	target=FileHandleTest,
	cc_sources=[FileHandleTest.cc],
	deps=[ textus/testing/Testing ],
},
{
	target=PipeTest,
	cc_sources=[PipeTest.cc],
	deps=[ textus/testing/Testing ],
},
{
	target=ShellTest,
	cc_sources=[ShellTest.cc],
	deps=[ textus/testing/Testing ],
},
{
	target=LineReaderTest,
	cc_sources=[LineReaderTest.cc],
	deps=[ textus/testing/Testing ],
},
{
	target=ProtoBufReaderTest,
	proto_sources=[ TestProtoMessage.proto ],
	cc_sources=[ProtoBufReaderTest.cc],
	deps=[ textus/testing/Testing ],
}
]
