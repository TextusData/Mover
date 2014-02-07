cc_sources = [ Mover.cc, MoverExtension.cc, MoverMessage.cc, ListExtensions.cc,
	       MoverMessageProcessor.cc, MoverEncryption.cc,
	       ExternalEncryption.cc, AddressBook.cc, AddressBookEntry.cc,
	       KeyDescription.cc, MultiKeyDescription.cc, KeyBook.cc,
	     ],
#proto_sources = [ ],
target=mover.a,
deps=[ textus/base/Base, textus/network/Network, textus/network/ssl/SSL,
       textus/network/server/Server, textus/base/init/Init,
       textus/util/Util, textus/template/Template, textus/config/Config],

others = [
{
     cc_sources = [ MoverMain.cc ],
     deps= [ textus/mover/Mover],
     target=mover,
},
{
     c_sources = [xor.c],
     target=xor,
},
{
     cc_sources = [otp.cc],
     target=otp,
},
{
     cc_sources = [ MoverSendMain.cc ],
     deps= [ textus/mover/Mover],
     target=mover-send,
},
{
     cc_sources = [ MoverReaderMain.cc ],
     deps= [ textus/mover/Mover],
     target=mover-reader,
},
]


unittests=[{
	name=MoverEncryptionTest,
	target=MoverEncryptionTest,
	cc_sources=[ MoverEncryptionTest.cc ],
	deps=[ textus/testing/Testing ],
	timeallowed=2400,
},
]
