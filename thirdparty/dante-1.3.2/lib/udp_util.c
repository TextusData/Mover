/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2008, 2009, 2011
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
"$Id: udp_util.c,v 1.61 2011/05/18 13:48:46 karls Exp $";

struct udpheader_t *
sockaddr2udpheader(to, header)
   const struct sockaddr *to;
   struct udpheader_t *header;
{

   SASSERTX(to->sa_family == AF_INET);

   bzero(header, sizeof(*header));

   fakesockaddr2sockshost(to, &header->host);

   return header;
}

void *
udpheader_add(host, msg, len, msgsize)
   const struct sockshost_t *host;
   void *msg;
   size_t *len;
   const size_t msgsize;
{
   const char *function = "udpheader_add()";
   struct udpheader_t header;
   unsigned char *offset;

   bzero(&header, sizeof(header));
   header.host = *host;

   if (msgsize < (size_t)(*len + PACKETSIZE_UDP(&header))) {
      swarnx("%s: could not prefix socks udp header of size %lu to udp "
             "payload of size %lu",
             function, (unsigned long)PACKETSIZE_UDP(&header),
             (unsigned long)msgsize);

      errno = EMSGSIZE;
      return NULL;
   }

   /* offset old contents by size of header we are about to prefix. */
   memmove((char *)msg + PACKETSIZE_UDP(&header), msg, *len);
   offset = msg;

   memcpy(offset, &header.flag, sizeof(header.flag));
   offset += sizeof(header.flag);

   memcpy(offset, &header.frag, sizeof(header.frag));
   offset += sizeof(header.frag);

   offset = sockshost2mem(&header.host, offset, PROXY_SOCKS_V5);
   offset += *len; /* len bytes copied above. */

   *len = (char *)offset - (char *)msg;

   return msg;
}
