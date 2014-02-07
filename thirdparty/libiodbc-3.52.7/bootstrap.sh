#! /bin/sh
#
#  bootstrap.sh
#
#  Bootstrap the iODBC project so we do not need to maintain the
#  files generated by autoconf, automake and libtool
#
#  The iODBC driver manager.
#
#  Copyright (C) 1996-2009 by OpenLink Software <iodbc@openlinksw.com>
#  All Rights Reserved.
#
#  This software is released under the terms of either of the following
#  licenses:
#
#      - GNU Library General Public License (see LICENSE.LGPL)
#      - The BSD License (see LICENSE.BSD).
#
#  Note that the only valid version of the LGPL license as far as this
#  project is concerned is the original GNU Library General Public License
#  Version 2, dated June 1991.
#
#  While not mandated by the BSD license, any patches you make to the
#  iODBC source code may be contributed back into the iODBC project
#  at your discretion. Contributions will benefit the Open Source and
#  Data Access community as a whole. Submissions may be made at:
#
#      http://www.iodbc.org
#
#
#  GNU Library Generic Public License Version 2
#  ============================================
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Library General Public
#  License as published by the Free Software Foundation; only
#  Version 2 of the License dated June 1991.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public
#  License along with this library; if not, write to the Free
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#
#  The BSD License
#  ===============
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#  3. Neither the name of OpenLink Software Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENLINK OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# ----------------------------------------------------------------------
#  Globals
# ----------------------------------------------------------------------
DIE=0
ELINES=3


# ----------------------------------------------------------------------
#  Color settings
# ----------------------------------------------------------------------
B=`tput bold 2>/dev/null`
N=`tput sgr0 2>/dev/null`

ERROR="${B}** ERROR **${N}"
WARNING="${B}* WARNING *${N}"


# ----------------------------------------------------------------------
#  Functions
# ----------------------------------------------------------------------
CHECK() {
    for PROG in $*
    do
	VERSION=`$PROG --version 2>/dev/null | head -1 | sed -e "s/$PROG //"`
	if test \! -z "$VERSION"
	then
	    echo "Using $PROG $VERSION"
	    USE_PROG=$PROG
	    break
	fi
    done

    if test -z "VERSION"
    then
	echo
	echo "${ERROR} : You must have \`${B}${PROG}${N}' installed on your system."
	echo
	DIE=1
    fi
}


CHECK_WARN() {
    for PROG in $*
    do
	VERSION=`$PROG --version 2>/dev/null | head -1 | sed -e "s/$PROG //"`
	if test \! -z "$VERSION"
	then
	    echo "Using $PROG $VERSION"
	    USE_PROG=$PROG
	    break
	fi
    done

    if test -z "VERSION"
    then
	echo
	echo "${WARNING} : You may need \`${B}${PROG}${N}' installed on your system."
	echo
    fi
}


RUN() {
    PROG=$1; shift
    ARGS=$*

    echo "Running ${B}${PROG}${N} ..."
    $PROG $ARGS 2>> bootstrap.log
    if test $? -ne 0
    then
	echo
	echo "${ERROR}"
	tail -$ELINES bootstrap.log
	echo
	echo "Bootstrap script aborting (see bootstrap.log for details) ..."
	exit 1
    fi
}


#
#  Check availability of build tools
#
echo
echo "${B}Checking build environment${N} ..."

CHECK libtoolize glibtoolize; LIBTOOLIZE=$USE_PROG
CHECK aclocal
CHECK autoheader
CHECK automake
CHECK autoconf
CHECK_WARN gtk-config

if test "$DIE" -eq 1
then
    echo
    echo "Please read the ${B}README.CVS${N} file for a list of packages you need"
    echo "to install on your system before bootstrapping this project."
    echo
    echo "Bootstrap script aborting ..."
    exit 1
fi


#
#  Generate the build scripts
#
> bootstrap.log

echo
echo "${B}Generating build scripts${N} ..."

RUN $LIBTOOLIZE --force --copy
RUN aclocal -I admin
RUN autoheader
RUN automake --copy --add-missing
RUN autoconf

echo
echo "Please check the ${B}INSTALL${N} and ${B}README${N} files for instructions to"
echo "configure, build and install iODBC on your system."
echo
echo "Certain build targets are only enabled in maintainer mode:"
echo
echo "    ./configure --enable-maintainer-mode ..."
echo
echo "Bootstrap script completed successfully."

exit 0
