cc_sources = [ textus/db/DBType.cc, textus/db/DBPool.cc, textus/db/DBHandle.cc, textus/db/DBStatement.cc,
	       textus/db/ColType.cc, textus/db/DBUtils.cc, textus/db/DBTypeUUID.cc, textus/db/DBName.cc,
	       textus/db/DBInstanceID.cc, textus/db/DBThread.cc ],
#proto_sources = [ ],
target=db.a,
deps=[ textus/base/Base, textus/logging/Logging, textus/base/locks/Locks ],
unittests=[{
	name=ColTypeTest,
	target=CoTypelTest,
	cc_sources=[ textus/db/ColTypeTest.cc ],
	deps=[ textus/testing/Testing ],
},{
	name=DBTypeTest,
	target=DBTypeTest,
	cc_sources=[ textus/db/DBTypeTest.cc ],
	deps=[ textus/testing/Testing ],
},{
	name=DBConnectTest,
	target=DBConnectTest,
	cc_sources=[ textus/db/DBConnectTest.cc ],
	deps=[ textus/testing/Testing ],
},{
	name=DBStatementTest,
	target=DBStatementTest,
	cc_sources=[ textus/db/DBStatementTest.cc ],
	deps=[ textus/testing/Testing ],
},{
	name=DBAutoDerefTest,
	target=DBAutoDerefTest,
	cc_sources=[ textus/db/DBAutoDerefTest.cc ],
	deps=[ textus/testing/Testing ],
},{
	name=ColTypeKeyTest,
	target=ColTypeKeyTest,
	cc_sources=[ textus/db/ColTypeKeyTest.cc ],
	deps=[ textus/testing/Testing, textus/keys/Keys ],
},{
	name=DBPoolTest,
	target=DBPoolTest,
	cc_sources=[ textus/db/DBPoolTest.cc ],
	deps=[ textus/testing/Testing, textus/keys/Keys ],
},{
	name=ColTypeDateTimeTest,
	target=ColTypeDateTimeTest,
	cc_sources=[ textus/db/ColTypeDateTimeTest.cc ],
	deps=[ textus/testing/Testing, textus/keys/Keys ],
},{
	name=DBManyManyTest,
	target=DBManyManyTest,
	cc_sources=[ textus/db/DBManyManyTest.cc ],
	deps=[ textus/testing/Testing ],
}
]
