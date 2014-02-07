target=event.a,
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
