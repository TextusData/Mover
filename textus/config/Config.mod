# Config.mod Copyright(c) 2013-2014 Ross Biro

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
cc_sources = [ Config.cc ]
#proto_sources = [ ],
target=config.a,
deps=[ textus/file/File, textus/base/Base, textus/template/Template ]

unittests=[
{
	name=ConfigTest,
	target=ConfigTest,
	cc_sources=[ ConfigTest.cc ],
	deps=[ textus/testing/Testing ],
},
]
