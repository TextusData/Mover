/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2008,
 *               2009, 2010, 2011
 *               2009
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
 */

#include "common.h"

static const char rcsid[] =
"$Id: accesscheck.c,v 1.62 2011/05/18 13:48:46 karls Exp $";

int
usermatch(auth, userlist)
   const struct authmethod_t *auth;
   const struct linkedname_t *userlist;
{
/*   const char *function = "usermatch()"; */
   const char *name;

   if ((name = authname(auth)) == NULL)
      return 0; /* no username, no match. */

   do
      if (strcmp(name, userlist->name) == 0)
         break;
   while ((userlist = userlist->next) != NULL);

   if (userlist == NULL)
      return 0; /* no match. */
   return 1;
}

int
groupmatch(auth, grouplist)
   const struct authmethod_t *auth;
   const struct linkedname_t *grouplist;
{
   const char *function = "groupmatch()";
   const char *username;
   struct passwd *pw;
   struct group *groupent;

   SASSERTX(grouplist != NULL);

   if ((username = authname(auth)) == NULL)
      return 0; /* no username, no match. */

   /*
    * First check the primary group of the user against grouplist.
    * If the groupname given there matches, we don't need to go through
    * all users in the list of group.
    */
   if ((pw = socks_getpwnam(username))   != NULL
   &&  (groupent = getgrgid(pw->pw_gid)) != NULL) {
      const struct linkedname_t *listent = grouplist;

      do
         if (strcmp(groupent->gr_name, listent->name) == 0)
            return 1;
      while ((listent = listent->next) != NULL);
   }
   else {
      if (pw == NULL)
         slog(LOG_DEBUG, "%s: unknown unsername \"%s\"", function, username);
      else if (groupent == NULL)
         slog(LOG_DEBUG, "%s: unknown primary groupid %ld",
         function, (long)pw->pw_gid);
   }

   /*
    * Go through grouplist, matching username against each groupmember of
    * all the groups in grouplist.
    */
   do {
      char **groupname;

      if ((groupent = getgrnam(grouplist->name)) == NULL) {
         swarn("%s: unknown groupname \"%s\"", function, grouplist->name);
         continue;
      }

      groupname = groupent->gr_mem;

      while (*groupname != NULL) {
         if (strcmp(username, *groupname) == 0)
            return 1; /* match. */

         ++groupname;
      }
   } while ((grouplist = grouplist->next) != NULL);

   return 0;
}

#if HAVE_LDAP
int
ldapgroupmatch(auth, rule)
   const struct authmethod_t *auth;
   const struct rule_t *rule;
{
   const char *function = "ldapgroupmatch()";
   const struct linkedname_t *grouplist;
   const char *username;
   char *userdomain, *groupdomain;
   int retval;

   if ((username = authname(auth)) == NULL)
      return 0; /* no username, no match. */

#if !HAVE_GSSAPI
   if (!rule->state.ldap.ldapurl)
      SERRX(rule->state.ldap.ldapurl != NULL);
#endif /* !HAVE_GSSAPI */

   if ((userdomain = strchr(username, '@')) != NULL)
      ++userdomain;

   if (userdomain == NULL && *rule->state.ldap.domain == NUL && rule->state.ldap.ldapurl == NULL) {
      slog(LOG_DEBUG, "%s: can not check ldap group membership for user %s: "
                      "user has no domain postfix and no ldap url is defined",
                      function, username);
      return 0;
   }

   if ((retval = ldap_user_is_cached(username)) >= 0)
      return retval;

   /* go through grouplist, matching username against members of each group. */
   grouplist = rule->ldapgroup;
   do {
      char groupname[MAXNAMELEN];

      slog(LOG_DEBUG, "%s: checking if user %s is member of ldap group %s",
                      function, username, grouplist->name);

      SASSERTX(sizeof(groupname) > strlen(grouplist->name));
      strcpy(groupname, grouplist->name);

      if ((groupdomain = strchr(groupname, '@')) != NULL) {
         *groupdomain = NUL; /* separates groupname from groupdomain. */
         ++groupdomain;
      }

      if (groupdomain != NULL && userdomain != NULL) {
         if (strcmp(groupdomain, userdomain) != 0
         &&  strcmp(groupdomain, "") != 0) {
            slog(LOG_DEBUG, "%s: userdomain \"%s\" does not match groupdomain "
                            "\"%s\" and groupdomain is not default domain.  "
                            "Trying next entry",
                            function, userdomain, groupdomain);
            continue;
         }
      }

      if (ldapgroupmatches(username, userdomain, groupname, groupdomain, rule)){
         cache_ldap_user(username, 1);
         return 1;
      }
   } while ((grouplist = grouplist->next) != NULL);

   cache_ldap_user(username, 0);
   return 0;
}
#endif /* HAVE_LDAP */

/* ARGSUSED */
int
accesscheck(s, auth, src, dst, emsg, emsgsize)
   int s;
   struct authmethod_t *auth;
   const struct sockaddr *src, *dst;
   char *emsg;
   size_t emsgsize;
{
   const char *function = "accesscheck()";
   char srcstr[MAXSOCKADDRSTRING], dststr[sizeof(srcstr)];
   int match, authresultisfixed;

   if (emsgsize > 0)
      *emsg = NUL;

   if (sockscf.option.debug)
      slog(LOG_DEBUG, "%s: method: %s, %s -> %s ",
      function, method2string(auth->method),
      src == NULL ? "<unknown>" : sockaddr2string(src, srcstr, sizeof(srcstr)),
      dst == NULL ? "<unknown>" : sockaddr2string(dst, dststr, sizeof(dststr)));

   /*
    * We don't want to re-check the same method.  This could
    * happen in several cases:
    *  - was checked as client-rule, is now checked as socks-rule.
    *  - a different rule with the same method.  The client is however
    *    the same, so if the auth failed on the method before, it will
    *    fail next time also.
   */

   if (methodisset(auth->method, auth->methodv, (size_t)auth->methodc)) {
      slog(LOG_DEBUG, "%s: method %s already checked, matches",
      function, method2string(auth->method));

      return 1; /* already checked, matches. */
   }

   if (methodisset(auth->method, auth->badmethodv, (size_t)auth->badmethodc)) {
      snprintf(emsg, emsgsize,
              "authentication provided by client for method \"%s\" "
              "does not match",
              method2string(auth->method));

      slog(LOG_DEBUG, "%s: method %s already checked, does not match",
      function, method2string(auth->method));

      return 0; /* already checked, won't match. */
   }

   match = 0;
   switch (auth->method) {
      /*
       * Methods where no further checking is done at this point, either
       * because there's nothing to check, or it has already been checked.
       */
      case AUTHMETHOD_NONE:
#if HAVE_GSSAPI
      case AUTHMETHOD_GSSAPI:
#endif /* HAVE_GSSAPI */
         match = 1;
         break;

      case AUTHMETHOD_UNAME:
         if (passwordcheck((const char *)auth->mdata.uname.name,
         (const char *)auth->mdata.uname.password, emsg, emsgsize) == 0)
            match = 1;
         break;

#if HAVE_LIBWRAP
      case AUTHMETHOD_RFC931:
         if (passwordcheck((const char *)auth->mdata.rfc931.name, NULL, emsg,
         emsgsize) == 0)
            match = 1;
         break;
#endif /* HAVE_LIBWRAP */

#if HAVE_PAM
      case AUTHMETHOD_PAM: {
#if DIAGNOSTIC
         const int freec = freedescriptors(sockscf.option.debug ?
         "start" : NULL);
#endif /* DIAGNOSTIC */

         if (pam_passwordcheck(s, src, dst, &auth->mdata.pam, emsg, emsgsize)
         == 0)
            match = 1;

#if DIAGNOSTIC
         if (freec != freedescriptors(sockscf.option.debug ?  "end" : NULL))
            swarnx("%s: lost %d file descriptor%s in pam_passwordcheck()",
                   function, freec - freedescriptors(NULL),
                   (freec - freedescriptors(NULL)) == 1 ? "" : "s");
#endif /* DIAGNOSTIC */
         break;
      }
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
      case AUTHMETHOD_BSDAUTH: {
#if DIAGNOSTIC
         const int freec = freedescriptors(sockscf.option.debug ?
         "start" : NULL);
#endif /* DIAGNOSTIC */

         if (bsdauth_passwordcheck(s, src, dst, &auth->mdata.bsd, emsg,
         emsgsize) == 0)
            match = 1;

#if DIAGNOSTIC
         if (freec != freedescriptors(sockscf.option.debug ?  "end" : NULL))
            swarnx("%s: lost %d file descriptor%s in bsdauth_passwordcheck()",
                   function, freec - freedescriptors(NULL),
                   (freec - freedescriptors(NULL)) == 1 ? "" : "s");
#endif /* DIAGNOSTIC */
         break;
      }
#endif /* HAVE_BSDAUTH */

      default:
         SERRX(auth->method);
   }

   /*
    * Some methods can be called with different values for the
    * same client, based on values configured in the rules.
    * Others can not and we want to mark those who can not as
    * "tried", so we don't waste time on re-trying them.
    */
   switch (auth->method) {
#if HAVE_PAM
      case AUTHMETHOD_PAM:
         if (sockscf.state.pamservicename == NULL)
            authresultisfixed = 0;
         else
            authresultisfixed = 1;
         break;
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
   case AUTHMETHOD_BSDAUTH:
         if (sockscf.state.bsdauthstylename == NULL)
            authresultisfixed = 0;
         else
            authresultisfixed = 1;
         break;
#endif /* HAVE_PAM */

#if HAVE_GSSAPI
      case AUTHMETHOD_GSSAPI:
         if (sockscf.state.gssapiservicename == NULL
         ||  sockscf.state.gssapikeytab      == NULL)
            authresultisfixed = 0;
         else
            authresultisfixed = 1;
         break;
#endif /* HAVE_GSSAPI */

      case AUTHMETHOD_NONE:
      case AUTHMETHOD_UNAME:
      case AUTHMETHOD_RFC931:
         authresultisfixed = 1;
         break;

      default:
         SERRX(auth->method);
   }

   if (authresultisfixed) {
      if (match) {
         SASSERTX(auth->methodc + 1 <= sizeof(auth->methodv));
         auth->methodv[auth->methodc++] = auth->method;
      }
      else {
         SASSERTX(auth->badmethodc + 1 <= sizeof(auth->badmethodv));
         auth->badmethodv[auth->badmethodc++] = auth->method;
      }

      /*
       * We might have wanted to bzero() the password here, but
       * then we wouldn't be able to use the password if we
       * at a later point needed to check for access against
       * a different method.  (For instance, PAM on setup,
       * UNAME on UDP packet.  Strange, but in theory possible.)
       */
   }

   if (!match && emsgsize > 0)
      slog(LOG_DEBUG, "%s: no match: %s", function, emsg);

   return match;
}
