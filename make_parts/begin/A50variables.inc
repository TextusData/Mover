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
ABSSRCROOT=$(shell pwd)
# the protocompiler doesn't currently seem to like absolute paths under some circumstances
SRCROOT=.
OBJROOT=$(ABSSRCROOT)/obj
BINROOT=$(ABSSRCROOT)/bin
MAKEROOT=$(ABSSRCROOT)/make
export OBJROOT BINROOT ABSSRCROOT MAKEROOT

