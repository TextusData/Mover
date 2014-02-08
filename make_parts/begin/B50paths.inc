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

# Some third party software needs makedepend which is in /usr/X11/bin,
# but /usr/X11/bin is not alway sin the path for non-X terminals.

ifeq ($(filter /usr/X11/bin,$(PATH)), )
PATH:=$(PATH):/usr/X11/bin
export PATH
endif
