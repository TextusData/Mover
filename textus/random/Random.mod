cc_sources = [ PRNG.cc, Random.cc, BBS.cc, Mixer.cc, SysRand.cc, RDRand.cc ],

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
#proto_sources = [ ],
target=random.a,
deps=[ textus/base/Base, textus/system/System ],
unittests=[{
	name=RandomTest,
	target=RandomTest,
	cc_sources=[ RandomTest.cc ],
	deps=[ textus/testing/Testing ],
},
]
