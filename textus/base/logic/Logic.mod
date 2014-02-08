cc_sources=[ Bool.cc ],

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
proto_sources=[ Bool.proto ],
target=logic.a,
deps=[ textus/logging/Log.h ],
unittests=[{
	 name=BoolTest,
	 cc_sources=[ BoolTest.cc ],
	 target=BoolTest,
	 deps=[ textus/testing/Testing, textus/base/Base ]
}]
