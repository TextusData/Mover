target=event.a,

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
cc_sources=[ ConditionTimer.cc, ConditionTimerEvent.cc, Event.cc, EventQueue.cc, Timer.cc, TimerEvent.cc,
	     TimerQueue.cc, Message.cc],
deps=[ textus/base/Base, textus/logging/Logging, textus/threads/Threads ],
unittests=[{
    name=TimerTest,
    target=TimerTest,
    cc_sources=[ TimerTest.cc ],
    deps = [ textus/testing/Testing ],
},
{
    name=TimerEventTest,
    target=TimerEventTest,
    cc_sources = [TimerEventTest.cc ],
    deps = [ textus/testing/Testing ],
},
{
    name=StateTest,
    target=StateTest,
    cc_sources = [ StateTest.cc ],
    deps = [ textus/testing/Testing ],
}]
