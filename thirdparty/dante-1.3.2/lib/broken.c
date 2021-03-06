/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2008, 2009
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 * $Id: broken.c,v 1.27 2009/10/23 11:43:35 karls Exp $
 */

#include "common.h"

static const char rcsid[] =
"$Id: broken.c,v 1.27 2009/10/23 11:43:35 karls Exp $";

#if HAVE_SHADOW_H && HAVE_GETSPNAM
#include <shadow.h>
#endif /* HAVE_SHADOW_H && HAVE_GETSPNAM */

struct passwd *
socks_getpwnam(login)
   const char *login;
{
   const int errno_s = errno;
   struct passwd *pwd;

   if ((pwd = getpwnam(login)) == NULL)
      return NULL;

#if HAVE_GETSPNAM /* broken sysv stuff. */
   {
      struct spwd *spwd;

      if ((spwd = getspnam(login)) != NULL)
         pwd->pw_passwd = spwd->sp_pwdp;
   }
#elif HAVE_GETPRPWNAM /* some other broken stuff. */
   /*
    * XXX, don't know how this looks yet.
   */
#error "getprpwnam() not supported"
   pwd = NULL;
#endif /* HAVE_GETSPNAM */

   /*
    * some systems can set errno even on success. :-/
    * E.g. OpenBSD 4.4. seems to do this.  Looks like it tries
    * /etc/spwd.db first, and if that fails, /etc/pwd.db, but it
    * forgets to reset errno.
    */
   errno = errno_s;

   return pwd;
}
