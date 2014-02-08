cc_sources=[ Socket.cc, NetworkAddress.cc, Protocol.cc, URL.cc ],

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
target=network.a,
deps=[ textus/base/Base, textus/file/File, textus/random/Random ],
unittests=[{
	name=SocketTest,
	target=SocketTest,
	cc_sources=[SocketTest.cc ],
	deps=[ textus/testing/Testing ],
},
{
	name=ResolverTest,
	target=ResolverTest,
	cc_sources=[ResolverTest.cc ],
	deps=[ textus/testing/Testing ],
}
]
