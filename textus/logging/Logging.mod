cc_sources=[ Log.cc, LoggerBuf.cc],

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
target=logging.a,
deps=[ textus/base/Base.h, textus/base/ThreadLocal.h, textus/base/locks/Mutex.h, textus/base/init/Init.h, textus/base/Thread.h ],
unittests=[ 
{
	name=LogTest,
	target=LogTest,
	cc_sources=[LogTest.cc],
	deps=[ textus/testing/Testing ],
	timeallowed=600,
}
]
