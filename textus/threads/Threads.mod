cc_sources = [WorkerThread.cc, EventQueueThread.cc  ],
target=threads.a,
deps=[ textus/event/Event, textus/base/Base, textus/system/System ],

