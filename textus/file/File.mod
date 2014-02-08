target=file.a,

#
#   This program is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, version 3 of the
#   License.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see
#   <http://www.gnu.org/licenses/>.
#
#
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
