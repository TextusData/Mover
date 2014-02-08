cc_sources = [ Mover.cc, MoverExtension.cc, MoverMessage.cc, ListExtensions.cc,

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
