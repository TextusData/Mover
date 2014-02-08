# Base.mod Copyright(c) 2012-2013 Ross Biro
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

cc_sources = [ Base.cc, Thread.cc, ThreadID.cc, ThreadLocal.cc,
	       Time.cc, OneTimeUse.cc, UUID.cc,
	       LeakyBucket.cc, ThreadCallBack.cc, SignalHandler.cc ],
proto_sources = [ UUID.proto ],
target=base.a,
deps=[ textus/base/init/Init.h, textus/base/locks/Lock.h,
       textus/base/locks/Mutex.h, textus/base/locks/Condition.h,
       textus/base/logic/Bool.h, textus/logging/Log.h ],

unittests=[{
	name=BaseTest,
	target=BaseTest,
	cc_sources=[ BaseTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=CacheTest,
	target=CacheTest,
	cc_sources=[ CacheTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=ThreadLocalTest,
	target=ThreadLocalTest,
	cc_sources=[ ThreadLocalTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=OneTimeUseTest,
	target=OneTimeUseTest,
	cc_sources=[ OneTimeUseTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=PerThreadTest,
	target=PerThreadTest,
	cc_sources=[ PerThreadTest.cc ],
	deps=[ textus/testing/Testing ],
},

]
