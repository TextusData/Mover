cc_sources=[ Key.cc, EncryptedKey.cc ],
proto_sources=[ Key.proto, EncryptedKey.proto ],
target=keys.a,
deps=[ textus/base/Base, textus/logging/Logging, textus/keys/providers/RSA/RSA, textus/base/locks/Locks ]
unittests=[{
	name=KeyTest,
	target=KeyTest,
	cc_sources=[ KeyTest.cc ],
	deps=[ textus/testing/Testing ],
}
]
