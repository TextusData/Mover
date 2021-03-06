/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011
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
#include "config_parse.h"

static const char rcsid[] =
"$Id: sockd_io.c,v 1.721 2011/08/03 11:27:26 michaels Exp $";

/*
 * IO-child:
 * Accept io objects from mother and do io on them.  We never
 * send back ancillary data, only ordinary data, so no need for
 * locking here even on broken systems (#ifdef HAVE_SENDMSG_DEADLOCK).
 * XXX with covenant, no longer true.  Need to add lock, if it is still
 * necessary due to kernel bugs, but hopefully it is not.
 *
 * XXX remove io_allocated()?  Add some variables instead that we
 * always keep updated.
 */

typedef enum { TIMEOUT_CONNECT = 1,
               TIMEOUT_IO,
               TIMEOUT_TCP_FIN_WAIT
} timeout_type_t;

/* why a doio() call failed. */
typedef enum { IO_NOERROR,
               IO_TMPERROR,          /* some error occurred, but not fatal.  */
               IO_EAGAIN,            /* no data available to read currently. */
               IO_TIMEOUT,
               IO_ERROR,
               IO_CLOSE,
               IO_BLOCK,
               IO_ADMINTERMINATION,
} iostatus_t;

typedef enum { RAWSOCKET_NOP,          /* nothing to care about. */
               RAWSOCKET_IO_DELETED    /* an io was deleted.     */
} rawsocketstatus_t;

static void siginfo(int sig, siginfo_t *sip, void *scp);

static size_t
io_allocated(int *for_tcp, int *for_udp);
/*
 * Returns the number of allocated (active) ios.
 *
 * If "for_tcp" is not NULL, on return it will contain the number of ios
 * allocated for tcp.
 * If "for_udp" is not NULL, on return it will contain the number of ios
 * allocated for udp.
 */

static struct sockd_io_t *
io_getset(const int nfds, const fd_set *set);
/*
 * Goes through our list until it finds an io object where at least one of the
 * descriptors in "set" is set.  "nfds" gives the number of descriptors in
 * "set" to check
 *
 * Returns NULL if none found.
 */

static struct sockd_io_t *
io_finddescriptor(int d);
/*
 * Finds the io object where one of the descriptors matches "fd".
 */


static int
io_fillset(fd_set *set, int antiflags, struct timeval *bwoverflowtil);
/*
 * Sets all descriptors from our list, in "set".
 * If "antiflags" is set, ios with any of the flags in "antiflags" set
 * will be excluded.
 * In addition, ios with state.fin set, ios that have not finished
 * connecting, and ios that have overflown the bandwidth limit, will also be 
 * excluded.
 *
 * If any ios were excluded due to haveing overflown the bandwidth limit,
 * the earliest time we can again do i/o over one of the bandwidth-excluded
 * ios will be stored in "bwoverflowtil", if not NULL.
 *
 * Returns the highest descriptor in our list, or -1 if we don't
 * have any descriptors we want to select() on currently.
 */

static int
io_fillset_connectinprogress(fd_set *set);
/*
 * Like io_fillset(), but fills "set" only with descriptors belonging
 * to connects that are marked as still being in progress.
 */


static void
io_clearset(const struct sockd_io_t *io, fd_set *set);
/*
 * Clears all file descriptors in "io" from "set".
 */


static iostatus_t
doio(struct sockd_io_t *io, fd_set *rset, fd_set *wset,
     const int flags, int *badfd);
/*
 * Does i/o over the descriptors in "io", in to out and out to in.
 * "io" is the object to do i/o over,
 * "flags" is the flags to set on the actual i/o calls
 * (read()/write(), recvfrom()/sendto()), currently only MSG_OOB.
 *
 * Returns the status of the doio() call, IO_NOERROR on success, or
 * some other value on error.  If error and "badfd" is not -1, it
 * will have the value of the file descriptor on which the error was
 * detected.
 *
 * In most cases, delete_io() should be called upon error.
 */

static int
io_rw(struct sockd_io_direction_t *in, struct sockd_io_direction_t *out,
      int *bad, const requestflags_t *reqflags, char *buf, size_t bufsize,
      size_t *bufused, int flags);
/*
 * Transfers *TCP* data from "in" to "out" using "buf" as a temporary buffer
 * to store the data, and sets flag "flags" in sendto()/recvfrom().
 * "reqflags" is flags for the clientside of the request.
 *
 * The size of "buf" is given by bufsize, and "bufused", if not NULL,
 * indicates how much of "buf" has previously been used, but not
 * written to "out".  Upon return, "bufused" is updated with the
 * new value.  "bufused" is only used by the Covenant server, and should
 * be set to NULL when called by anyone else.
 *
 * "inauth" is the authentication used for reading from "in",
 * "outauth" is the authentication * used when writing to out.
 * The data transferred uses "buf" as a buffer, which is of size "bufsize".
 * The function never transfers more than the receive low watermark
 * of "out".
 *
 * Returns:
 *      On success: number of bytes transferred from "in" to "out".
 *      On failure: -1.  "bad" is set to the value of the descriptor that
 *                  "failure" was first detected on, or -1 if the "failure"
 *                  was temporary.
 *                  "failure" may also be eof, in which case the number of
 *                  bytes transferred will be zero.
 */

static void
delete_io(int mother, struct sockd_io_t *io, int fd, const iostatus_t status);
/*
 * deletes the io object "io".  "fd" is the descriptor on which "status"
 * was returned.  If "fd" is negative, it is ignored.
 * If "mother" is >= 0, the deletion of "io" is ACK'ed to her.
 * "status" is the reason for why the io was deleted.
 */

static void
proctitleupdate(void);
/*
 * Updates the title of this process.
 */

static int
io_timeoutispossible(const struct sockd_io_t *io);
/*
 * Returns true if it's possible the ioboject "io" could time out, i.e.,
 * the config and state of the io object is such that it is possible.
 *
 * Returns false if it is not possible for the i/o object to time out
 * in it's current state with the current config.
 */

static long
io_timetiltimeout(struct sockd_io_t *io, const struct timeval *tnow, 
                  timeout_type_t *type);
/*
 * Returns the number of seconds til the io object "io" will timeout, at
 * the earliest.  
 *
 * "tnow" is the current time.
 *
 * "type", if not NULL, is filled in with the type of timeout that will 
 * occur at that time, if any.
 *
 * Returns -1 if no timeout on the io is currently present.
 */

static struct timeval *
io_gettimeout(struct timeval *timeout);
/*
 * If there is an applicable timeout on the current clients for how long 
 * we should wait for them to do i/o again, this function fills in "timeout"
 * with the time remaining til then.
 *
 * Returns:
 *      If there is a timeout: timeout.
 *      If there is no applicable timeout currently: NULL.
 */

static struct sockd_io_t *
io_gettimedout(void);
/*
 * Scans all clients for one that has timed out according to sockscf
 * settings.
 *
 * Returns:
 *      If timed out client found: pointer to timed out i/o object.
 *      Else: NULL.
 */

static void
getnewios(void);
/*
 * Receives new ios from mother.  Closes mother and sets
 * mother sockets to -1 if there is an error.
 */

static void
freebuffers(const struct sockd_io_t *io);
/*
 * Frees buffers, if any, used by "io".
 */

static int
checkconnectstatus(struct sockd_io_t *io, int *badfd);
/*
 * Checks if the socket on "io->dst" has finished connecting, and fills
 * in status flags as appropriate.  Note that this function should only
 * be called once the connnect has completed (successfully or otherwise).
 *
 * Note that this function must be called after the connect has completed,
 * as in the socks case (and some covenant cases) we need to send a
 * response back to the client before it will start sending us data.
 * We can thus not delay calling this function til we get ordinary i/o
 * from one side, as it's possible none will be coming til after we
 * have sent the response to the client.
 *
 * Returns 0 if the socket connected successfully.
 * Returns -1 if the socket was not connected successfully, or some other error
 *            occured.  In this case, "badfd" has the value of the "bad" fd.
 */

static void 
io_update(const struct timeval *timenow, const size_t bwused, 
         struct rule_t *rule, const int lock);
/*
 * update the time/bw counters in "rule" according to "bwused" and "timenow" 
 * "bwused" is the bandwidth used at time "timenow".
 */


static int rawsocket = -1;
static void
rawsocket_send(const int s, const struct sockshost_t *peer,
               const struct sockshost_t *local, const struct sockshost_t *dst,
               const int code);
/*
 * Handles packets output in on the raw socket "s".  Used
 * to send icmp-errors concerning packets we forward.
 * "peer" is the address that sent the problematic packet,
 * "local" is the address we received the packet on,
 * "dst" it the address the packet was intended for, and
 * "code" is the icmp code of the error.
 */



#if BAREFOOTD

#define UDP_MEMBLOCK (16)   /*
                                * by how many clients to increase allocated
                                * memory when needed.
                                */

#define MAX_ICMPLEN (60 /* MAX IP LEN */ + 8 + 60 + 8)
#define MIN_ICMPLEN (20 /* MIN IP LEN */ + 8)

static rawsocketstatus_t
rawsocket_recv(const int s);
/*
 * Handles packets input in on the raw socket "s".  Used
 * to read icmp-errors concerning packets we send to udp clients.
 */

static size_t
io_udpclients(void);
/*
 * Returns the number of active udp clients.
 */


static struct udpclient *
udpclientofladdr(const struct sockaddr *addr, const size_t udpclientc,
                 struct udpclient *udpclientv);
/*
 * Returns the udpclient that has the local address "addr", or NULL
 * if no such client exists.
 */

static struct udpclient *
udpclientofclientaddr(const struct sockaddr *addr, const size_t udpclientc,
                      struct udpclient *udpclientv);
/*
 * Returns the udpclient that has the client address "addr", or NULL
 * if no such client exists.
 */


static struct udpclient *
udpclientofsocket(const int s, const size_t udpclientc,
                  struct udpclient *udpclientv);
/*
 * Returns the udpclient belonging to socket "s", or NULL if no
 * such client.
 */

static struct udpclient *
addudpclient(const struct udpclient *client, size_t *clientc,
             size_t *maxclientc, struct udpclient **clientv);
/*
 * Adds the udpclient "client" to the "clientv" array, which is large
 * enough to contain "maxclientc" clients.
 * "clientc" gives the index of the last slot in "clientv" that is
 * currently in use.
 *
 * Returns a pointer to the added client ("client"), or NULL if there
 * is no more room and clientv can not be expanded.
 */

static int
removeudpclient(const int s, size_t *clientc, struct udpclient *clientv);
/*
 * Removes the udpclient associated with the socket "s" from the
 * "clientv" array, which contains "clientc" elements, and decrements
 * "clientc".
 * Returns 0 on success, -1 on failure.
 */

/*
 * In barefoot, udp is a point-to-multipoint case: we receive all client 
 * packets on one socket, but use different outgoing sockets for each client.
 * We use this macro to sync the necessary parts of the udpclient to the
 * io->dst object before usage.
 */
#define SYNC_UDPDST(dst, udpclient)                                            \
do {                                                                           \
   (dst)->s      = (udpclient)->s;                                             \
   (dst)->laddr  = (udpclient)->laddr;                                         \
   (dst)->raddr  = (udpclient)->raddr;                                         \
} while (/*CONSTCOND*/ 0)

#endif /* BAREFOOTD */

#define IOSTATUS_FATALERROR(error)     \
(!(  (error) == IO_NOERROR             \
  || (error) == IO_TMPERROR            \
  || (error) == IO_BLOCK               \
  || (error) == IO_EAGAIN))


static struct sockd_io_t iov[SOCKD_IOMAX];   /* each child has these ios. */
static const size_t ioc = ELEMENTS(iov);

/*
 * instead of including memory for this as part of the i/o object, we 
 * set up the pointer from the i/o object to the appropriate index into
 * this array when we receive the i/o object.  We can do that since
 * we only use the io.replyrule object in this process.
 *
 * The only reason for going through with this hassle is so we can reduce
 * the size of the i/o object.  Since the i/o object is passed around between
 * processes, we want it to be as small as possible, reducing the min-size
 * of the socketbuffers between mother and child.
 * XXX is this only needed in SOCKS_SERVER case?
 */
static struct rule_t fwdrulev[SOCKD_IOMAX];
static struct rule_t replyrulev[SOCKD_IOMAX];


/*
 * number of currently free file descriptors.  Should never become
 * a problem for Dante or Covenant, but Barefoot needs to keep track
 * of it so we does not end up using up all fds for udp clients, then
 * become unable to receive a new io from mother.
 */
static int freefds;

/*
 * if not 0, we have "overflowed" according to max bandwidth configured.
 * We can not attribute it to any given client though, so we penalize
 * all by delaying a little.  This object gives the earlist time at which we
 * can again do i/o over one of the object that has overflown it's bandwith
 * limit.
 */
static struct timeval bwoverflowtil;

void
run_io()
{
   const char *function = "run_io()";
   struct sigaction sigact;
   fd_set *rset, *wset, *xset, *newrset, *tmpset, *bufrset, *buffwset;
   int p;
#if BAREFOOTD
   socklen_t optlen;
#endif /* BAREFOOTD */

/* calls io_clearset() on all fd_sets. */
#define IO_CLEAR_ALL_SETS(io)       \
do {                                \
   io_clearset((io), rset);         \
   io_clearset((io), wset);         \
   io_clearset((io), xset);         \
   io_clearset((io), newrset);      \
   io_clearset((io), tmpset);       \
   io_clearset((io), bufrset);      \
   io_clearset((io), buffwset);     \
} while (/* CONSTCOND */ 0)


   bzero(&sigact, sizeof(sigact));
   sigact.sa_flags     = SA_RESTART | SA_SIGINFO;
   sigact.sa_sigaction = siginfo;

#if HAVE_SIGNAL_SIGINFO
   if (sigaction(SIGINFO, &sigact, NULL) != 0)
      serr(EXIT_FAILURE, "%s: sigaction(SIGINFO)", function);
#endif /* HAVE_SIGNAL_SIGINFO */

   /* same handler, for systems without SIGINFO. */
   if (sigaction(SIGUSR1, &sigact, NULL) != 0)
      serr(EXIT_FAILURE, "%s: sigaction(SIGUSR1)", function);

#if BAREFOOTD
   /*
    * Set up a raw socket so we can get icmp errors for
    * udp packets sent to clients, and delete the clients if so.
    */

   sockd_priv(SOCKD_PRIV_NET_ICMPACCESS, PRIV_ON);
   if ((rawsocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
      swarn("%s: could not open raw socket", function);
   else
      slog(LOG_DEBUG, "%s: created raw socket, index %d", function, rawsocket);
   sockd_priv(SOCKD_PRIV_NET_ICMPACCESS, PRIV_OFF);

#if HAVE_PRIVILEGES
   /* don't need this privilege any more, permanently loose it. */

   if (sockscf.privileges.haveprivs) {
      priv_delset(sockscf.privileges.privileged, PRIV_NET_ICMPACCESS);
      if (setppriv(PRIV_SET, PRIV_PERMITTED, sockscf.privileges.privileged)
      != 0)
         swarn("%s: setppriv() to relinquish PRIV_NET_ICMPACCESS failed",
         function);
   }
#endif /* HAVE_PRIVILEGES */

   if (rawsocket != -1) {
      optlen = sizeof(p);
      if (getsockopt(rawsocket, SOL_SOCKET, SO_RCVBUF, &p, &optlen) != 0)
         swarn("%s: getsockopt(SO_RCVBUF)", function);
      else {
         if (p < RAW_SOCKETBUFFER) {
            p = RAW_SOCKETBUFFER;
            if (setsockopt(rawsocket, SOL_SOCKET, SO_RCVBUF, &p, sizeof(p))
            != 0)
               swarn("%s: failed setsockopt(SO_RCVBUF, %d) on raw socket",
               function, p);
            else
               slog(LOG_DEBUG, "%s: changed buffer size to %d bytes",
               function, p);
         }
         else
            slog(LOG_DEBUG, "%s: default buffer size is %d bytes, keeping it",
            function, p);
      }
   }

#endif /* BAREFOOTD */

   rset       = allocate_maxsize_fdset();
   wset       = allocate_maxsize_fdset();
   xset       = allocate_maxsize_fdset();
   newrset    = allocate_maxsize_fdset();
   tmpset     = allocate_maxsize_fdset();
   bufrset    = allocate_maxsize_fdset();
   buffwset   = allocate_maxsize_fdset();

   proctitleupdate();

   freefds = freedescriptors(NULL);
   slog(LOG_DEBUG, "%s: ready to serve with %d free file descriptors",
        function, freefds);

   /* CONSTCOND */
   while (1) {
      /*
       * The heart and soul of the server.  This is the loop where
       * all i/o is done and involves some tricky stuff.
       *
       * We need to check for write separately to avoid busy-looping.
       * The problem is that if the descriptor is ready for reading but
       * the corresponding descriptor to write out on is not ready, we will
       * be busy-looping; above select will keep returning descriptors set,
       * but we will not be able to write (and thus won't read) them.
       *
       * Our solution to this is two select(2) calls.  One to see
       * what descriptors are readable, and another select(2) call to
       * block until at least one of the descriptors on the corresponding
       * write-side has become writable.
       *
       * We therefore only set in wset the descriptors that have the
       * corresponding read descriptor readable, so that when the
       * second select() returns, the io objects we get from wset will
       * be both readable and writable.
       *
       * Another problem is that if while we wait for writability, a new
       * descriptor becomes readable, we thus can't block forever here.
       * We solve this by in the second select() also checking for
       * readability, but now only the descriptors that were not found
       * to be readable in the previous select().
       * This means that a positive return from the second select does not
       * necessarily indicate we have i/o to do, but it does mean we
       * either have it or a new descriptor became readable; in either
       * case, something has happened.
       * Reason we do not check for exceptions in this select is that
       * there is nothing we do about them until the descriptor becomes
       * readable too, thus any new exceptions will be in newrset before
       * we have reason to care about them.
       */
      iostatus_t iostatus;
      struct sockd_io_t *io;
      struct timeval timeout, *timeoutpointer;
      int i, bits, first_rbits, rbits, wbits, newsocketsconnected, badfd, flags;

      errno = 0; /* reset for each iteration. */

      /* look for timed-out clients and calculate the next timeout, if any. */
      while ((io = io_gettimedout()) != NULL)
         delete_io(sockscf.state.mother.ack, io, -1, IO_TIMEOUT);

      io_fillset(xset, MSG_OOB, NULL);
      rbits = io_fillset(rset, 0, &bwoverflowtil);

      /*
       * buffwset.  What descriptors do we want to check whether have
       * data buffered for write?  Having data buffered for write means
       * we have data to write on them, thus we want to know if they
       * are writable.  Pretty much any client-related descriptor we want
       * to check for having data buffered for write, except those
       * specifically skipped (due to e.g., bw overflow).
       */
      FD_COPY(buffwset, rset);

      /* likewise for having data buffered for read. */
      FD_COPY(bufrset, rset);

      if (sockscf.state.mother.s != -1) {
         FD_SET(sockscf.state.mother.s, rset);
         rbits = MAX(rbits, sockscf.state.mother.s);

         /* checked so we know if mother goes away.  */
         FD_SET(sockscf.state.mother.ack, rset);
         rbits = MAX(rbits, sockscf.state.mother.ack);
      }
      else { /* no mother.  Do we have any other descriptors to work with? */
         if (rbits == -1 && !timerisset(&bwoverflowtil)) {
            /* no clients in fd_sets, and not due to bwoverflow ... */
            SASSERTX(io_allocated(NULL, NULL) == 0);

            slog(LOG_DEBUG, "%s: no connection to mother and no clients.  "
                            "Exiting",
                            function);

            sockdexit(EXIT_SUCCESS);
         }
      }
#if BAREFOOTD
      if (rawsocket != -1 && (io_udpclients() > 0)) {
      /* raw socket is only of interest if we have udp clients. */

         FD_SET(rawsocket, rset);
         rbits = MAX(rbits, rawsocket);
      }
#endif /* BAREFOOTD */

      /*
       * First find descriptors that are readable; we won't write if
       * we can't read.
       * Connects that are in progress is a special case that we also need
       * to check for here.  Once the connect has completed, successfully or
       * not, the socket will become writable and we may need to send a
       * response to the client.
       *
       * Also select for exceptions so we can tell the i/o function if
       * there's one pending later.
       */

      wbits               = io_fillset_connectinprogress(wset);
      newsocketsconnected = 0;
      bits                = MAX(rbits, wbits) + 1;

      slog(LOG_DEBUG,
           "%s: first select.  What is readable, what has finished connecting",
           function);
      switch (selectn(bits,
                      rset,
                      bufrset,
                      buffwset,
                      wset,
                      xset,
                      io_gettimeout(&timeout))) {
         case -1:
            if (errno == EINTR)
               continue;

            SERR(errno);
            /* NOTREACHED */

         case 0:
            continue; /* restart the loop. */
      }

      if (sockscf.state.mother.ack != -1
      && FD_ISSET(sockscf.state.mother.ack, rset)) { /* only eof expected. */
         slog(LOG_DEBUG, "%s: mother closed the connection to us",
         function);

         sockscf.state.mother.s = sockscf.state.mother.ack = -1;

#if BAREFOOTD
         /*
          * terminate all udp sessions.  If not, a restart will not
          * be able to rebind the ports used.
          */
         io_remove_session(NULL, SOCKS_UDP);
#endif /* BAREFOOTD */

         continue; /* safest to regenerate the fd_sets. */
      }

      /* needs to be after check of ack-pipe to limit error messages. */
      if (sockscf.state.mother.s != -1
      && FD_ISSET(sockscf.state.mother.s, rset)) {
         getnewios();
         continue; /* need to scan rset again; should have a new client. */
                   /*
                    * XXX Or can we instead add it to newrset, and rescan as    
                    * normal after that?
                    */

      }

      first_rbits = bits;

      FD_ZERO(tmpset);
      if (FD_CMP(tmpset, wset) != 0) {
         for (p = 0; p < bits; ++p)
            if (FD_ISSET(p, wset)) {
               int badfd;

               io = io_finddescriptor(p);
               SASSERTX(io != NULL);
               SASSERTX(p == io->dst.s);

               badfd = -1;
               if (checkconnectstatus(io, &badfd) == 0)
                  ++newsocketsconnected;
               else {
                  SASSERTX(badfd != -1);
                  SASSERTX(badfd == io->src.s
                  ||       badfd == io->dst.s
                  ||       badfd == io->control.s);

                  IO_CLEAR_ALL_SETS(io);
                  delete_io(sockscf.state.mother.ack, io, badfd, IO_ERROR);
               }
            }

         slog(LOG_DEBUG, "%s: %d new socket%s finished connecting",
              function,
              newsocketsconnected,
              newsocketsconnected == 1 ? "" : "s");
      }

      /*
       * Add bufrset to rset, so rset contains all sockets we can
       * read from, whether it's from the socket or from the buffer.
       */
      fdsetop(bits, '|', rset, bufrset, rset);

#if BAREFOOTD
      if (rawsocket != -1 && FD_ISSET(rawsocket, rset)) {
         FD_CLR(rawsocket, rset);

         if (rawsocket_recv(rawsocket) == RAWSOCKET_IO_DELETED)
            /*
             * one or more ios were deleted.  Don't know which, so
             * need to regenerate the descriptor sets for select.
             */
            continue;
      }
#endif /* BAREFOOTD */

      /*
       * We now know what descriptors are readable; rset.
       * Next prepare for the second select(2), where we want to
       * know which of the descriptors, paired with the above readable
       * descriptors, we can write to.  In that select(2) we also need to
       * check for read again, but only those descriptors that are not
       * already readable, as that constitutes at least a status change
       * which we should loop around for.  Likewise, we again need to
       * check whether some new sockets have finished connecting (and
       * thus become writable).
       */

      i     = io_fillset(tmpset, 0, &bwoverflowtil);
      rbits = fdsetop(i, '^', rset, tmpset, newrset);

      if (sockscf.state.mother.s != -1) { /* mother status may change too. */
         FD_SET(sockscf.state.mother.s, newrset);
         rbits = MAX(rbits, sockscf.state.mother.s);

         /* checked so we know if mother goes away.  */
         SASSERTX(sockscf.state.mother.ack != -1);
         FD_SET(sockscf.state.mother.ack, newrset);
         rbits = MAX(rbits, sockscf.state.mother.ack);
      }

      /*
       * descriptors to check for writability: those with the corresponding
       * read-descriptor set or data already buffered for write, as well
       * as the connects that are still in progress.
       * Initialize with the set of connects still in progress, and then add
       * those fds that have the corresponding other side readable.
       */
      wbits = io_fillset_connectinprogress(wset);

      for (p = 0; p < MAX(bits, first_rbits); ++p) {
         if (FD_ISSET(p, buffwset)) {
            /*
             * Descriptor has data buffered for write.  That means
             * we should mark the other side as readable as regardless
             * of whether we can read from the other side or not now, we have
             * data that we previously read from it and that we need to write.
             */
            int other_side;

            io = io_finddescriptor(p);
            SASSERTX(io != NULL);
            SASSERTX(socks_bufferhasbytes(p, WRITE_BUF));

            if (p == io->src.s)
               other_side = io->dst.s;
            else if (p == io->dst.s)
               other_side = io->src.s;
            else
               SERRX(p);

            slog(LOG_DEBUG, "%s: fd %d has data buffered for write; "
                            "checking it for writability and marking "
                            "other side, fd %d, as readable",
                            function, p, other_side);

            FD_SET(other_side, rset);
            rbits = MAX(other_side, rbits);

            /*
             * ok, we know we have data buffered for write, but /can/
             * we write?  Need to check.
             * XXX possible optimization target: if we have enough room
             *     in the writebuffer, we can pretend the fd is writable
             *     as long as we do not read too much (gssapi encapsulation
             *     included).  As of now, we just use the buffer to even
             *     out differences between the two sides, but if one side
             *     stops readinging completly, the fd will not be writable
             *     and we won't try to write anything.  No big deal.
             */
            FD_SET(p, wset);
            wbits = MAX(wbits, p);
         }
         else {
            /*
             * No data buffered for write.  Is the socket readable,
             * from the buffer or from the socket itself?
             */

            if (!FD_ISSET(p, rset))
               continue; /* no. */

            /*
             * Yes, have data to read.
             */
            io = io_finddescriptor(p);
            SASSERTX(io != NULL);

            /*
             * find out what sockets we should check for writability.
             */

#if BAREFOOTD
            /*
             * The tcp case is the same as socks, but in the case of udp,
             * we have a one-to-many scenario, where packets received
             * on "in" can go to many different "outs.", and we don't
             * know which out socket to use until we have read the packet,
             * to see who the sender is.
             *
             * Udp sockets shouldn't block though, so selecting
             * for writability is not really required.  Thus,
             * we just need to make sure at all times that dst.s
             * always contains one of the valid out sockets.  If
             * one is writable, all should be.
             *
             * The reverse, when a packet comes in on one of the many
             * out sockets is slightly more complicated. In this case,
             * we need to select for readability on all the many out
             * sockets.  This is handled in io_fillset(), but we also
             * need to make sure that io->dst.s is set to the (possibly
             * one of many) descriptors in dstv[] that is readable when
             * we call doio().
             */
#endif /* BAREFOOTD */

            if (io->src.s == p) { /* read from in requires writable out. */
               FD_SET(io->dst.s, wset);
               wbits = MAX(wbits, io->dst.s);
            }
            else if (io->dst.s == p) { /* read from out requires writable in. */
               FD_SET(io->src.s, wset);
               wbits = MAX(wbits, io->src.s);
            }
            else {
               /*
                * controlconnection is also readable without matching writable
                * and is used to signal session close in udp and bind extension 
                * cases.
                * Since it doesn't have any side to forward the data to,
                * it is more optimal to read from it now so we can both
                * simplify the rest of the code and make it faster.
                */
               ssize_t r;
               char buf[1024];

               SASSERTX(io->control.s == p);
               SASSERTX(io->control.s != io->src.s); 
               SASSERTX(io->control.s != io->dst.s);

               SASSERTX(io->state.command  == SOCKS_UDPASSOCIATE 
               ||       (io->state.command == SOCKS_BINDREPLY 
                  &&     io->state.extension.bind));

               /*
                * Only thing we expect from client's control connection is
                * an eof.
                */
               r = socks_recvfrom(io->control.s,
                                  buf,
                                  sizeof(buf),
                                  0,
                                  NULL,
                                  NULL,
                                  &io->control.auth);


               if (r <= 0) {
                  IO_CLEAR_ALL_SETS(io);
                  delete_io(sockscf.state.mother.ack,
                            io,
                            io->control.s,
                            r == 0 ? IO_CLOSE : IO_ERROR);
               }
               else {
                  char visbuf[256];

                  slog(LOG_NOTICE, "%s: %ld unexpected byte%s over control "
                                   "connection from client %s:  %s",
                                   function,
                                   (long)r,
                                   r == 1 ? "" : "s",
                                   sockaddr2string(&io->control.raddr, NULL, 0),
                                   str2vis(buf, r, visbuf, sizeof(visbuf)));

                  FD_CLR(io->control.s, rset);

                  FD_SET(io->control.s, newrset);
                  rbits = MAX(io->control.s, rbits);
               }
            }
         }
      }

      if (wbits++ == -1)
         continue;

      if (newsocketsconnected) {
         /*
          * Don't wait.  Handle what we can now and then restart the loop,
          * which will then include handling of any new sockets.
          */
         bzero(&timeout, sizeof(timeout));
         timeoutpointer = &timeout;
      }
      else
         timeoutpointer = io_gettimeout(&timeout);

      bits = MAX(rbits, wbits) + 1;

      slog(LOG_DEBUG, "%s: second select, what is writable?", function);
      switch (selectn(bits, newrset, NULL, NULL, wset, NULL, timeoutpointer)) {
         case -1:
            if (errno == EINTR)
               continue;

            SERR(-1);
            /* NOTREACHED */

         case 0:
            continue;
      }

      if (sockscf.state.mother.ack != -1
      && FD_ISSET(sockscf.state.mother.ack, newrset))
         continue; /* eof presumably; handle it in one place, above. */

      if (sockscf.state.mother.s != -1
      && FD_ISSET(sockscf.state.mother.s, newrset)) {
         FD_CLR(sockscf.state.mother.s, newrset);
         getnewios();
      }

      /*
       * newrset: descriptors readable.  All new fds. 
       *          Don't do anything with them here as we don't know if 
       *          the write-side is writable; loop around and check for
       *          writability first.
       *
       * rset: descriptors readable, from buffer or from socket.
       *
       * xset: subset of rset with exceptions pending.
       *
       * wset: descriptors writable with at least one of:
       *          a) a matching descriptor in rset.
       *          b) data buffered on the write side.
       *          c) a connect previously in progress, and now completed.
       *       a) and b) we can do i/o over, c) we can't know for sure.
       */


      /*
       * Get all ios which are writable.  They will either have a matching
       * descriptor that is readable, data buffered for write, or belong to
       * an io that has just finished connecting, successfully or otherwise.
       */
      FD_ZERO(tmpset);
      while ((io = io_getset(bits, wset)) != NULL) {
         if (io->state.command == SOCKS_CONNECT
         && !io->dst.state.connected) {
            int badfd;

            SASSERTX(FD_ISSET(io->dst.s, wset));
            FD_CLR(io->dst.s, wset);

            badfd = -1;
            if (checkconnectstatus(io, &badfd) == -1) {
               SASSERTX(badfd != -1);
               SASSERTX(badfd == io->src.s || badfd == io->dst.s);

               delete_io(sockscf.state.mother.ack, io, badfd, IO_ERROR);
            }

            /*
             * regardless of whether the connect was successfull or not we
             * can't do anything here as we don't know the status of src.s is,
             * so have to loop around.
             * XXX we could check though --- if src is readable,
             * "coincidentally",  and the connect succeeded, we can do i/o?
             */
            continue;
         }

#if BAREFOOTD
         if (io->state.command == SOCKS_UDPASSOCIATE) {
            /*
             * Since udp is a "point-to-multipoint" case, the
             * descriptors in rset could be any of the descriptors in
             * io->dstv, or the one in io->src.s.
             *
             * If it's the former, we need to find out which one(s) it is,
             * and set io->dst.s appropriately.
             *
             * If it's the later, there could be many packets queued
             * up, especially since that one socket will be handling
             * io->dstc number of clients.  If so, we'll want to \
             * read from the socket until there are no more packets,
             * to reduce the chance of packets being dropped because
             * the socket buffer runs full.
             */
            size_t i;

            /*
             * possibly wset contains the dummy fd only, but
             * we want to treat the udp socket as always
             * writable, no mater which udp socket it is.
             */
            FD_SET(io->dst.s, wset);

            if (FD_ISSET(io->src.s, rset)) {
               do
                  iostatus = doio(io, rset, wset, 0, &badfd);
               while (!timerisset(&bwoverflowtil)
               &&     io->dstc > 1
               &&     iostatus != IO_EAGAIN
               &&     !IOSTATUS_FATALERROR(iostatus));

               FD_CLR(io->dst.s, wset);
               for (i = 0; i < io->dstc; ++i)
                  FD_CLR(io->dstv[i].s, wset);
            }
            else {
               /*
                 * reply from at least one destination.  Loop through all.
                 * Don't care about about bwoverflow here, as if we do,
                 * we might end up only reading packets from the first
                 * range of clients for a long while.  More fair to read
                 * from all, and then pause if necessary.
                 */
               for (i = 1; i < io->dstc; ++i) {
                  if (FD_ISSET(io->dstv[i].s, rset)) {
                     SYNC_UDPDST(&io->dst, &io->dstv[i]);
                     iostatus = doio(io, rset, wset, 0, &badfd);
                     FD_CLR(io->dst.s, wset);

                     if (IOSTATUS_FATALERROR(iostatus))
                        delete_io(sockscf.state.mother.ack,
                                  io,
                                  badfd,
                                  iostatus);
                  }
               }
            }

            io_clearset(io, rset);
            io_clearset(io, wset);
         }
         else { /* tcp; same as Dante. */
#endif /* BAREFOOTD */

         if (FD_CMP(tmpset, xset) != 0
         && (FD_ISSET(io->src.s, xset) || FD_ISSET(io->dst.s, xset)))
            flags = MSG_OOB;
         else
            flags = 0;

         iostatus = doio(io, rset, wset, flags, &badfd);
         io_clearset(io, rset);
         io_clearset(io, wset);

         if (IOSTATUS_FATALERROR(iostatus))
            delete_io(sockscf.state.mother.ack, io, badfd, iostatus);
#if BAREFOOTD
         }
#endif /* BAREFOOTD */
      }

      /* possible future optimization: if newrset not empty, use it? */
   }
}

void
io_handlesighup(void)
{
   const char *function = "io_handlesighup()";
   size_t i;

   slog(LOG_DEBUG, "%s: ioc = %ld", function, (long)ioc);

   for (i = 0; i < ioc; ++i) {
#if BAREFOOTD
      size_t clienti;
#endif /* BAREFOOTD */

      if (!iov[i].allocated)
         continue;

      iov[i].use_saved_rule      = 0;
      iov[i].use_saved_replyrule = 0;

#if BAREFOOTD
      slog(LOG_DEBUG, "%s: iov #%ld, dstc = %ld",
      function, (long)ioc, (long)iov[i].dstc);

      for (clienti = 1; clienti < iov[i].dstc; ++clienti) {
         iov[i].dstv[clienti].use_saved_rule      = 0;
         iov[i].dstv[clienti].use_saved_replyrule = 0;
      }
#endif /* BAREFOOTD */
   }
}


#if BAREFOOTD
int
io_remove_session(laddr, protocol)
   const struct sockaddr  *laddr;
   const int protocol;
{
   const char *function = "io_remove_session()";
   size_t i;

   SASSERTX(protocol == SOCKS_UDP);

   slog(LOG_DEBUG, "%s: searching for local address %s",
   function, laddr == NULL ? "<any>" : sockaddr2string(laddr, NULL, 0));

   for (i = 0; i < ioc; ++i) {
      size_t clienti;

      if (!iov[i].allocated
      || iov[i].state.protocol != protocol)
         continue;

      if (laddr != NULL
      &&  memcmp(&iov[i].src.laddr, laddr, sizeof(*laddr)) != 0)
         continue;

      slog(LOG_DEBUG, "removing iov #%lu with %lu active udp session%s in "
                      "child %lu",
                      (unsigned long)i,
                      (unsigned long)iov[i].dstc - 1,
                      (iov[i].dstc - 1) == 1 ? "" : "s",
                      (unsigned long)sockscf.state.pid);


      close(iov[i].src.s); /* delete_io() does not close udp src in barefoot. */
      for (clienti = 1; clienti < iov[i].dstc; ++clienti) {
         iov[i].rule  = iov[i].dstv[clienti].rule;
         iov[i].crule = iov[i].dstv[clienti].crule;

         SYNC_UDPDST(&iov[i].dst, &iov[i].dstv[clienti]);
         delete_io(-1, &iov[i], -1, IO_ADMINTERMINATION);
      }
      SYNC_UDPDST(&iov[i].dst, &iov[i].dstv[0]);

      freebuffers(&iov[i]);
      free(iov[i].dstv);

      iov[i].allocated = 0;

      if (sockscf.state.mother.ack != -1) {
         const char p = SOCKD_FREESLOT_UDP;

         /* ack io slot free. */
         if (socks_sendton(sockscf.state.mother.ack, &p, sizeof(p), sizeof(p),
         0, NULL, 0, NULL) != sizeof(p))
             swarn("%s: socks_sendton(): mother", function);
      }

      proctitleupdate();
      return 0;
   }

   return -1;
}
#endif /* BAREFOOTD */

static void
delete_io(mother, io, fd, status)
   int mother;
   struct sockd_io_t *io;
   int fd;
   const iostatus_t status;
{
   const char *function = "delete_io()";
   const int errno_s = errno;
   size_t i;
   struct timeval tnow, timeestablished;
   uint64_t src_read, src_written, dst_read, dst_written,
            src_packetsread, src_packetswritten, dst_packetsread,
            dst_packetswritten;
   struct rule_t *rulev[] = { &io->rule,
                              &io->crule };
   int isclientrulev[] =    { 0,
                              1 };
   int command, protocol;
#if HAVE_GSSAPI
   gss_buffer_desc output_token;
   OM_uint32 minor_status;
#endif /* HAVE_GSSAPI */

   SASSERTX(  fd < 0
           || fd == io->src.s
           || fd == io->control.s
           || fd == io->dst.s);

   SASSERTX(io->allocated);

   SHMEM_UNUSE(&io->rule,
               &(TOIN(&io->control.raddr)->sin_addr),
               sockscf.shmemfd);

   gettimeofday(&tnow, NULL);

   /* log the disconnect if client-rule or socks-rule says so. */
   for (i = 0; i < ELEMENTS(rulev); ++i) {
      const struct rule_t *rule = rulev[i];
      size_t bufused;
      char in[MAX_IOLOGADDR], out[MAX_IOLOGADDR],
           timeinfo[512],
           logmsg[sizeof(in) + sizeof(out) + 1024 + sizeof(timeinfo)];

#if !HAVE_TWO_LEVEL_ACL
      if (rule == &io->crule)
         if (!sockscf.option.debug)
            continue; /* normally, only log from the socks-rule. */
#endif /* HAVE_TWO_LEVEL_ACL */

      if (rule->log.disconnect
      || (rule->log.error && status == IO_ERROR))
         /* LINTED */ /* EMPTY */;
      else
         continue;

      src_written        = io->src.written.bytes;
      src_packetswritten = io->src.written.packets;

      src_read           = io->src.read.bytes;
      src_packetsread    = io->src.read.packets;

      dst_written        = io->dst.written.bytes;
      dst_packetswritten = io->dst.written.packets;

      dst_read           = io->dst.read.bytes;
      dst_packetsread    = io->dst.read.packets;

      timeestablished    = io->state.time.established;

      if (isclientrulev[i]) {
#if BAREFOOTD
         if (io->state.protocol == SOCKS_UDP)
            /*
             * In udp-case, we copy logstate from client-rule
             * to socks-rule, as we need to do a rulecheck on
             * each packet.  Since we don't want to log twice,
             * we need to ignore the logstate of the client-rule.
             */
            continue;
#endif

         /*
          * XXX if support for server chaining is added to bind, the
          * bindreply might involve a proxy on the src side.
          */
         BUILD_ADDRSTR_SRC(&io->control.host,
                           NULL,
                           NULL,
                           &io->control.laddr,
                           &io->clientauth,
                           NULL,
                           in,
                           sizeof(in));

         *out = NUL; /* client-rule is from client to socks-server, and stop. */

         command  = io->state.clientcommand;
         protocol = io->state.clientprotocol;
      }
      else { /* socks rule. */
         BUILD_ADDRSTR_SRC(&io->src.host,
                           NULL,
                           NULL,
                           &io->src.laddr,
                           &io->src.auth,
                           NULL,
                           in,
                           sizeof(in));

         switch (io->state.command) {
            case SOCKS_BIND:
            case SOCKS_BINDREPLY:
            case SOCKS_CONNECT: {
               BUILD_ADDRSTR_DST(&io->dst.laddr,
                                 io->state.proxyprotocol == PROXY_DIRECT
                                    ? NULL : &io->state.proxychain.server,
                                 io->state.proxyprotocol == PROXY_DIRECT
                                    ? NULL : &io->state.proxychain.extaddr,
                                 &io->dst.host,
                                 &io->dst.auth,
                                 NULL,
                                 out,
                                 sizeof(out));
               break;
            }

            case SOCKS_UDPASSOCIATE: {
#if BAREFOOTD
               struct udpclient *client;
#endif /* BAREFOOTD */
               if (io->dst.state.connected)
                  BUILD_ADDRSTR_DST(&io->dst.laddr,
                                    io->state.proxyprotocol == PROXY_DIRECT
                                       ? NULL : &io->state.proxychain.server,
                                    io->state.proxyprotocol == PROXY_DIRECT
                                       ? NULL : &io->state.proxychain.extaddr,
                                    &io->dst.host,
                                    &io->dst.auth,
                                    NULL,
                                    out,
                                    sizeof(out));
               else
                  BUILD_ADDRSTR_DST(&io->dst.laddr,
                                    io->state.proxyprotocol == PROXY_DIRECT
                                       ? NULL : &io->state.proxychain.server,
                                    io->state.proxyprotocol == PROXY_DIRECT
                                       ? NULL : &io->state.proxychain.extaddr,
                                    NULL,
                                    &io->dst.auth,
                                    NULL,
                                    out,
                                    sizeof(out));
#if BAREFOOTD
               client = udpclientofsocket(io->dst.s, io->dstc, io->dstv);
               SASSERTX(client != NULL);

               src_read           = client->src_read.bytes;
               src_packetsread    = client->src_read.packets;

               src_written        = client->src_written.bytes;
               src_packetswritten = client->src_written.packets;

               dst_written        = client->dst_written.bytes;
               dst_packetswritten = client->dst_written.packets;

               dst_read           = client->dst_read.bytes;
               dst_packetsread    = client->dst_read.packets;

               timeestablished    = client->firstio;
#endif /* BAREFOOTD */

               break;
            }

            default:
               SERRX(io->state.command);
         }

         command  = io->state.command;
         protocol = io->state.protocol;
      }

      bufused = snprintf(logmsg, sizeof(logmsg), "%s(%lu): %s/%s ]: ",
                         rule->verdict == VERDICT_PASS ?
                         VERDICT_PASSs : VERDICT_BLOCKs,
#if BAREFOOTD
                        /* use the number from the user-created rule. */
                        io->state.protocol == SOCKS_UDP ?
                          (unsigned long)io->crule.number
                        : (unsigned long)io->rule.number,
#else /* !BAREFOOTD */
                        (unsigned long)io->rule.number,
#endif /* !BAREFOOTD */
                         protocol2string(protocol),
                         command2string(command));

      if (protocol == SOCKS_TCP) {
         if (*out == NUL) {
            const int isreversed
            = (io->state.command == SOCKS_BINDREPLY ? 1 : 0);

            bufused
            += snprintf(&logmsg[bufused], sizeof(logmsg) - bufused,
                       "%"PRIu64" -> %s -> %"PRIu64"",
                       (isreversed ? dst_written : src_written),
                       in,
                       (isreversed ? src_written  : dst_written));
         }
         else
            bufused
            += snprintf(&logmsg[bufused], sizeof(logmsg) - bufused,
                       "%"PRIu64" -> %s -> %"PRIu64", "
                       "%"PRIu64" -> %s -> %"PRIu64"",
                       src_written, in, src_read,
                       dst_written, out, dst_read);
      }
      else {
         SASSERTX(*out != NUL);

         bufused += snprintf(&logmsg[bufused], sizeof(logmsg) - bufused,
                            "%"PRIu64"/%"PRIu64" -> %s -> %"PRIu64"/%"PRIu64", "
                            "%"PRIu64"/%"PRIu64" -> %s -> %"PRIu64"/%"PRIu64"",
                            src_written, src_packetswritten, in,
                            src_read, src_packetsread,
                            dst_written, dst_packetswritten, out,
                            dst_read, dst_packetsread);
      }

      bufused = snprintf(timeinfo, sizeof(timeinfo), "Session duration: %lds",
                         (long)(tnow.tv_sec - timeestablished.tv_sec));

      if (sockscf.option.debug
#if BAREFOOTD
      &&  protocol == SOCKS_TCP
#endif /* BAREFOOTD */
      ) {
         struct timeval accept2negotiate, neg2establish, sessionduration;
         char established2io_str[16];

         timersub(&io->state.time.negotiate, &io->state.time.accepted,
                  &accept2negotiate);

         timersub(&io->state.time.established, &io->state.time.negotiate,
                  &neg2establish);

         if (io->state.time.firstio.tv_sec == 0) {
            SASSERTX(sizeof(established2io_str) >= sizeof("N/A"));
            strcpy(established2io_str, "N/A");
         }
         else {
            struct timeval established2io;

            timersub(&io->state.time.firstio, &io->state.time.established,
                     &established2io);

            snprintf(established2io_str, sizeof(established2io_str),
                     "%ld.%06lds",
                    (long)established2io.tv_sec,
                    (long)established2io.tv_usec);
         }

         timersub(&tnow, &io->state.time.accepted, &sessionduration);

         bufused += snprintf(&timeinfo[bufused], sizeof(timeinfo) - bufused,
                             "\n"
                             "accept to negotiate start       : %ld.%06lds\n"
                             "negotiate start to finish       : %ld.%06lds\n"
                             "session establish to first i/o  : %s\n"
                             "total session duration          : %ld.%06lds\n",
                             (long)accept2negotiate.tv_sec,
                             (long)accept2negotiate.tv_usec,
                             (long)neg2establish.tv_sec,
                             (long)neg2establish.tv_usec,
                             established2io_str,
                             (long)sessionduration.tv_sec,
                             (long)sessionduration.tv_usec);
      }

      errno = errno_s;
      switch (status) {
         case IO_BLOCK:
            slog(LOG_INFO, "%s: blocked.  %s", logmsg, timeinfo);
            break;

         case IO_ERROR: {
            char errbuf[128];

            if (errno != 0)
               snprintf(errbuf, sizeof(errbuf), " (%s)", strerror(errno));
            else
               *errbuf = NUL;

            slog(LOG_INFO, "%s: %s error%s.  %s",
                 logmsg, 
                 fd < 0 ? "session"
                        : fd == io->dst.s ? "remote peer" : "local client",
                 errbuf,
                 timeinfo);

            if (fd >= 0) { /* try to generate a rst for the other end. */
               struct linger linger;

               linger.l_onoff  = 1;
               linger.l_linger = 0;

               if (setsockopt(fd == io->dst.s ? io->src.s : io->dst.s,
                              SOL_SOCKET,
                              SO_LINGER,
                              &linger,
                              sizeof(linger)) != 0)
                  slog(LOG_DEBUG,
                       "%s: setsockopt(io->dst, SO_LINGER) failed: %s",
                       function, strerror(errno));
            }
            break;
         }

         case IO_CLOSE:
            slog(LOG_INFO, "%s: %s closed.  %s",
                logmsg,
                fd < 0 ? "session" : fd == io->dst.s ?
                "remote peer" : "local client",
                timeinfo);
            break;

         case IO_TIMEOUT: {
            const char *timeoutinfo;
            timeout_type_t timeouttype;
            long timetiltimeout;

            timetiltimeout = io_timetiltimeout(io, &tnow, &timeouttype);
            SASSERTX(timetiltimeout <= 0);

            switch (timeouttype) {
               case TIMEOUT_CONNECT:
                  timeoutinfo = "the connect to complete";
                  break;

               case TIMEOUT_IO:
                  timeoutinfo = "more data";
                  break;

               case TIMEOUT_TCP_FIN_WAIT:
                  SASSERTX(io->src.state.shutdown_wr
                  ||       io->dst.state.shutdown_wr);

                  if (io->dst.state.shutdown_wr)
                     timeoutinfo = "the client to close the connection";
                  else
                     timeoutinfo = "the remote peer to close the connection";
                  break;

              default:
               SERRX(timeouttype);
            }

            slog(LOG_INFO, "%s: timeout while waiting for %s.  %s",
            logmsg, timeoutinfo, timeinfo);

            break;
         }

         case IO_ADMINTERMINATION:
            slog(LOG_INFO, "%s: administrative termination.  %s",
                 logmsg, timeinfo);
            break;

         default:
            SERRX(status);
      }

      if (io->state.command == SOCKS_BINDREPLY && !isclientrulev[i]) {
         /*
          * log the close of the open'd bind session also.
          */
          struct sockshost_t host_remote;

         const int original_command = io->state.command;

         sockaddr2sockshost(&io->dst.raddr, &host_remote);

         /*
          * bindreply src/dst order is reversed compared to bind.
          */
         io->state.command = SOCKS_BIND;
         iolog(&io->cmd.bind.rule,
               &io->state,
               OPERATION_DISCONNECT,
               &io->dst.laddr,
               &host_remote,
               &io->dst.auth,
               NULL,
               NULL,
               NULL,
               &io->src.laddr,
               io->state.extension.bind ? NULL : &io->cmd.bind.host,
               &io->src.auth,
               NULL,
               NULL,
               NULL,
               NULL,
               0);
         io->state.command = original_command;
      }
   }

#if HAVE_GSSAPI
   if (io->src.auth.method == AUTHMETHOD_GSSAPI) {
      if (gss_delete_sec_context(&minor_status,
      &io->src.auth.mdata.gssapi.state.id, &output_token) != GSS_S_COMPLETE)
         swarnx("%s: gss_delete_sec_context of src failed", function);

      CLEAN_GSS_TOKEN(output_token);
   }

   if (io->dst.auth.method == AUTHMETHOD_GSSAPI) {
      if (gss_delete_sec_context(&minor_status,
      &io->dst.auth.mdata.gssapi.state.id, &output_token) != GSS_S_COMPLETE)
         swarnx("%s: gss_delete_sec_context of dst failed", function);

      CLEAN_GSS_TOKEN(output_token);
   }
#endif /* HAVE_GSSAPI */


#if BAREFOOTD
   if (io->state.command == SOCKS_UDPASSOCIATE) {
      /*
       * The io itself is normally not freed in the udp-case, as we can
       * always get new clients; only the client is removed.
       * The only exception to this is if sockd.conf is changed before
       * a sighup.  It may then be we should no longer maintain a
       * udp session for these clients, but that needs to be handled
       * specially.
       */
      removeudpclient(io->dst.s, &io->dstc, io->dstv);
   }
   else { /* not UDP, must be TCP.  Free io as usual then. */
#endif /* BAREFOOTD */

   freebuffers(io);
   close_iodescriptors(io);

   if (mother != -1) {
      const char b = io->state.command
      == SOCKS_UDPASSOCIATE ? SOCKD_FREESLOT_UDP : SOCKD_FREESLOT_TCP;

      /* ack io slot free. */
      if (socks_sendton(mother, &b, sizeof(b), sizeof(b), 0, NULL, 0, NULL)
      != sizeof(b))
          swarn("%s: socks_sendton(): mother", function);
   }

   bzero(io, sizeof(*io));
   proctitleupdate();

#if BAREFOOTD
   }
#endif /* BAREFOOTD */
}

void
close_iodescriptors(io)
   const struct sockd_io_t *io;
{
   const char *function = "close_iodescriptors()";

   close(io->src.s);
   close(io->dst.s);
   freefds += 2;

#if SOCKS_SERVER  /* may have control connection also. */
   switch (io->state.command) {
      case SOCKS_CONNECT:
         break;

      case SOCKS_BIND:
      case SOCKS_BINDREPLY:
         if (!io->state.extension.bind)
            break;
         /* else: */ /* FALLTHROUGH */

      case SOCKS_UDPASSOCIATE:
         close(io->control.s);
         ++freefds;
         break;

      default:
         SERRX(io->state.command);
   }
#endif /* !SOCKS_SERVER */
}

int
recv_io(s, io)
   int s;
   struct sockd_io_t *io;
{
   const char *function = "recv_io()";
   struct sockd_io_t tmpio;
   struct iovec iovecv[2];
   struct msghdr msg;
   size_t ioi;
   ssize_t received;
   struct timeval tnow;
   int wearechild, fdexpect, fdreceived, iovecc;
#if HAVE_GSSAPI
   gss_buffer_desc gssapistate;
   char gssapistatemem[MAXGSSAPITOKENLEN];
#endif /* HAVE_GSSAPI */
   CMSG_AALLOC(cmsg, sizeof(int) * FDPASS_MAX);

   bzero(iovecv, sizeof(iovecv));
   iovecc = 0;

   iovecv[iovecc].iov_base = &tmpio;
   iovecv[iovecc].iov_len  = sizeof(tmpio);
   ++iovecc;

#if HAVE_GSSAPI
   iovecv[iovecc].iov_base = gssapistatemem;
   iovecv[iovecc].iov_len  = sizeof(gssapistatemem);
   ++iovecc;
#endif /* HAVE_GSSAPI */

   bzero(&msg, sizeof(msg));
   msg.msg_iov    = iovecv;
   msg.msg_iovlen = iovecc;
   msg.msg_name   = NULL;
   msg.msg_namelen = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));

   if (io == NULL) /* child semantics; find a free io ourselves. */
      wearechild = 1;
   else
      wearechild = 0;

   if ((received = recvmsgn(s, &msg, 0)) < (ssize_t)sizeof(*io)) {
      if (received == 0)
         slog(LOG_DEBUG, "%s: recvmsg(): %s closed connection",
         function, pidismother(sockscf.state.pid) ? "mother" : "child");

      return -1;
   }

   if (socks_msghaserrors(function, &msg))
      return -1;

   /*
    * ok, received a io.  Find out where to store it.
    */
   ioi = 0;
   if (wearechild) { /* child semantics; find a free io ourselves. */
      SASSERTX(io == NULL);

      for (; ioi < ioc; ++ioi)
         if (!iov[ioi].allocated) {
            io            = &iov[ioi];
            break;
         }

      if (io == NULL) {
         /*
          * Only reason this should happen is if mother died/closed connection,
          * and what we are getting now is not actually an i/o, but just an
          * errorindicator.
          */
         char buf;

         if (recv(s, &buf, sizeof(buf), MSG_PEEK) > 0)
            /*
             * not an errror indicator, but a mismatch between us and mother.
             * Should never happen.
             */
            SERRX(io_allocated(NULL, NULL));

         return -1;
      }
   }

   *io = tmpio;
   if (wearechild) {
         if (io->state.command == SOCKS_UDPASSOCIATE) {
         io->cmd.udp.fwdrule   = &fwdrulev[ioi];
         io->cmd.udp.replyrule = &replyrulev[ioi];

         bzero(io->cmd.udp.replyrule, sizeof(*io->cmd.udp.replyrule));
         bzero(io->cmd.udp.fwdrule, sizeof(*io->cmd.udp.fwdrule));
      }
   }

   received -= sizeof(*io);

   /* figure out how many descriptors we are supposed to be passed. */
   switch (io->state.command) {
      case SOCKS_BIND:
      case SOCKS_BINDREPLY:
         if (io->state.extension.bind)
            fdexpect = 3;   /* in, out, control. */
         else
            fdexpect = 2;   /* in and out. */
         break;

      case SOCKS_CONNECT:
         fdexpect = 2;   /* in and out */
         break;

      case SOCKS_UDPASSOCIATE:
#if BAREFOOTD
         fdexpect = 2;   /* in and out. */
#else /* SOCKS_SERVER */
         fdexpect = 3;   /* in, out, control. */
#endif /* SOCKS_SERVER */
         break;

      default:
         SERRX(io->state.command);
   }

   /* verify expected datalen */
   CMSG_VERIFY_RCPTLEN(msg, sizeof(int) * fdexpect);

   /*
    * Get descriptors sent us.  Should be at least two.
    */

   fdreceived = 0;

   CMSG_GETOBJECT(io->src.s, cmsg, sizeof(io->src.s) * fdreceived++);
   CMSG_GETOBJECT(io->dst.s, cmsg, sizeof(io->dst.s) * fdreceived++);

#if HAVE_GSSAPI
   gssapistate.value  = gssapistatemem;
   gssapistate.length = received;

   if (received > 0)
      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: read gssapistate of size %ld",
         function, (unsigned long)received);
#endif /* HAVE_GSSAPI */

   /* any more descriptors to expect? */
   switch (io->state.command) {
      case SOCKS_BIND:
#if HAVE_GSSAPI
         if (io->src.auth.method == AUTHMETHOD_GSSAPI) {
            if (gssapi_import_state(&io->src.auth.mdata.gssapi.state.id,
            &gssapistate) != 0)
               return -1;
         }
#endif /* HAVE_GSSAPI */

         if (io->state.extension.bind)
            /* LINTED pointer casts may be troublesome */
            CMSG_GETOBJECT(io->control.s, cmsg,
            sizeof(io->control.s) * fdreceived++);
         else
            io->control.s = -1;
         break;

      case SOCKS_BINDREPLY:
#if HAVE_GSSAPI
         if (io->dst.auth.method == AUTHMETHOD_GSSAPI) {
            if (gssapi_import_state(&io->dst.auth.mdata.gssapi.state.id,
            &gssapistate) != 0)
               return -1;
         }
#endif /* HAVE_GSSAPI */

         if (io->state.extension.bind)
            /* LINTED pointer casts may be troublesome */
            CMSG_GETOBJECT(io->control.s, cmsg,
            sizeof(io->control.s) * fdreceived++);
         else
            io->control.s = -1;
         break;

      case SOCKS_CONNECT:
#if HAVE_GSSAPI
         if (io->src.auth.method == AUTHMETHOD_GSSAPI) {
            if (gssapi_import_state(&io->src.auth.mdata.gssapi.state.id,
            &gssapistate) != 0)
               return -1;
         }
#endif /* HAVE_GSSAPI */

         io->control.s = -1;
         break;

      case SOCKS_UDPASSOCIATE:
#if SOCKS_SERVER
         /* LINTED pointer casts may be troublesome */
         CMSG_GETOBJECT(io->control.s, cmsg,
         sizeof(io->control.s) * fdreceived++);

#if HAVE_GSSAPI
         if (io->src.auth.method == AUTHMETHOD_GSSAPI) {
            if (gssapi_import_state(&io->src.auth.mdata.gssapi.state.id,
            &gssapistate) != 0)
               return -1;
         }
#endif /* HAVE_GSSAPI */
#else /* !SOCKS_SERVER */

         io->control.s = -1;
#endif /* !SOCKS_SERVER */

         break;

      default:
         SERRX(io->state.command);
   }

   if (wearechild) { /* only child does i/o and needs a buffer. */
      gettimeofday(&tnow, NULL);

      /* needs to be set now for correct bandwidth calculation/limiting. */
      io->iotime = tnow;

#if BAREFOOTD
      if (io->state.command == SOCKS_UDPASSOCIATE) {
         /*
          * prepare things for udp client
          */

         if ((io->dstv = malloc(UDP_MEMBLOCK * sizeof(*io->dstv))) == NULL) {
            swarn("%s: failed to allocate memory for udp clients", function);
            close(io->src.s);
            close(io->dst.s);

            return 0; /* strange, but not fatal. */
         }

         io->dstc    = 0;
         io->dstcmax = UDP_MEMBLOCK;

         /*
          * dummy-socket.  Need to put in something valid so
          * select(2) has something to select for write on, but it
          * is never actually used for any i/o, as each client will
          * require it's own outgoing socket, with it's own local address
          * so that we know what destinations are failing.
          *
          * The dst, as used for rulespermit(), is however the address
          * we receive the udp packet on (src.laddr), and that remains the
          * same for all our udp clients of course.
          */
         bzero(&io->dstv[io->dstc], sizeof(io->dstv[io->dstc]));

         io->state.time.established         = tnow;

         io->dstv[io->dstc].laddr.sa_family = AF_INET;
         io->dstv[io->dstc].raddr           = io->dstv[io->dstc].laddr;
         io->dstv[io->dstc].client          = io->dstv[io->dstc].laddr;

         io->dstv[io->dstc].s               = io->dst.s;
         ++io->dstc;
      }
#endif /* BAREFOOTD */

      if (io->control.s != -1)
         socks_allocbuffer(io->control.s, SOCK_STREAM);

      socks_allocbuffer(io->src.s,
                        io->state.command
                        == SOCKS_UDPASSOCIATE ? SOCK_DGRAM : SOCK_STREAM);

#if HAVE_NEGOTIATE_PHASE
      if (io->clientdatalen != 0) {
         slog(LOG_DEBUG, "%s: adding initial data of size %ld "
                         "from client %s to iobuf",
                         function,
                         (long)io->clientdatalen,
                         sockshost2string(&io->src.host, NULL, 0));

         /*
          * XXX if covenant, this request has already been parsed and we
          * already know we need to forward it; should optimize away
          * re-parsing.
          */

         socks_addtobuffer(io->src.s,
                           READ_BUF,
                           0,
                           io->clientdata,
                           io->clientdatalen);

         io->clientdatalen = 0;
      }

#if COVENANT
      io->src.state.isclientside = 1;
#endif /* COVENANT */

#endif /* HAVE_NEGOTIATE_PHASE */

      socks_allocbuffer(io->dst.s,
                        io->state.command
                        == SOCKS_UDPASSOCIATE ? SOCK_DGRAM : SOCK_STREAM);
   }

   if (sockscf.option.debug >= DEBUG_VERBOSE) {
      slog(LOG_DEBUG, "%s: received %d descriptor(s) for command %d.  "
                      "Control: %d, src: %d, dst: %d.  Allocated to iov #%lu",
                      function, fdreceived, io->state.command,
                      io->control.s, io->src.s, io->dst.s, (unsigned long)ioi);

      slog(LOG_DEBUG, "%s: received src socket %d (%s) ...",
      function, io->src.s, socket2string(io->src.s, NULL, 0));

      slog(LOG_DEBUG, "%s: received dst socket %d (%s) ...",
      function, io->dst.s, socket2string(io->dst.s, NULL, 0));

      if (io->control.s != -1) {
         slog(LOG_DEBUG, "%s: received control socket %d (%s) ...",
         function, io->control.s, socket2string(io->control.s, NULL, 0));
      }
   }

   if (io->crule.bw_shmid != 0 || io->crule.ss_shmid != 0)
      slog(LOG_DEBUG, "%s: client-rule: bw_shmid: %ld, ss_shmid: %ld",
                      function, io->crule.bw_shmid, io->crule.ss_shmid);


   if (io->rule.bw_shmid != 0 || io->rule.ss_shmid != 0)
      slog(LOG_DEBUG, "%s: socks-rule: bw_shmid: %ld, ss_shmid: %ld",
                      function, io->rule.bw_shmid, io->rule.ss_shmid);

   if (sockscf.option.debug >= DEBUG_VERBOSE) {
      sockd_shmat(&io->rule, SHMEM_ALL);
      sockd_shmat(&io->crule, SHMEM_ALL);

      if (io->crule.bw_shmid != 0)
         slog(LOG_DEBUG, "%s: client-rule: "
                         "bw object at location %p, ts %ld.%06ld, "
                         "clients: %ld, rulenumber: %ld, isclientrule: %d",
                         function, &io->crule.bw,
                         (long)io->crule.bw->mstate.allocatedts.tv_sec,
                         (long)io->crule.bw->mstate.allocatedts.tv_usec,
                         (long)io->crule.bw->mstate.clients,
                         (unsigned long)io->crule.bw->mstate.rulenumber,
                         io->crule.bw->mstate.isclientrule);

      if (io->rule.bw_shmid != 0)
         slog(LOG_DEBUG, "%s: socks-rule: "
                         "bw object at location %p, ts %ld.%06ld, "
                         "clients: %ld, rulenumber: %ld, isclientrule: %d",
                         function, &io->rule.bw,
                         (long)io->rule.bw->mstate.allocatedts.tv_sec,
                         (long)io->rule.bw->mstate.allocatedts.tv_usec,
                         (long)io->rule.bw->mstate.clients,
                         (unsigned long)io->rule.bw->mstate.rulenumber,
                         io->rule.bw->mstate.isclientrule);

      if (io->crule.ss_shmid != 0)
         slog(LOG_DEBUG, "%s: client-rule: "
                         "ss object at location %p, ts %ld.%06ld, "
                         "clients: %ld, rulenumber: %ld, isclientrule: %d",
                         function, &io->crule.ss,
                         (long)io->crule.ss->mstate.allocatedts.tv_sec,
                         (long)io->crule.ss->mstate.allocatedts.tv_usec,
                         (long)io->crule.ss->mstate.clients,
                         (unsigned long)io->crule.ss->mstate.rulenumber,
                         io->crule.ss->mstate.isclientrule);

      if (io->rule.ss_shmid != 0)
         slog(LOG_DEBUG, "%s: socks-rule: "
                         "ss object at location %p, ts %ld.%06ld, "
                         "clients: %ld, rulenumber: %ld, isclientrule: %d",
                         function, &io->rule.ss,
                         (long)io->rule.ss->mstate.allocatedts.tv_sec,
                         (long)io->rule.ss->mstate.allocatedts.tv_usec,
                         (long)io->rule.ss->mstate.clients,
                         (unsigned long)io->rule.ss->mstate.rulenumber,
                         io->rule.ss->mstate.isclientrule);

      sockd_shmdt(&io->crule, SHMEM_ALL);
      sockd_shmdt(&io->rule, SHMEM_ALL);
   }


#if BAREFOOTD
   /*
    * in barefootd, only the resources for the tcp-rule are fixed at this
    * point.
    * For udp we need to wait til we get the clients before we know what
    * rules to use.
    */
   if (io->state.protocol == SOCKS_TCP)
#endif
      /* attach now, so we don't have to attach/detach for every i/o op. */
      sockd_shmat(&io->rule, SHMEM_ALL);

   freefds -= fdreceived;

   io->allocated = 1;
   return 0;
}

static void
io_clearset(io, set)
   const struct sockd_io_t *io;
   fd_set *set;
{

   SASSERTX(io->src.s != -1 && io->dst.s != -1);
   FD_CLR(io->src.s, set);
   FD_CLR(io->dst.s, set);

   switch (io->state.command) {
      case SOCKS_CONNECT:
         break;

      case SOCKS_BIND:
      case SOCKS_BINDREPLY:
         if (!io->state.extension.bind)
            break;
         /* else: */ /* FALLTHROUGH */

      case SOCKS_UDPASSOCIATE:
#if SOCKS_SERVER
         SASSERTX(io->control.s != -1);
         FD_CLR(io->control.s, set);
#endif /* SOCKS_SERVER */
         break;

      default:
         SERRX(io->state.command);
   }
}

static size_t
io_allocated(tcp_alloc, udp_alloc)
   int *tcp_alloc;
   int *udp_alloc;
{
   const char *function = "io_allocated()";
   size_t i;
   int tcp_alloc_mem, udp_alloc_mem;

   if (tcp_alloc == NULL)
      tcp_alloc = &tcp_alloc_mem;

   if (udp_alloc == NULL)
      udp_alloc = &udp_alloc_mem;

   *udp_alloc = *tcp_alloc = 0;
   for (i = 0; i < ioc; ++i) {
      if (!iov[i].allocated)
         continue;

      switch (iov[i].state.protocol) {
         case SOCKS_UDP:
            ++(*udp_alloc);
            break;

         case SOCKS_TCP:
            ++(*tcp_alloc);
            break;

         default:
            SERRX(iov[i].state.protocol);
      }

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: iov #%lu allocated for %s",
         function, (unsigned long)i, protocol2string(iov[i].state.protocol));
   }

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: allocated for tcp: %d, udp: %d",
      function, *tcp_alloc, *udp_alloc);

   return *tcp_alloc + *udp_alloc;
}


/*
 * XXX this function has grown far too big and must be split up.
 */
static iostatus_t
doio(io, rset, wset, flags, bad)
   struct sockd_io_t *io;
   fd_set *rset, *wset;
   const int flags;
   int *bad;
{
   const char *function = "doio()";
   ssize_t r;
   size_t bwused;
#if COVENANT
   char *buf      = io->clientdata;
   size_t buflen  = sizeof(io->clientdata) - io->clientdatalen;
   size_t bufused = io->clientdatalen;
#else
   /*
    * Static so that it gets allocated on the heap rather than the
    * stack, as the latter seems to cause some problems on Solaris
    * where under stress Solaris is unable to grow the stack by 
    * that much.
    */
   static char buf[SOCKD_BUFSIZE];
   size_t buflen  = sizeof(buf);
   size_t bufused = 0;
#endif /* !COVENANT */
   int srchaswritebuf, dsthaswritebuf;

   slog(LOG_DEBUG, "%s: control: %d, src: %d, dst: %d",
   function, io->control.s, io->src.s, io->dst.s);

   SASSERTX(io->allocated);

   if (io->state.command == SOCKS_UDPASSOCIATE)
      SASSERTX(io->cmd.udp.replyrule != NULL);

   if (io->state.command == SOCKS_CONNECT)
      SASSERTX(io->dst.state.connected);

   errno = 0; /* reset on each call. */

   /*
    * XXX we now check for writebuffer in any case, further down, 
    * so remove the if-check here?
    * Also, why do we need to check here?  Isn't this checked 
    * by caller, and then the fd's are set as writable in rset?
    */
   if (FD_ISSET(io->src.s, rset))
      dsthaswritebuf = 0; /* or really, "don't care". */
   else {
      if (socks_bufferhasbytes(io->dst.s, WRITE_BUF))
         dsthaswritebuf = 1;
      else
         dsthaswritebuf = 0;
   }

   if (FD_ISSET(io->dst.s, rset))
      srchaswritebuf = 0; /* or really, "don't care". */
   else {
      if (socks_bufferhasbytes(io->src.s, WRITE_BUF))
         srchaswritebuf = 1;
      else
         srchaswritebuf = 0;
   }

#if BAREFOOTD
   if (io->state.command == SOCKS_UDPASSOCIATE)
      SASSERTX((FD_ISSET(io->src.s, rset) || dsthaswritebuf)
      ||       (FD_ISSET(io->dst.s, rset) || srchaswritebuf));
   else
#endif /* BAREFOOTD */

   if (((FD_ISSET(io->src.s, rset) || dsthaswritebuf)
     && FD_ISSET(io->dst.s, wset))
   ||       ((FD_ISSET(io->dst.s, rset) || srchaswritebuf)
     && FD_ISSET(io->src.s, wset))
   ||       (io->control.s != -1 && FD_ISSET(io->control.s, rset)))
      ;
   else {
      swarnx("%s: FD_ISSET(io->src.s, rset) = %d, dsthaswritebuf = %d "
                 "FD_ISSET(io->dst.s, wset) = %d "
                 "FD_ISSET(io->dst.s, rset) = %d, srchaswritebuf = %d "
                 "FD_ISSET(io->src.s, wset) = %d, "
                 "io->control.s = %d, FD_ISSET(io->control.s, rset) = %d",
                 function,
                 FD_ISSET(io->src.s, rset), dsthaswritebuf,
                 FD_ISSET(io->dst.s, wset),
                 FD_ISSET(io->dst.s, rset), srchaswritebuf,
                 FD_ISSET(io->src.s, wset),
                 io->control.s,
                 io->control.s == -1 ? 0 : FD_ISSET(io->control.s, rset));

      SASSERTX(0);

      /* should only get here if asserts are disabled. */
      *bad = io->dst.s;
      return IO_ERROR;
   }

   if (io->state.time.firstio.tv_sec  == 0
   &&  io->state.time.firstio.tv_usec == 0)
      gettimeofday(&io->state.time.firstio, NULL);

   bwused = 0;
   switch (io->state.protocol) {
      case SOCKS_TCP: {
         if (io->rule.bw_shmid != 0) {
            /*
             * If all clients are active, this should distribute the bw
             * reasonably fair.  If not, this is suboptimal as we may
             * need to do more i/o operations than otherwise necessary,
             * as our buflen is smaller than need be.
             */
            buflen = MAX(1, buflen / io->rule.bw->mstate.clients);

            slog(LOG_DEBUG, "%s: buflen is %lu",
            function, (unsigned long)buflen);
         }

         /* from in to out ... */
         if ((FD_ISSET(io->src.s, rset) || dsthaswritebuf)
         && FD_ISSET(io->dst.s, wset)) {
            int allflushed;

            /*
             * If we have previously tried to write, but could not write it
             * all, we will have data buffered for the socket. In that case
             * we need to flush the buffer before writing anything else. 
             * Since that data has already been logged as written (even if
             * only to buffer), don't log it again.
             */
            if (socks_flushbuffer(io->dst.s, -1) == -1) {
               if (!ERRNOISTMP(errno)) {
                  *bad = io->dst.s;
                  return IO_ERROR;
               }

               allflushed = 0;
            }
            else
               allflushed = 1;

            if (allflushed) {
               r = io_rw(&io->src,
                         &io->dst,
                         bad,
                         &io->reqflags,
                         buf,
                         buflen,
                         &bufused,
                         flags);

               if (*bad != -1)
                  return r == 0 ? IO_CLOSE : IO_ERROR;

               switch (r) {
                  case -1:
                     r = 0; /* bad is not set, so temporary error. */
                     break;

                  case 0:
                     break;

                  default:
                     iolog(&io->rule,
                           &io->state,
                           OPERATION_IO,
                           &io->src.laddr,
                           &io->src.host,
                           &io->src.auth,
                           NULL,
                           NULL,
                           NULL,
                           &io->dst.laddr,
                           &io->dst.host,
                           &io->dst.auth,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                           NULL,
                           buf,
                           r);
               }

               bwused         += r;
               dsthaswritebuf  = 0;
            }
         }

         /* ... and out to in. */
         if ((FD_ISSET(io->dst.s, rset) || srchaswritebuf)
         && FD_ISSET(io->src.s, wset)) {
            int allflushed;

            /*
             * If we have previously tried to write, but could not write it
             * all, we will have data buffered for the socket. In that case
             * we need to flush the buffer before writing anything else. 
             * Since that data has already been logged as written (even if
             * only to buffer), don't log it again.
             */
            if (socks_flushbuffer(io->src.s, -1) == -1) {
               if (!ERRNOISTMP(errno)) {
                  *bad = io->src.s;
                  return IO_ERROR;
               }

               allflushed = 0;
            }
            else
               allflushed = 1;

            if (allflushed) {
               r = io_rw(&io->dst,
                         &io->src,
                         bad,
                         &io->reqflags,
                         buf,
                         buflen,
                         &bufused,
                         flags);

               if (*bad != -1)
                  return r == 0 ? IO_CLOSE : IO_ERROR;

               switch (r) {
                  case -1:
                     r = 0; /* bad is not set, so temporary error. */
                     break;

                  case 0:
                     break;

                  default:
                     iolog(&io->rule,
                           &io->state,
                           OPERATION_IO,
                           &io->dst.laddr,
                           &io->dst.host,
                           &io->dst.auth,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                           NULL,
                           &io->src.laddr,
                           &io->src.host,
                           &io->src.auth,
                           NULL,
                           NULL,
                           NULL,
                           buf,
                           (size_t)r);
               }

               bwused         += r;
               srchaswritebuf  = 0;
            }
         }

         gettimeofday(&io->iotime, NULL);
         io_update(&io->iotime, bwused, &io->rule, sockscf.shmemfd);
         break;
      }

#if HAVE_UDP_SUPPORT
      case SOCKS_UDP: {
         /*
          * In the socks case the client-side of udp i/o is always fixed.
          * In barefoot the client-side can/will vary for each packet,
          * as one socket can receive packets from multiple clients.
          *
          * The destination side can vary for each packet in both Dante
          * Barefoot.  In Dante, because the client wants to send to a
          * different address from last time.  In Barefoot it can normally
          * not vary, since the bounce-to address is set in the rule and
          * fixed there.  A sighup can change the bounce-to address however,
          * so we must be prepared to handle that.
          *
          * This makes udp i/o considerably more complicated compared
          * to tcp, as we may need to check rules on each packet if the
          * destination changes from one packet to the next, or the source
          * also in Barefoots case.
          *
          * Also note that we are less strict about bandwidth in the udp
          * case since we can't truncate packets.  Therefore we don't limit
          * the amount of i/o we do in one go for the udp-case; it has to be
          * whole packets.
          * (Now that we have the iobuffer mechanism, we could in theory check
          * if we have enough bandwidth allocated to send the packet, and
          * if not, buffer it for later, but don't bother for now.)
          */
#if BAREFOOTD
         struct udpclient *udpclient;
         struct connectionstate_t clientrulestate;
#endif /* BAREFOOTD */
         struct sockshost_t host;
         char hosta[MAXSOCKSHOSTSTRING], hostb[MAXSOCKSHOSTSTRING];
         struct udpheader_t header;
         socklen_t len;
         ssize_t w;
         int permit;
#if SOCKS_SERVER
         /*
          * In both Barefoot and Dante, we need to do both a rulespermit()
          * per packet, but we also need to save the original rule that
          * allowed the session.
          *
          * In Dante, the original rule is the socks-rule that matched the
          * control-connection, while in Barefoot, it is client-rule that
          * was used to generate the corresponding socks-rule.
          *
          * In Barefoot's case, we always have the client-rule available in
          * io.crule, following a rulespermit() lookup.
          * In Dante's case however, it is only available in io.rule, so
          * we can not pass io.rule to rulespermit() every time, as we would
          * loose the original rule.
          * In Dante's case we therefore need to let packetrule, i.e. the rule
          * the current packet matched, be stored in some other memory
          * than io.rule, while in Barefoot's case, we want to use io.rule for
          * this.
          */
#endif /* SOCKS_SERVER */

         /*
          * UDP to relay from client to destination?
          */
         if ((FD_ISSET(io->src.s, rset) || dsthaswritebuf)
         &&  FD_ISSET(io->dst.s, wset)) {
            const int lflags = flags & ~MSG_OOB;
            struct sockaddr from;
            int failed = 0;
#if SOCKS_SERVER
            struct rule_t *packetrule = io->cmd.udp.fwdrule;

            if (!io->use_saved_rule)
               *packetrule = io->rule; /* default, until rulespermit(). */

#else /* BAREFOOTD */
            struct rule_t *packetrule = &io->rule;
            struct udpclient udpsrc;

            bzero(&udpsrc, sizeof(udpsrc));

            /* for any logging done before rulespermit(). */
            *packetrule = io->crule;
#endif /* BAREFOOTD */

            len = sizeof(from);
            if ((r = socks_recvfrom(io->src.s,
                                    buf,
                                    buflen,
                                    lflags,
                                    &from,
                                    &len,
                                    &io->src.auth)) == -1) {
#if BAREFOOTD
               /*
                * Barefoot can have many udp clients and we can't know
                * here which one this error is for.  Hopefully it will be
                * picked up via the raw socket also if we have one,
                * and logged correctly there.
                */
               io->src.host.atype            = (unsigned char)SOCKS_ADDR_IPV4;
               io->src.host.addr.ipv4.s_addr = htonl(INADDR_ANY);
               io->src.host.port             = htons(0);
#endif /* BAREFOOTD */

               if (ERRNOISTMP(errno) || errno == ECONNREFUSED) {
                  if (errno == ECONNREFUSED) {
                     /*
                      * error is from a previous packet sent by us to
                      * this client.
                      */

#if SOCKS_SERVER
                     /*
                      * Don't treat it as fatal, more packets could come
                      * and they may be accepted by the client.  As long
                      * as we still have the control connection, assume
                      * the client is alive.
                      */
#elif BAREFOOTD /* !SOCKS_SERVER */
                     if (rawsocket == -1) {
                        /*
                         * no raw socket, but log at least something.
                         * No sense in returning a permanent error for this
                         * as we have no idea which client it is.  Will have
                         * to wait til the session times out the regular way.
                         */
#endif /* BAREFOOTD */

                     iolog(packetrule,
                           &io->state,
                           OPERATION_ERROR_IO,
                           &io->dst.laddr,
                           &io->dst.host,
                           &io->dst.auth,
                           io->state.proxyprotocol == PROXY_DIRECT ?
                              NULL : &io->state.proxychain.server,
                           io->state.proxyprotocol == PROXY_DIRECT ?
                              NULL : &io->state.proxychain.extaddr,
                           NULL,
                           &io->src.laddr,
                           &io->src.host,
                           &io->src.auth,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           0);
#if BAREFOOTD
                     }
#endif /* BAREFOOTD */

                     return IO_TMPERROR;
                  }
                  else
                     return IO_EAGAIN;
               }

               /* else: unknown error, assume fatal. */
               *bad = io->src.s;
               return IO_ERROR;
            }

#if SOCKS_SERVER
            if (!ADDRISBOUND(TOIN(&io->src.raddr))
            ||  !PORTISBOUND(TOIN(&io->src.raddr))) {
               /*
                * Client hasn't sent us it's complete address yet, but if
                * the parts of the address it has sent, if any, matches
                * the source of this packet, we have to assume this packet
                * is from it.  We then connect the socket to the client, for
                * better performance, for receiving errors from sendto(),
                * for getpeername() libwrap in rulespermit()), for ...
                * well, that's reasons enough.
                */

               if (!ADDRISBOUND(TOIN(&io->src.raddr)))
                  TOIN(&io->src.raddr)->sin_addr.s_addr
                  = TOIN(&from)->sin_addr.s_addr;

               if (!PORTISBOUND(TOIN(&io->src.raddr)))
                  TOIN(&io->src.raddr)->sin_port = TOIN(&from)->sin_port;

               if (!sockaddrareeq(&io->src.raddr, &from)) {
                  char src[MAXSOCKADDRSTRING], dst[MAXSOCKADDRSTRING];

                  r = snprintf(buf, sizeof(buf),
                              "expected udp packet from %s, but got it from %s",
                              sockaddr2string(&io->src.raddr, src, sizeof(src)),
                              sockaddr2string(&from, dst, sizeof(dst)));

                  failed = 1;
               }

               if (!failed) {
                  sockaddr2sockshost(&io->src.raddr, &io->src.host);
                  if (connect(io->src.s, &io->src.raddr, sizeof(io->src.raddr))
                  != 0) {
                     r = snprintf(buf, sizeof(buf),
                                 "%s: strange ... could not connect(2) back to "
                                 "udp client who sent us a packet from %s",
                                 function,
                                 sockaddr2string(&io->src.raddr, NULL, 0));

                     swarn("%s", buf);
                     failed = 1;
                  }
               }

               if (failed) {
                  iolog(packetrule,
                        &io->state,
                        OPERATION_ERROR_IO,
                        &io->src.laddr,
                        &io->src.host,
                        &io->src.auth,
                        NULL,
                        NULL,
                        NULL,
                        &io->dst.laddr,
                        &io->dst.host,
                        &io->dst.auth,
                        io->state.proxyprotocol
                          == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                        io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                        NULL,
                        buf,
                        r);

                  return IO_TMPERROR;
               }
            }

            io->src.read.bytes   += r;
            io->src.read.packets += 1;
#endif /* SOCKS_SERVER */

#if BAREFOOTD
            /*
             * no socks header.  Dst is the fixed bounce-to address
             * in the matching client-rule, so need to find the matching
             * client-rule before we know what dst will be.
             * That rule also contains the resource limits to use, if any.
             */

            udpclient = udpclientofclientaddr(&from, io->dstc, io->dstv);
            if (udpclient != 0)
               io->dst.s = udpclient->s;
            else
               io->dst.s = -1;

            slog(LOG_DEBUG,
                 "%s: %sfirst udp packet from %s received on %s, length %ld",
                 function,
                 io->dst.s == -1 ? "" : "not ",
                 sockaddr2string(&from, hosta, sizeof(hosta)),
                 sockaddr2string(&io->src.laddr, hostb, sizeof(hostb)),
                 (long)r);

            if (io->dst.s == -1 || !udpclient->use_saved_rule) {
               io->src.raddr = from;
               sockaddr2sockshost(&io->src.raddr, &io->src.host);
               sockaddr2sockshost(&io->src.laddr, &io->dst.host);

               clientrulestate         = io->state;
               clientrulestate.command = SOCKS_BOUNCETO;

               bzero(&io->clientauth, sizeof(io->clientauth));
               bzero(&io->src.auth, sizeof(io->src.auth));

               io->clientauth.method   = AUTHMETHOD_NOTSET;
               io->src.auth.method     = AUTHMETHOD_NOTSET;

               permit = rulespermit(io->src.s,
                                    &io->src.raddr,
                                    &io->src.laddr,
                                    &io->clientauth,
                                    &io->src.auth,
                                    &io->crule,
                                    &clientrulestate,
                                    &io->src.host,
                                    &io->dst.host,
                                    NULL,
                                    0);

               /*
                * for now, til we've done rulespermit() and got the right
                * one.
                */
               io->rule       = io->crule;
               io->rule.crule = &io->crule;
            }
            else { /* reuse rules from last time. */
               SASSERTX(udpclient != NULL);

               io->crule      = udpclient->crule;
               io->rule.crule = &io->crule;
               io->rule       = udpclient->rule;

               permit         = io->crule.verdict == VERDICT_PASS;
            }

            bzero(&header, sizeof(header));
            ruleaddr2sockshost(&io->crule.bounce_to, &header.host, SOCKS_UDP);

            slog(LOG_DEBUG,
                 "%s: bounce-to address for client %s is %s",
                 function,
                 sockshost2string(&io->src.host, hosta, sizeof(hosta)),
                 sockshost2string(&header.host, hostb, sizeof(hostb)));

            io->dst.host = header.host;

            if (!permit) {
               iolog(&io->crule,
                     &io->state,
                     OPERATION_BLOCK_IO,
                     &io->src.laddr,
                     &io->src.host,
                     &io->src.auth,
                     NULL,
                     NULL,
                     NULL,
                     &io->dst.laddr,
                     &io->dst.host,
                     &io->dst.auth,
                     io->state.proxyprotocol
                        == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                     io->state.proxyprotocol
                        == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                     NULL,
                     buf,
                     (size_t)r);

               rawsocket_send(rawsocket,
                              &io->src.host,
                              sockaddr2sockshost(&io->src.laddr, &host),
                              &io->dst.host,
                              -1);

               return IO_BLOCK;
            }

#else /* SOCKS_SERVER */

            /* got a packet, pull out the socks UDP header. */
            if (string2udpheader(buf, (size_t)r, &header) == NULL) {
               r = snprintf(buf, sizeof(buf),
                           "unsupported udp packet receved (length %u)",
                           (unsigned)r);

               failed = 1;
            }

            if (!failed && header.frag != 0) {
               r = snprintf(buf, sizeof(buf),
                            "fragmented udp packets are not supported");
               failed = 1;
            }
#endif /* SOCKS_SERVER */

            if (!failed) {
               sockshost2sockaddr(&header.host, &io->dst.raddr);
               if (!ADDRISBOUND(TOIN(&io->dst.raddr))) {
                  r = snprintf(buf, sizeof(buf),
                               "can not resolve destination %s for client %s",
                               sockshost2string(&header.host, NULL, 0),
                               sockaddr2string(&from, NULL, 0));

                  failed = 1;
               }
            }

            if (failed) {
               iolog(packetrule,
                     &io->state,
                     OPERATION_ERROR_IO,
                     &io->src.laddr,
                     &io->src.host,
                     &io->src.auth,
                     NULL,
                     NULL,
                     NULL,
                     &io->dst.laddr,
                     header.host.atype == SOCKS_ADDR_NOTSET
                        ? NULL : &header.host,
                     &io->dst.auth,
                     io->state.proxyprotocol == PROXY_DIRECT ?
                        NULL : &io->state.proxychain.server,
                     io->state.proxyprotocol == PROXY_DIRECT ?
                        NULL : &io->state.proxychain.extaddr,
                     NULL,
                     buf,
                     r);

               if (header.host.atype != SOCKS_ADDR_NOTSET)
                  rawsocket_send(rawsocket,
                                 &io->src.host,
                                 sockaddr2sockshost(&io->src.laddr, &host),
                                 &header.host,
                                 -1);

               return IO_TMPERROR;
            }

            /*
             * An optimization: if the client will only be sending udp 
             * packets to one address, it is much more efficient to connect 
             * the socket to that address. 
             *
             * This is always the case in barefootd, and may also be the 
             * usual case with socks.
             *
             * However, if we do that, we must be sure to unconnect
             * the socket before sending out on it again if the client wants
             * to send to a new address, and from that point on, leave the
             * socket unconnected, so that possible future packets from the
             * address we first connected to will also be received.
             */


#if BAREFOOTD
            if (io->dst.s == -1 || !udpclient->use_saved_rule)
#else /* SOCKS_SERVER */
            if (!io->dst.state.connected || !io->use_saved_rule)
#endif /* SOCKS_SERVER */
            {
               slog(LOG_DEBUG,
                    "%s: udp packet #%"PRIu64" from client %s.  Destination %s",
                    function,
                    io->src.read.packets,
                    sockaddr2string(&from, NULL, 0),
                    sockshost2string(&header.host, NULL, 0));

#if BAREFOOTD
               /*
                * Create a new socket and use that for sending out packets
                * from this client only.  When reading replies on this socket,
                * we will thus know who it's destined for (from).
                *
                * Since we place no bound on the number of udp clients we
                * handle, we need to make sure we leave room for at least
                * SOCKD_IOMAX tcp clients, so we don't fail on recvmsg(2)
                * when mother sends us a new tcp client.
                */
               if (io->dst.s == -1) {
                  udpclient = NULL;

                  if (freefds <= ((SOCKD_IOMAX - 1) * FDPASS_MAX)
                  || (io->dst.s = udpsrc.s = socket(AF_INET, SOCK_DGRAM, 0))
                  == -1){
                     r = snprintf(buf, sizeof(buf),
                                  "could not create udp socket (%s)",
                                  strerror(errno));

                     swarnx("%s: %s", function, buf);
                     failed = 1;
                  }

                  if (!failed) {
                     --freefds;
                     slog(LOG_DEBUG, "%s: free fds: %d", function, freefds);

                     setsockoptions(udpsrc.s, SOCK_DGRAM, 0);

                     bzero(&udpsrc.laddr, sizeof(udpsrc.laddr));
                     TOIN(&udpsrc.laddr)->sin_family = AF_INET;
                     TOIN(&udpsrc.laddr)->sin_port   = htons(0);
                     TOIN(&udpsrc.laddr)->sin_addr
                     = getoutaddr(TOIN(&from)->sin_addr, header.host.addr.ipv4);

                     if (sockd_bind(udpsrc.s, &udpsrc.laddr, 0) != 0) {
                        r = snprintf(buf, sizeof(buf),
                                     "could not bind udp address %s (%s)",
                                      sockaddr2string(&udpsrc.laddr,
                                                      NULL,
                                                      0),
                                         strerror(errno));

                        swarnx("%s: %s", function, buf);
                        failed = 1;
                     }
                  }

                  if (!failed) {
                     len = sizeof(udpsrc.laddr);
                     if (getsockname(udpsrc.s, &udpsrc.laddr, &len) != 0) {
                        r = snprintf(buf, sizeof(buf),
                                     "getsockname() on udpsrc.s failed: %s",
                                     strerror(errno));
                        swarnx("%s: %s", function, buf);
                        failed = 1;
                     }
                     else {
                        char laddr[MAXSOCKADDRSTRING], caddr[MAXSOCKADDRSTRING];

                        slog(LOG_DEBUG,
                             "%s: address bound on udp socket %d, for sending "
                             "packets on behalf of client at %s, is %s",
                             function,
                             udpsrc.s,
                             sockaddr2string(&from, caddr, sizeof(caddr)),
                             sockaddr2string(&udpsrc.laddr,
                                             laddr,
                                             sizeof(laddr)));
                     }
                  }

                  if (!failed) {
                     gettimeofday(&udpsrc.iotime, NULL);

                     udpsrc.client  = from;
                     udpsrc.raddr   = io->dst.raddr;
                     udpsrc.firstio = udpsrc.iotime;

                     if ((udpclient = addudpclient(&udpsrc,
                                                   &io->dstc,
                                                   &io->dstcmax,
                                                   &io->dstv)) == NULL) {
                        r = snprintf(buf, sizeof(buf),
                                     "could not add udpclient %s (%s)",
                                      sockaddr2string(&udpsrc.laddr,
                                                      NULL,
                                                      0),
                                      strerror(errno));

                        swarnx("%s: %s", function, buf);
                        failed = 1;
                     }
                  }

                  if (failed) {
                     iolog(packetrule,
                           &io->state,
                           OPERATION_ERROR_IO,
                           &io->src.laddr,
                           &io->src.host,
                           &io->src.auth,
                           NULL,
                           NULL,
                           NULL,
                           &io->dst.laddr,
                           &header.host,
                           &io->dst.auth,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                           NULL,
                           buf,
                           r);

                     if (udpsrc.s != -1) {
                        if (udpclient != NULL)
                           removeudpclient(udpclient->s, &io->dstc, iov->dstv);
                        else 
                           close(udpsrc.s);

                        ++freefds;
                     }

                     slog(LOG_DEBUG, "%s: free fds: %d", function, freefds);
                     return IO_TMPERROR;
                  }

                  SYNC_UDPDST(&io->dst, udpclient);
               }
#endif /* BAREFOOTD */

               /*
                * Now check if the rules allow this packet from the
                * client through.
                */

               permit = rulespermit(
#if SOCKS_SERVER
                                    io->control.s,
                                    &io->control.raddr,
                                    &io->control.laddr,
#else /* BAREFOOTD */
                                    io->src.s,
                                    &io->src.raddr,
                                    &io->src.laddr,
#endif /* BAREFOOTD */
                                    &io->clientauth,
                                    &io->src.auth,
                                    packetrule,
                                    &io->state,
                                    &io->src.host,
                                    &header.host,
                                    NULL,
                                    0);

#if BAREFOOTD
               SASSERTX(udpclient != NULL);
               udpclient->rule           = *packetrule;
               udpclient->crule          = io->crule;
               udpclient->use_saved_rule = 1;

               /* use redirected addresses, if applicable. */
               if (permit) {
                  if (redirect(io->dst.s,
                           &io->dst.laddr,
                           &header.host,
                           io->state.command,
                           &packetrule->rdr_from,
                           &packetrule->rdr_to) != 0) {
                     r = snprintf(buf, sizeof(buf), "redirect failed (%s)",
                                  strerror(errno));
                     failed = 1;
                  }

                  if (!failed) {
                     sockd_shmat(&io->crule, SHMEM_ALL);
                     if (io->crule.ss_shmid != 0) {
                        if (!session_use(io->crule.ss, sockscf.shmemfd)) {
                           permit            = 0;
                           io->crule.verdict = VERDICT_BLOCK;

                           r = snprintf(buf, buflen, DENY_SESSIONLIMITs);

                           sockd_shmdt(&io->crule, SHMEM_SS);

                        }
                        /*
                         * else; remain attached, so that if mother exits and
                         * deletes the shmem file, we still have access.
                         */
                     }

                     if (permit) {
                        if (io->crule.bw_shmid != 0)
                           bw_use(io->crule.bw, sockscf.shmemfd);
                     }

                     udpclient->crule      = io->crule;
                     udpclient->rule       = io->rule;
                     udpclient->rule.crule = &io->crule;
                  }
               }
#endif /* BAREFOOTD */

               if (failed || !permit) {
                  iolog(packetrule,
                        &io->state,
                        failed ? OPERATION_ERROR_IO : OPERATION_BLOCK_IO,
                        &io->src.laddr,
                        &io->src.host,
                        &io->src.auth,
                        NULL,
                        NULL,
                        NULL,
                        &io->dst.laddr,
                        &header.host,
                        &io->dst.auth,
                        io->state.proxyprotocol == PROXY_DIRECT ?
                           NULL : &io->state.proxychain.server,
                        io->state.proxyprotocol == PROXY_DIRECT ?
                           NULL : &io->state.proxychain.extaddr,
                        NULL,
                        buf,
                        (size_t)r);

                  rawsocket_send(rawsocket,
                                 &io->src.host,
                                 sockaddr2sockshost(&io->src.laddr, &host),
                                 &header.host,
                                 -1);

#if BAREFOOTD
                  removeudpclient(udpclient->s, &io->dstc, iov->dstv);
#endif /* BAREFOOTD */
                  return IO_BLOCK;
               }

               if (io->dst.state.connected && !io->use_saved_rule) {
                  const int sametarget
                  = sockshostareeq(&header.host, &io->dst.host);

                  slog(LOG_DEBUG,
                       "%s: socket %d is connected to dst (%s), but "
                       "use_saved_rule is not set.   Means we got a SIGHUP. "
                       "Target for this packet is %s (%s before)",
                       function,
                       io->dst.s,
                       sockshost2string(&io->dst.host, hosta, sizeof(hosta)),
                       sockshost2string(&header.host, hostb, sizeof(hostb)),
                       sametarget ? "same as" : "different from");

                 if (!sametarget) {
                     socks_unconnect(io->dst.s);
                     io->dst.state.connected = 0;
                  }
               }

               if (sockscf.udpconnectdst
               &&  io->dst.written.packets == 0) { /* connect once only. */ 
                  char emsg[256];

                  slog(LOG_DEBUG,
                       "%s: connecting udp socket to %s, for client at %s",
                       function,
                       sockshost2string(&header.host, hosta, sizeof(hosta)),
                       sockshost2string(&io->src.host, hostb, sizeof(hostb)));

                  if (socks_connecthost(io->dst.s,
                                        &header.host,
                                        NULL,
                                        -1,
                                        emsg,
                                        sizeof(emsg)) == -1) {
                     iolog(packetrule,
                           &io->state,
                           OPERATION_ERROR_IO,
                           &io->src.laddr,
                           &io->src.host,
                           &io->src.auth,
                           NULL,
                           NULL,
                           NULL,
                           &io->dst.laddr,
                           &header.host,
                           &io->dst.auth,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                           NULL,
                           emsg,
                           strlen(emsg));

#if BAREFOOTD
                     removeudpclient(udpclient->s, &io->dstc, iov->dstv);
                     ++freefds;
#endif /* BAREFOOTD */

                     slog(LOG_DEBUG, "%s: free fds: %d", function, freefds);
                     return IO_TMPERROR;
                  }

                  io->dst.host            = header.host;
                  io->dst.state.connected = 1;
                  io->use_saved_rule      = 1; /* only if connected. */
               }
            }
            else {
               const int sametarget
               = sockshostareeq(&header.host, &io->dst.host);

               slog(LOG_DEBUG,
                    "%s: udp packet #%"PRIu64" from client %s.  Destination %s."
                    "  Previously set up with client-rule #%lu (%s) and "
                    "socks-rule #%lu (%s) for destination %s (%ssame as now)",
                    function,
                    io->src.read.packets,
                    sockaddr2string(&from, NULL, 0),
                    sockshost2string(&header.host, NULL, 0),
                    (unsigned long)io->crule.number,
                    verdict2string(io->crule.verdict),
                    (unsigned long)packetrule->number,
                    verdict2string(packetrule->verdict),
                    sockshost2string(&io->dst.host, hosta, sizeof(hosta)),
                    sametarget ? "" : "not ");

               if (sametarget)
                  permit = (packetrule->verdict == VERDICT_PASS);
               else {
                  /*
                   * use a tmprule.  If new destination is blocked, we
                   * might continue to use saved rule.
                   */
                  struct rule_t tmprule;
                  char client[MAXSOCKADDRSTRING],
                       dstbefore[MAXSOCKSHOSTSTRING],
                       dstnow[MAXSOCKSHOSTSTRING];

                  SASSERTX(!BAREFOOTD);

                  permit = rulespermit(io->control.s,
                                       &io->control.raddr,
                                       &io->control.laddr,
                                       &io->clientauth,
                                       &io->src.auth,
                                       &tmprule,
                                       &io->state,
                                       &io->src.host,
                                       &header.host,
                                       NULL,
                                       0);

                  slog(LOG_DEBUG, "%s: destination host for packet from %s "
                                  "changed from %s to %s, %s",
                                  function,
                                  sockaddr2string(&io->control.raddr,
                                                  client,
                                                  sizeof(client)),
                                  sockshost2string(&io->dst.host,
                                                   dstbefore,
                                                   sizeof(dstbefore)),
                                  sockshost2string(&header.host,
                                                   dstnow,
                                                   sizeof(dstnow)),
                                  permit ?
                                 "unconnecting from old address" 
                               : "but packets to this address is not permited");

                  if (!permit) {
                     iolog(&tmprule,
                           &io->state,
                           OPERATION_BLOCK_IO,
                           &io->src.laddr,
                           &io->src.host,
                           &io->src.auth,
                           NULL,
                           NULL,
                           NULL,
                           &io->dst.laddr,
                           &header.host,
                           &io->dst.auth,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                           io->state.proxyprotocol
                         == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                           NULL,
                           buf,
                           (size_t)r);

                     rawsocket_send(rawsocket,
                                    &io->src.host,
                                    sockaddr2sockshost(&io->src.laddr, &host),
                                    &io->dst.host,
                                    -1);

                     return IO_BLOCK;
                  }

                  socks_unconnect(io->dst.s);
                  io->dst.host            = header.host;
                  io->dst.state.connected = 0;
                  io->use_saved_rule      = 0;

                  *packetrule             = tmprule;
               }

               if (permit) { /* only bother with this if permit. */
                  if (redirect(io->dst.s,
                               &io->dst.laddr,
                               &io->dst.host,
                               io->state.command,
                               &packetrule->rdr_from,
                               &packetrule->rdr_to)
                  != 0) {
                     r = snprintf(buf, sizeof(buf), "redirect failed (%s)",
                                  strerror(errno));
                     failed = 1;
                  }
                 
                  if (!failed) {
                     sockshost2sockaddr(&header.host, &io->dst.raddr);
                     if (!ADDRISBOUND(TOIN(&io->dst.raddr))) {
                        r = snprintf(buf, sizeof(buf),
                                     "could not resolve dst %s for client %s",
                                     sockshost2string(&header.host, NULL, 0),
                                     sockaddr2string(&from, NULL, 0));

                        failed = 1;
                     }
                  }
               }
            }

#if SOCKS_SERVER
            /* set r to bytes sent by client sans socks UDP header. */
            r -= PACKETSIZE_UDP(&header);
#endif /* SOCKS_SERVER */

#if BAREFOOTD
            udpclient->src_read.bytes   += r;
            udpclient->src_read.packets += 1;
#endif /* BAREFOOTD */

#if BAREFOOTD
            if (!permit || failed)
               removeudpclient(udpclient->s, &io->dstc, iov->dstv);
#endif /* BAREFOOTD */

            if (permit && !failed)  {
               io->dst.host = header.host;

               if ((w = socks_sendto(io->dst.s,
#if BAREFOOTD
                                     buf,
#else /* SOCKS_SERVER */
                                     &buf[PACKETSIZE_UDP(&header)],
#endif /* SOCKS_SERVER */
                                     (size_t)r,
                                     lflags,
                                     io->dst.state.connected ?
                                     NULL : &io->dst.raddr,
                                     io->dst.state.connected ?
                                     0 : sizeof(io->dst.raddr),
                                     &io->dst.auth)) != r) {
                  r = snprintf(buf, sizeof(buf), "sendto() failed: %s", 
                               strerror(errno));
                  failed = 1;
               }
            }

            if (failed || !permit) {
               iolog(packetrule,
                     &io->state,
                     failed ? OPERATION_ERROR : OPERATION_BLOCK_IO,
                     &io->src.laddr,
                     &io->src.host,
                     &io->src.auth,
                     NULL,
                     NULL,
                     NULL,
                     &io->dst.laddr,
                     &header.host,
                     &io->dst.auth,
                     io->state.proxyprotocol
                        == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                     io->state.proxyprotocol
                        == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                     NULL,
                     buf,
                     (size_t)r);

               rawsocket_send(rawsocket,
                              &io->src.host,
                              sockaddr2sockshost(&io->src.laddr, &host),
                              &io->dst.host,
                              -1);

               return failed ? IO_TMPERROR : IO_BLOCK;
            }
            
            iolog(packetrule,
                  &io->state,
                  OPERATION_IO,
                  &io->src.laddr,
                  &io->src.host,
                  &io->src.auth,
                  NULL,
                  NULL,
                  NULL,
                  &io->dst.laddr,
                  &io->dst.host,
                  &io->dst.auth,
                  io->state.proxyprotocol
                     == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                  io->state.proxyprotocol
                     == PROXY_DIRECT ? NULL : &io->state.proxychain.extaddr,
                  NULL,
#if BAREFOOTD
                  buf,
#else /* SOCKS_SERVER */
                  &buf[PACKETSIZE_UDP(&header)],
#endif /* SOCKS_SERVER */
                  (size_t)w);

            gettimeofday(&io->iotime, NULL);

            bwused = MAX(0, w);

#if SOCKS_SERVER || COVENANT

            io->dst.written.bytes += MAX(0, w);
            if (w >= 0)
               ++io->dst.written.packets;

#else /* BAREFOOTD */

            udpclient->dst_written.bytes += MAX(0, w);
            if (w >= 0) {
               ++udpclient->dst_written.packets;
               udpclient->iotime = io->iotime;
            }

#endif /* BAREFOOTD */

            if (w >= 0) {
               io_update(&io->iotime,
                        MAX(0, w),
                        HAVE_TWO_LEVEL_ACL ? &io->rule : &io->crule,
                        sockscf.shmemfd);
            }
         }

         /*
          * Datagram reply from remote present?
          */

         if ((FD_ISSET(io->dst.s, rset) || srchaswritebuf)
         && FD_ISSET(io->src.s, wset)) {
            /*
             * - io->dst is src of packet, and can vary for each packet unless
             *   the socket is connected.
             *
             * - io->src is dst of packet (socks client).
             */
            const int lflags = flags & ~MSG_OOB;
            struct connectionstate_t replystate;
            struct sockaddr rfrom;
            struct sockshost_t replyto;
            char *newbuf;
            int redirected = 0, failed = 0;

#if BAREFOOTD
            udpclient = udpclientofsocket(io->dst.s, io->dstc, io->dstv);
            SASSERTX(udpclient != NULL);

            sockaddr2sockshost(&udpclient->client, &io->src.host);
            io->dst.laddr = udpclient->laddr;

            if (io->dst.state.connected)
               io->dst.raddr = udpclient->raddr;
#endif /* BAREFOOTD */

            len = sizeof(rfrom);
            if ((r = socks_recvfrom(io->dst.s,
                               buf,
                               buflen,
                               lflags,
                               &rfrom,
                               &len,
                               &io->dst.auth)) == -1) {
               if (ERRNOISTMP(errno) || errno == ECONNREFUSED) {
                  if (errno == ECONNREFUSED) {
                     /*
                      * means the error is actually from our previous
                      * write to this destination.  That failing is
                      * non-fatal, keep the session.
                      */

                     if (!io->dst.state.connected) {
                        /* can't be sure who the error is related to. */
                        io->dst.host.atype
                        = (unsigned char)SOCKS_ADDR_IPV4;
                        io->dst.host.addr.ipv4.s_addr = htonl(INADDR_ANY);
                        io->dst.host.port             = htons(0);
                     }
#if BAREFOOTD
                     else
                        sockaddr2sockshost(&io->dst.raddr, &io->dst.host);
#endif /* BAREFOOTD */

                     iolog(&io->rule,
                           &io->state,
                           OPERATION_ERROR_IO,
                           &io->dst.laddr,
                           &io->dst.host,
                           &io->dst.auth,
                           io->state.proxyprotocol == PROXY_DIRECT
                              ? NULL : &io->state.proxychain.server,
                           io->state.proxyprotocol == PROXY_DIRECT
                              ? NULL : &io->state.proxychain.extaddr,
                           NULL,
                           &io->src.laddr,
                           &io->src.host,
                           &io->src.auth,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           0);

                     /* can't send icmp error as we don't save sent data. */
                     return IO_TMPERROR;
                  }

                  return IO_EAGAIN;
               }

               /* other, unknown error.  Assume it's fatal. */
               *bad = io->dst.s;
               return IO_ERROR;
            }

#if SOCKS_SERVER
            if (io->dst.written.packets == 0) {
               slog(LOG_DEBUG,
                    "%s: how unusual ... the client at %s has not sent any "
                    "packets yet, but already it has a %ld byte reply from %s",
                    function,
                    sockaddr2string(&io->src.raddr, hosta, sizeof(hosta)),
                    (long)r,
                    sockaddr2string(&rfrom, hostb, sizeof(hostb)));

               if (!io->src.state.connected) {
                  r = snprintf(buf, sizeof(buf), 
                               "received a reply, but client has not sent us "
                               "it's address yet");

                  iolog(&io->rule,
                        &io->state,
                        OPERATION_ERROR_IO,
                        &io->dst.laddr,
                        &io->dst.host,
                        &io->dst.auth,
                        io->state.proxyprotocol == PROXY_DIRECT
                           ? NULL : &io->state.proxychain.server,
                        io->state.proxyprotocol == PROXY_DIRECT
                           ? NULL : &io->state.proxychain.extaddr,
                        NULL,
                        &io->src.laddr,
                        &io->src.host,
                        &io->src.auth,
                        NULL,
                        NULL,
                        NULL,
                        buf,
                        (size_t)r);

                  return IO_TMPERROR;
               }
            }
#endif /* SOCKS_SERVER */                          

            if (io->dst.state.connected
#if BAREFOOTD
            &&  udpclient->use_saved_replyrule
#else /* SOCKS_SERVER */
            &&  io->use_saved_replyrule
#endif /* SOCKS_SERVER */
            ) {
               /*
                * connected and not first reply; rule must have been
                * matched previously, so reuse previous match.
                */
               slog(LOG_DEBUG, "%s: not first udp reply from %s to %s, "
                               "length %ld.  "
                               "Already set up with reply-rule #%lu (%s), ",
                               function,
#if BAREFOOTD
                               sockaddr2string(&udpclient->laddr, NULL, 0),
                               sockaddr2string(&udpclient->client, NULL, 0),
                               (long)r,
                               (unsigned long)udpclient->replyrule.number,
                               verdict2string(udpclient->replyrule.verdict));
#else /* SOCKS_SERVER */
                               sockaddr2string(&io->dst.raddr, NULL, 0),
                               sockaddr2string(&io->src.raddr, NULL, 0),
                               (long)r,
                               (unsigned long)io->cmd.udp.replyrule->number,
                               verdict2string(io->cmd.udp.replyrule->verdict));
#endif /* SOCKS_SERVER */

#if BAREFOOTD
               io->crule              = udpclient->crule;
               io->rule               = udpclient->rule;
               io->cmd.udp.replyrule = &udpclient->replyrule;
#endif /* BAREFOOTD */

               permit              = io->cmd.udp.replyrule->verdict
                                     == VERDICT_PASS;
               replystate          = io->state;
               replystate.command  = SOCKS_UDPREPLY;
            }
            else {
               /*
                * First reply, or not connected.  Is it allowed in?
                */

               slog(LOG_DEBUG, "%s: udp reply from %s to %s, length = %ld",
                               function,
                               sockaddr2string(&rfrom, hosta, sizeof(hosta)),
                               sockshost2string(&io->src.host, hostb, 0),
                               (long)r);

               sockaddr2sockshost(&rfrom, &io->dst.host);

#if BAREFOOTD
               bzero(&io->clientauth, sizeof(io->clientauth));
               io->clientauth.method = AUTHMETHOD_NOTSET;
#endif /* BREFOOTD */

               bzero(&io->dst.auth, sizeof(io->dst.auth));
               io->dst.auth.method   = AUTHMETHOD_NOTSET;

               replystate            = io->state;
               replystate.command    = SOCKS_UDPREPLY;

               permit = rulespermit(
#if SOCKS_SERVER
                                    io->control.s,
                                    &io->control.raddr,
                                    &io->control.laddr,
#else /* BAREFOOTD */
                                    io->dst.s,
                                    &rfrom,
                                    &io->control.laddr,
#endif /* BAREFOOTD */
                                    &io->clientauth,
                                    &io->dst.auth,
                                    io->cmd.udp.replyrule,
                                    &replystate,
                                    &io->dst.host,
                                    &io->src.host,
                                    NULL,
                                    0);

               if (io->dst.state.connected) { /* save it for future replies. */
#if BAREFOOTD
                  udpclient->replyrule           = *io->cmd.udp.replyrule;
                  udpclient->use_saved_replyrule = 1;
#else /* SOCKS_SERVER  */
                  io->use_saved_replyrule = 1;
#endif /* SOCKS_SERVER */
               }
            }

#if BAREFOOTD
            udpclient->dst_read.bytes   += r;
            udpclient->dst_read.packets += 1;
#else /* !BAREFOOTD */
            io->dst.read.bytes   += r;
            io->dst.read.packets += 1;
#endif /* BAREFOOTD */

            if (permit) {
               replyto = io->src.host;
               if (redirect(io->src.s,
                            &rfrom,
                            &replyto,
                            replystate.command,
                            &io->cmd.udp.replyrule->rdr_from,
                            &io->cmd.udp.replyrule->rdr_to) != 0) {

                  r = snprintf(buf, sizeof(buf), "redirect failed (%s)",
                               strerror(errno));
                  failed = 1;
               }   
            }

#if !BAREFOOTD /* barefootd never connects to client. */
            if (permit && !failed) {
               if (!sockshostareeq(&replyto, &io->src.host)) {
                  char oldto[MAXSOCKSHOSTSTRING], newto[MAXSOCKSHOSTSTRING];

                  slog(LOG_DEBUG,
                       "%s: need to redirect reply, unconnecting socket "
                       "temporarily from %s, for redirection to %s ...",
                       function,
                       sockshost2string(&io->src.host, oldto, sizeof(oldto)),
                       sockshost2string(&replyto, newto, sizeof(newto)));

                  if (socks_unconnect(io->src.s) != 0) {
                     r = snprintf(buf, sizeof(buf),
                                  "socks_unconnect() failed (%s)",
                                  strerror(errno));

                     swarnx("%s: %s", function, buf);
                     failed = 1;
                  }

                  if (!failed)
                     redirected = 1;
               }
            }
#endif /* !BAREFOOTD */

            if (failed || !permit) {
               iolog(io->cmd.udp.replyrule,
                     &replystate,
                     failed ? OPERATION_ERROR_IO : OPERATION_BLOCK_IO,
                     &io->dst.laddr,
                     &io->dst.host,
                     &io->dst.auth,
                     io->state.proxyprotocol
                        == PROXY_DIRECT ? NULL : &io->state.proxychain.server,
                     io->state.proxyprotocol == PROXY_DIRECT
                        ? NULL : &io->state.proxychain.extaddr,
                     NULL,
                     &io->src.laddr,
                     &io->src.host,
                     &io->src.auth,
                     NULL,
                     NULL,
                     NULL,
                     buf,
                     (size_t)r);

               rawsocket_send(rawsocket,
                              &io->dst.host,
                              sockaddr2sockshost(&io->dst.laddr, &host),
                              &io->src.host,
                              -1);

               return IO_BLOCK;
            }

#if BAREFOOTD
            newbuf = buf;
#else /* SOCKS_SERVER; add socks UDP header before sending to client.  */
            newbuf = udpheader_add(&io->dst.host, buf, (size_t *)&r, buflen);
            SASSERTX(newbuf == buf);
#endif /* SOCKS_SERVER */

            w = socks_sendto(io->src.s,
                             newbuf,
                             (size_t)r,
                             lflags,
#if BAREFOOTD /* never connected; same socket receives from all clients. */
                             (struct sockaddr *)&udpclient->client,
                             sizeof(udpclient->client),
#else /* SOCKS_SERVER: always connected. */
                              NULL,
                              0,
#endif /* SOCKS_SERVER */
                              &io->src.auth);

            iolog(io->cmd.udp.replyrule,
                  &replystate,
                  w >= 0 ? OPERATION_IO : OPERATION_ERROR_IO,
                  &io->dst.laddr,
                  &io->dst.host,
                  &io->dst.auth,
                  io->state.proxyprotocol == PROXY_DIRECT
                     ? NULL : &io->state.proxychain.server,
                  io->state.proxyprotocol == PROXY_DIRECT
                     ? NULL : &io->state.proxychain.extaddr,
                  NULL,
                  &io->src.laddr,
                  &io->src.host,
                  &io->src.auth,
                  NULL,
                  NULL,
                  NULL,
                  w >= 0 ? buf : NULL,
                  w >= 0 ? w   : 0);
            

            if (w >= 0) {
               gettimeofday(&io->iotime, NULL);

#if BAREFOOTD
               udpclient->iotime = io->iotime;
#endif /* BAREFOOTD */

               bwused += w;

               io_update(&io->iotime,
                        w,
                        HAVE_TWO_LEVEL_ACL ? &io->rule :  &io->crule,
                        sockscf.shmemfd);
            }
            else {
               rawsocket_send(rawsocket,
                              &io->dst.host,
                              sockaddr2sockshost(&io->dst.laddr, &host),
                              &io->src.host,
                              -1);
            }

#if BAREFOOTD
            udpclient->src_written.bytes += MAX(0, w);
            if (w >= 0)
               ++udpclient->src_written.packets;

#else /* !BAREFOOTD */
            io->src.written.bytes += MAX(0, w);
            if (w >= 0)
               ++io->src.written.packets;
#endif /* BAREFOOTD */

            if (redirected) /* connect back to client. */
               if (connect(io->src.s, &io->src.raddr, sizeof(io->src.raddr))
               != 0) {
                  swarn("%s: failed to connect back to client %s after "
                        "redirection (%s)",
                        function,
                        sockaddr2string(&io->src.raddr, NULL, 0),
                        strerror(errno));

                  rawsocket_send(rawsocket,
                                 &io->dst.host,
                                 sockaddr2sockshost(&io->dst.laddr, &host),
                                 &io->src.host,
                                 -1);

                  *bad = io->src.s;
                  return IO_ERROR;
               }

            if (w == -1) {
               if (ERRNOISTMP(errno))
                  return IO_TMPERROR;

               rawsocket_send(rawsocket,
                              &io->dst.host,
                              sockaddr2sockshost(&io->dst.laddr, &host),
                              &io->src.host,
                              -1);

               *bad = io->src.s;
               return IO_ERROR;
            }
         }

         break;
      }
#endif /* HAVE_UDP_SUPPORT */

      default:
         SERRX(io->state.protocol);
   }

#if BAREFOOTD
   if (io->state.protocol == SOCKS_UDP && io->dst.s == -1)
      SYNC_UDPDST(&io->dst, &io->dstv[0]); /* needs to point at something. */
#endif /* BAREFOOTD */

   slog(LOG_DEBUG, "%s: bwused = %ld", function, (unsigned long)bwused);
   if (bwused)
      return IO_NOERROR;
   else
      return IO_EAGAIN;
}

static int
io_rw(in, out, bad, reqflags, buf, bufsize, bufused, flag)
   struct sockd_io_direction_t *in;
   struct sockd_io_direction_t *out;
   int *bad;
   const requestflags_t *reqflags;
   char *buf;
   size_t bufsize;
   size_t *bufused;
   int flag;
{
   const char *function = "io_rw()";
   ssize_t r, w, p;
   size_t bufusedmem = 0;

   if (bufused == NULL)
      bufused = &bufusedmem;

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: %d -> %d, bufsize = %lu, bufused = %lu, flag = %d",
                      function,
                      in->s,
                      out->s,
                      (unsigned long)bufsize,
                      (unsigned long)*bufused,
                      flag);

   *bad = -1; /* no error so far. */

   if (in->state.err != 0) {
      slog(LOG_DEBUG, "%s: failure already detected on socket %d "
                      "(%s, errno = %d)",
                      function, in->s,
                      strerror(in->state.err), in->state.err);

      errno = in->state.err;
      *bad  = in->s;

      return -1;
   }

   if (out->state.err != 0) {
      slog(LOG_DEBUG, "%s: failure already detected on socket %d "
                      "(%s, errno = %d)",
                      function, out->s,
                      strerror(out->state.err), out->state.err);

      errno = out->state.err;
      *bad  = out->s;

      return -1;
   }

   /*
    * read data from in ...
    */


   /*
    * We receive OOB in-line.  If flag has MSG_OOB set, it means we
    * are at the OOB marker.  The next byte we read (and it will be
    * only one) is the OOB byte, and since we receive it in-line,
    * we should turn off the MSG_OOB flag.
    *
    * When we write the data we will keep the MSG_OOB flag, and
    * hopefully that will work even if the write is combined with
    * data read from the buffer (so we send more than one byte). 
    * The last byte, which we will now read from the socket, should then be
    * tagged as the "oob" data.  Possible this will not work 100% correctly 
    * on some non-BSD implementations, but go with it for now.
    */

   if (in->flags & MSG_OOB)
      /*
       * The problem with oob is that select(2) signals we have oob until;
       * we've read past it (i.e., read at least one normal byte). 
       *
       * To handle receiving two oob-bytes in a row, we need to check whether
       * the last byte we receved on this socket was also an oob-byte, as 
       * if it was, we can't use select(2) to check for it (select would
       * keep returning, since oob is set until _past_ the byte we read), and
       * we must instead check here for oob regardless of what flag says.
       */
      flag |= MSG_OOB;

   if (flag & MSG_OOB) {
      if (sockatmark(in->s) != 1)
         flag &= ~MSG_OOB;
      else
         slog(LOG_DEBUG, "%s: have OOB data on socket %d", function, in->s);
   }

   /*
    * never read more from in than we can write to out, iobuf included.
    * Since we don't know how much we can write to the socket, except
    * it should normally always be at least one byte, only count the
    * space left in the iobuf.
    * Also make sure we can always NUL-terminate buf if necessary.
    */
   p = MIN(bufsize - *bufused, socks_freeinbuffer(out->s, WRITE_BUF));
   SASSERTX(p > 0);

#if HAVE_GSSAPI
   /*
    * If the data we are writing needs to be gssapi-encapsulated,
    * also make sure we don't read more than we can encapsulate in
    * a gssapi token; we don't want to deal with "segmentation" of
    * gssapi tokens.
    */
   if (out->auth.method == AUTHMETHOD_GSSAPI) {
      SASSERTX(out->auth.mdata.gssapi.state.maxgssdata > 0);
      p -= GSSAPI_OVERHEAD(&out->auth.mdata.gssapi.state);

      p = MIN(p, (ssize_t)out->auth.mdata.gssapi.state.maxgssdata);
      if (p <= 0) {
         /*
          * We are not expecting this to happen since we should not
          * get here as long as we have unflushed data left in 
          * the buffer.
          */
         swarnx("%s: write buffer for socket %d is almost full.  "
                "Only %lu bytes free, with a gssapi overhead of %lu",
                function, out->s,
                (unsigned long)socks_freeinbuffer(out->s, WRITE_BUF),
                (unsigned long)GSSAPI_OVERHEAD(
                  &out->auth.mdata.gssapi.state));
         p = 1; 
      }
   }
#endif /* HAVE_GSSAPI */

#if COVENANT
   if (in->state.isclientside && !reqflags->httpconnect)
      flag |= MSG_PEEK;
#endif /* COVENANT  */

   r = socks_recvfrom(in->s,
                      &buf[*bufused],
                      p,
                      flag & ~MSG_OOB,
                      NULL,
                      NULL,
                      &in->auth);

   if (r <= 0) {
      if (r == 0) {
         /*
          * FIN from "in".  It won't send us any more data, so
          * we shutdown "out" for writing to let it know.
          *
          * When "out" has nothing more to send, it will send us a
          * FIN too, and we will shutdown "in" for writing.
          * At that point, both "in" and "out" have sent a FIN,
          * meaning none of them will send us any more data.
          * Only then can we close the socket.  Since we may clear
          * state.fin however, state.shutdown should be used
          * for testing here.
          */

         slog(LOG_DEBUG, "%s: got EOF on in->s (%d) when trying to read up to "
                         "%ld bytes",
                         function, in->s, (long)p);

         in->state.fin = 1;

#if HAVE_DARWIN
         if (in->read.bytes == 0) {
            socklen_t optlen;
            int opt;

            optlen = sizeof(opt);
            if (getsockopt(in->s, SOL_SOCKET, SO_RCVBUF, &opt, &optlen) == -1)
               swarn("%s: getsockopt(SO_RCVBUF)", function);

            if (opt == 0)
               swarnx("%s: There appears to be a bug in the OS X Kernel, "
                      "v10.5.0 at least, that for some reason makes a sockets "
                      "SO_RCVBUF become zero sometimes during the processes of "
                      "passing the filedescriptor around.  "
                      "Subsequent reads from it return 0 even if the other "
                      "side has not closed the connection.  "
                      "This makes TCP's EOF indication not work correctly, "
                      "and %s ends up closing the session prematurely.",
                      function, PACKAGE);
         }
#endif /* HAVE_DARWIN */

         SASSERTX(socks_bytesinbuffer(out->s, WRITE_BUF, 0) == 0);

         if (in->state.shutdown_wr) /* have already received FIN from out. */
            *bad = out->s; /* done with this socket, "out" closed first. */

         if (!out->state.shutdown_wr) {
            /*
             * use shutdown() to forward FIN, but continue reading.
             */
            slog(LOG_DEBUG, "%s: shutting down out->s (%d) for writing",
            function, out->s);

            if (shutdown(out->s, SHUT_WR) != 0) {
               slog(LOG_DEBUG,
                    "%s: shutdown() for write towards remote peer failed: %s",
                    function, strerror(errno));

               *bad = out->s;
            }
            else
               out->state.shutdown_wr = 1;
         }
      }
      else
         if (!ERRNOISTMP(errno))
            *bad = in->s;

      return r;
   }

#if COVENANT
   if (in->state.isclientside && !reqflags->httpconnect) {
      /*
       * As long as the target of the clients request does not change, we
       * can forward it as normal.  If it changes, we need to restart
       * negotiation however.
       * Since we have no other way to know when the target changes, we have
       * to parse all data from the the http client before we can forward it,
       * as if the request is to a different server, it should not be
       * forwarded to the current target.
       */
      const char *http_eof = "\r\n\r\n";
      struct sockd_client_t client;
      char *p, emsg[512];

      buf[*bufused + r] = NUL;
      p = strstr(buf, http_eof);

      slog(LOG_DEBUG, "%s: read %ld bytes now, %lu bytes in total.  "
                      "%s HTTP request eof",
                      function, (long)r, (unsigned long)*bufused + r,
                      p == NULL ? "Not yet at" : "Now have");

      if (p == NULL)
         ;  /* no eof yet, save all read. */
      else { /* got the end of the request.  How far out in the buffer is it? */
          r        = (p + strlen(http_eof)) - buf;
          *bufused = 0;
      }

      flag &= ~MSG_PEEK;

      /* re-read the data peeked at. */
      w = socks_recvfrom(in->s,
                         &buf[*bufused],
                         r,
                         flag & ~MSG_OOB,
                         NULL,
                         NULL,
                         &in->auth);
      SASSERTX(r == w);

      if (p == NULL)
         return 0; /* no eof.  Return. */

      /*
       * got the request.  Parse it and see if the target is still
       * the same.
       */
       client.request.auth = &client.auth;
       if (parse_httprequest(&client.request, buf, emsg, sizeof(emsg)) != 0) {
         char visbuf[2048];

         swarnx("%s: failed to parse http request \"%s\" from %s: %s",
                function,
                socket2string(in->s, NULL, 0),
                str2vis(buf, r, visbuf, sizeof(visbuf)),
                emsg);
      }

      if (!sockshostareeq(&out->host, &client.request.host)) {
         char old[MAXSOCKSHOSTSTRING], new[MAXSOCKSHOSTSTRING];

         slog(LOG_DEBUG, "%s: client at %s changing target from %s to %s.  "
                         "Need to renegotiate",
                         function,
                         socket2string(in->s, NULL, 0),
                         sockshost2string(&out->host, old, sizeof(old)),
                         sockshost2string(&client.request.host,
                                          new, sizeof(new)));

         memcpy(client.clientdata, buf, *bufused + r);
         client.clientdatalen = *bufused + r;
         client.s             = in->s;
         gettimeofday(&client.accepted, NULL);

         send_client(sockscf.state.mother.s, &client, buf, *bufused);

         *bad = in->s;
         return 0;
      }
      else
         slog(LOG_DEBUG, "%s: no problem, target in the new request is the "
                         "same as before (%s)",
                         function,
                         sockshost2string(&client.request.host, NULL, 0));
   }
#endif /* COVENANT */

   in->read.bytes += r;

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: read %ld", function, (long)r);

   if (flag & MSG_OOB)
      in->flags |= MSG_OOB;   /* read oob data.            */
   else
      in->flags &= ~MSG_OOB;  /* did not read oob data.    */

   /*
    * ... and send the data read to out.
    */

   if ((w = socks_sendto(out->s, buf, (size_t)r, flag, NULL, 0, &out->auth))
   > 0)
      out->written.bytes += w;

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: wrote %ld", function, (long)w);

   if (w != r) {
      if (w == -1)
         *bad = out->s;
      else {
          /*
           * Should never read more than we can write/buffer.
           */
         swarn("%s: wrote %ld/%ld", function, (long)w, (long)r);
         SASSERT(0);
         *bad  = out->s;
      }
   }

   /*
    * we want to select for read again on socket we sent data out on,
    * regardless of whether we have received a FIN from it, to get
    * write errors.
    *
    * Unfortunately, there's no way to make select() not keep
    * returning ready-for-read once the client has sent the FIN,
    * and we do not want to busy-loop around this.  What we would want,
    * is to only select for error on the socket after we receive
    * a FIN.
    * Best we can do is to let io_fillset() skip sockets that
    * have state.fin set, and reset state.fin if we send data on on the
    * socket, hoping to catch any pending errors on second go round.
    * This means some sessions can occupy space for a long time, until
    * tcp keep-alive check kicks in.
    */
   out->state.fin = 0;

   return w;
}

static void
proctitleupdate(void)
{

   setproctitle("iorelayer: %lu/%d",
   (unsigned long)io_allocated(NULL, NULL), SOCKD_IOMAX);
}

static struct sockd_io_t *
io_getset(nfds, set)
   const int nfds;
   const fd_set *set;
{
   struct sockd_io_t *best, *evaluating;
   size_t i;
   int s;

   for (s = 0, best = NULL; s < nfds; ++s) {
      if (!FD_ISSET(s, set))
         continue;

      /*
       * find the io d is part of.
       */
      for (i = 0, evaluating = NULL; i < ioc; ++i) {
         if (!iov[i].allocated)
            continue;

         if (s == iov[i].src.s || s == iov[i].dst.s) {
            evaluating = &iov[i];
            break;
         }

         switch (iov[i].state.command) {
            case SOCKS_BIND:
            case SOCKS_BINDREPLY:
               if (iov[i].state.extension.bind && s == iov[i].control.s)
                  evaluating = &iov[i];
               break;

            case SOCKS_UDPASSOCIATE: {
               if (s == iov[i].control.s)
                  evaluating = &iov[i];
#if BAREFOOTD
               else {
                  struct udpclient *client;

                  if ((client = udpclientofsocket(s, iov[i].dstc, iov[i].dstv))
                  != NULL) {
                     SYNC_UDPDST(&iov[i].dst, client);
                     evaluating = &iov[i];
                  }
               }
#endif /* BAREFOOTD */
               break;
            }

            default:
               break;
         }

         if (evaluating != NULL)
            break;
      }

      SASSERTX(evaluating != NULL);

      /* want the i/o object that has least recently done i/o. */
      if (best == NULL || timercmp(&evaluating->iotime, &best->iotime, <))
         best = evaluating;
   }

   return best;
}


static struct sockd_io_t *
io_finddescriptor(d)
   int d;
{
   size_t i;

   for (i = 0; i < ioc; ++i)
      if (iov[i].allocated) {
         if (d == iov[i].src.s || d == iov[i].dst.s) {
#if BAREFOOTD
            if (d == iov[i].src.s
            && iov[i].state.command == SOCKS_UDPASSOCIATE)
               /* dummy socket. demux later. */
               SYNC_UDPDST(&iov[i].dst, &iov[i].dstv[0]);
#endif /* BAREFOOTD */

            return &iov[i];
         }

         switch (iov[i].state.command) {
            case SOCKS_BIND:
            case SOCKS_BINDREPLY:
               if (!iov[i].state.extension.bind)
                  break;
               /* else: */ /* FALLTHROUGH */

            case SOCKS_UDPASSOCIATE: {
#if BAREFOOTD
               struct udpclient *udpclient;
               if ((udpclient = udpclientofsocket(d, iov[i].dstc, iov[i].dstv))
               != NULL) {
                  SYNC_UDPDST(&iov[i].dst, udpclient);
                  return &iov[i];
               }
#else
               if (d == iov[i].control.s)
                  return &iov[i];
#endif /* BAREFOOTD */
               break;
            }

            default:
               break;
         }
      }

   return NULL;
}

static int
io_fillset(set, antiflags, bwoverflowtil)
   fd_set *set;
   int antiflags;
   struct timeval *bwoverflowtil;
{
   const char *function = "io_fillset()"; 
   struct timeval tnow, firstbwoverflowok;
   size_t i;
   int max;

   gettimeofday(&tnow, NULL);
   timerclear(&firstbwoverflowok);

   FD_ZERO(set);
   for (i = 0, max = -1; i < ioc; ++i) {
      struct sockd_io_t *io = &iov[i];

      if (!io->allocated)
         continue;

#if BAREFOOTD
      /* udp-clients need special handling in barefootd. */
      if (io->state.protocol == SOCKS_TCP) {
#endif /* BAREFOOTD */

      if (io->rule.bw_shmid != 0) {
         struct timeval bwoverflowok, howlongtil;

         if (bw_rulehasoverflown(&io->rule,
                                 &tnow,
                                 &bwoverflowok,
                                io->control.s == -1 ? io->src.s : io->control.s,
                                 io->src.s,
                                 io->dst.s)) {
               if (!timerisset(&firstbwoverflowok)
               ||  timercmp(&bwoverflowok, &firstbwoverflowok, <))
                  firstbwoverflowok = bwoverflowok;

            SASSERTX(timercmp(&bwoverflowok, &tnow, >)); 
            timersub(&bwoverflowok, &tnow, &howlongtil); 
            slog(LOG_DEBUG,                                                     
                 "%s: skipping io #%lu belonging to rule #%lu/bw_shmid %lu "
                 "due to bwoverflow.  Fds are %d, %d, %d.  "
                 "Have to wait for %ld.%06lds, til %ld.%06ld)",   
                 function,                                                      
                 (unsigned long)i,
                 (unsigned long)(io->rule.number),                 
                 (unsigned long)(io->rule.bw_shmid),
                 io->control.s,
                 io->src.s,
                 io->dst.s,
                 (long)howlongtil.tv_sec,
                 (long)howlongtil.tv_usec,
                 (long)bwoverflowok.tv_sec,
                 (long)bwoverflowok.tv_usec);

            continue;
         }
      }

#if BAREFOOTD
      }
#endif /* BAREFOOTD */

      if (!io->src.state.fin && !(antiflags & io->src.flags)) {
         FD_SET(io->src.s, set);
         max = MAX(max, io->src.s);
      }

      if (!io->dst.state.fin && !(antiflags & io->dst.flags)) {
#if BAREFOOTD
         if (io->state.command == SOCKS_UDPASSOCIATE) {
            size_t ii;

            for (ii = 1; ii < io->dstc; ++ii) {
               if (io->dstv[ii].crule.bw_shmid != 0) {
                  struct timeval bwoverflowok;

                  if (bw_rulehasoverflown(&io->dstv[ii].crule,
                                          &tnow,
                                          &bwoverflowok,
                                          io->control.s,
                                          io->src.s,
                                          io->dstv[ii].s)) {
                     if (!timerisset(&firstbwoverflowok)
                     ||  timercmp(&bwoverflowok, &firstbwoverflowok, <))
                        firstbwoverflowok = bwoverflowok;

                     continue;
                  }
               }

               FD_SET(io->dstv[ii].s, set);
               max = MAX(max, io->dstv[ii].s);
            }
         }
         else {
            if (iov[i].state.command == SOCKS_CONNECT
            && !iov[i].dst.state.connected)
               ;
            else {
               FD_SET(io->dst.s, set);
               max = MAX(max, io->dst.s);
            }
         }
#else /* SOCKS_SERVER */
         if (iov[i].state.command == SOCKS_CONNECT
         && !iov[i].dst.state.connected)
            ;
         else {
            FD_SET(io->dst.s, set);
            max = MAX(max, io->dst.s);
         }
#endif /* SOCKS_SERVER */
      }

#if SOCKS_SERVER
      switch (io->state.command) {
         case SOCKS_BIND:
         case SOCKS_BINDREPLY:
            if (!io->state.extension.bind)
               break;
            /* else: */ /* FALLTHROUGH */

         case SOCKS_UDPASSOCIATE:
            if (!(antiflags & io->control.flags)) {
               FD_SET(io->control.s, set);
               max = MAX(max, io->control.s);
            }
            break;

         default:
            break;
      }
#endif /* SOCKS_SERVER */
   }

   if (bwoverflowtil != NULL)
      *bwoverflowtil = firstbwoverflowok;

   return max;
}


static int
io_fillset_connectinprogress(set)
   fd_set *set;
{
   const char *function = "io_fillset_connectinprogress()";
   int i, bits, count;

   FD_ZERO(set);
   for (i = count = 0, bits = -1; (size_t)i < ioc; ++i) {
      if (iov[i].allocated
      && !iov[i].dst.state.connected
      && iov[i].state.command == SOCKS_CONNECT) {

         FD_SET(iov[i].dst.s, set);
         bits = MAX(bits, iov[i].dst.s);

         slog(LOG_DEBUG, "%s: socket %d marked as still connecting",
         function, iov[i].dst.s);

         ++count;
      }
   }

   return bits;
}

static struct timeval *
io_gettimeout(timeout)
   struct timeval *timeout;
{
/*   const char *function = "io_gettimeout()"; */
   struct timeval tnow;
   size_t i;
   int havetimeout;

   gettimeofday(&tnow, NULL);

   if (timerisset(&bwoverflowtil)) {
      struct timeval time_havebw;

      timersub(&bwoverflowtil, &tnow, &time_havebw);

      if (time_havebw.tv_sec < 0 
      || (time_havebw.tv_sec == 0 && time_havebw.tv_sec <= 0)) {
         timerclear(&bwoverflowtil);
         timerclear(timeout);
      }
      else
          *timeout = time_havebw;
      
      /*
       * else; still have some time to wait.  Since we assume we will
       * not overflow by too much, assume the bwoverflow "timeout" 
       * is always the shortest, or close enough.  I.e., don't bother
       * going through all ios to find one with a possibly slightly
       * shorter timeout.
       */

      if (timeout->tv_sec <= 1)
         /* 
          * good enough; don't bother going through all i/os to find one 
          * with a possibly slightly shorter timeout til deletion.
          */
         return timeout;
   }

   /*
    * Could perhaps add a "timeoutispossible" object also by checking 
    * each io object as we receive it (and each udp client as we 
    * add it).  If we find one where timeout is possible, set the 
    * global timeoutispossible, if not, don't set it.  Each time 
    * we delete_io(), we change timeoutispossible to true, and
    * upon scanning through all i/o's here, we may possible set it
    * to false again.
    */
   if (io_allocated(NULL, NULL) == 0)
      return NULL;

   for (i = 0, havetimeout = 0; i < ioc; ++i) {
      struct timeval timeout_found;

      timeout_found.tv_usec = 0;
      if ((timeout_found.tv_sec = io_timetiltimeout(&iov[i], &tnow, NULL))
      != -1) {
         if (!havetimeout
         ||  timercmp(&timeout_found, timeout, <)) {
            *timeout    = timeout_found;
            havetimeout = 1;
         }
      }

      if (havetimeout
      && (timeout->tv_sec <= 0 && timeout->tv_usec <= 0))
         return timeout; /* at or past timeout already, don't look further. */

      /* else; continue looking for the one that will time out most imminent. */
   }

   if (havetimeout)
      return timeout;
   else
      return NULL;
}

static struct sockd_io_t *
io_gettimedout(void)
{
   const char *function = "io_gettimedout()";
   struct timeval tnow;
   size_t i;

   gettimeofday(&tnow, NULL);
   for (i = 0; i < ioc; ++i) {
      struct timeval timeout;

      if (!iov[i].allocated)
         continue;

      if ((timeout.tv_sec = io_timetiltimeout(&iov[i], &tnow, NULL)) == -1)
         continue;  /* no timeout on this object. */

      timeout.tv_usec = 0; /* whole seconds is good enough. */

      if (timeout.tv_sec <= 0 && timeout.tv_usec <= 0) { /* timeout is now. */
         slog(LOG_DEBUG, "%s: io #%lu with control %d, src %d, dst %d, "
                         "has reached the timeout point",
                         function,
                         (unsigned long)i,
                         iov[i].control.s,
                         iov[i].src.s,
                         iov[i].dst.s);
         return &iov[i]; 
      }
   }

   return NULL;
}

static int
io_timeoutispossible(io)
   const struct sockd_io_t *io;
{

   if (!io->allocated)
      return 0;

   if (io->state.command == SOCKS_UDPASSOCIATE) {
#if BAREFOOTD
      if (io->dstc <= 1) /* index 0 is dummy. */
         return -1;
#endif /* BAREFOOTD */

      return io->rule.timeout.udpio != 0;
   }

   if (io->dst.state.connected) {
      if (io->src.state.shutdown_wr || io->dst.state.shutdown_wr)
         return io->rule.timeout.tcp_fin_wait != 0;
      else
         return io->rule.timeout.tcpio != 0;
   }
   else
      return io->rule.timeout.connect != 0;

   /* NOTREACHED */
   SERRX(0);
}


static long
io_timetiltimeout(io, tnow, timeouttype)
   struct sockd_io_t *io;
   const struct timeval *tnow;
   timeout_type_t *timeouttype;
{
   const char *function = "io_timetiltimeout()";
   timeout_type_t timeouttype_mem;
   time_t *lastio;
   long protocoltimeout;

   if (!io_timeoutispossible(io))
      return -1;

   if (timeouttype == NULL)
      timeouttype = &timeouttype_mem;

   /* 
    * First find out what the correct timeoutobject to use for this
    * io at this time is, and then see if a timeout value has been
    * set in that object (i.e., is not 0).
    */
#if BAREFOOTD
   if (io->state.command == SOCKS_UDPASSOCIATE) {
      size_t ii;
      long timetiltimeout;

      if (io->dstc <= 1) /* index 0 is dummy. */
         return -1;

      for (ii = 1, timetiltimeout = -1; ii < io->dstc; ++ii) {
         struct udpclient *udpclient = &io->dstv[ii];

         if (tnow->tv_sec < udpclient->iotime.tv_sec) {
            slog(LOG_DEBUG, "%s: clock was stepped backwards?", function);

            udpclient->iotime = *tnow;
            continue;
         }

         if (timetiltimeout == -1
         || (unsigned long)timetiltimeout
         > io->rule.timeout.udpio - (tnow->tv_sec - udpclient->iotime.tv_sec)) {
            timetiltimeout = io->rule.timeout.udpio
                             - (tnow->tv_sec - udpclient->iotime.tv_sec);

            *timeouttype    = TIMEOUT_IO;
            SYNC_UDPDST(&io->dst, udpclient);
         }
      }

      return timetiltimeout;
   }
#else /* SOCKS_SERVER */
   if (io->state.protocol == SOCKS_UDP) {
      *timeouttype    = TIMEOUT_IO; /* only type possible for an udp client. */
      protocoltimeout = io->rule.timeout.udpio;
      lastio          = (time_t *)&io->iotime.tv_sec;
   }
#endif /* SOCKS_SERVER */

   else {
      SASSERTX(io->state.protocol == SOCKS_TCP);

      if (io->dst.state.connected) {
         if (io->src.state.shutdown_wr || io->dst.state.shutdown_wr) {
            *timeouttype    = TIMEOUT_TCP_FIN_WAIT;
            protocoltimeout = io->rule.timeout.tcp_fin_wait;
         }
         else {
            *timeouttype    = TIMEOUT_IO;
            protocoltimeout = io->rule.timeout.tcpio;
         }

         lastio = (time_t *)&io->iotime.tv_sec;
      }
      else {
         *timeouttype    = TIMEOUT_CONNECT;
         protocoltimeout = io->rule.timeout.connect;
         lastio          = (time_t *)&io->state.time.established.tv_sec;
      }
   }

   if (protocoltimeout == 0)
      return -1;

   if (difftime(*lastio, tnow->tv_sec) > 0) {
      slog(LOG_DEBUG, "%s: clock was stepped backwards?", function);

      *lastio = tnow->tv_sec;
   }

   if (MAX(0, protocoltimeout - difftime(tnow->tv_sec, *lastio)) == 0)
      slog(LOG_DEBUG, "%s: protocoltimeout = %ld, tnow = %ld, lastio = %ld",
      function, protocoltimeout, (long)tnow->tv_sec, (long)*lastio);

   return MAX(0,
              protocoltimeout - ROUNDFLOAT(difftime(tnow->tv_sec, *lastio)));
}

static void
getnewios()
{
   const char *function = "getnewios()";
   int rc, receivedc;

   receivedc = 0;
   while ((rc = recv_io(sockscf.state.mother.s, NULL)) == 0)
      ++receivedc;

   slog(LOG_DEBUG, "%s: received %d new io%s, errno = %d (%s)",
        function,
        receivedc,
        receivedc == 1 ? "" : "s",
        errno,
        strerror(errno));

   SASSERTX(rc == -1);

   if (receivedc == 0)
      slog(LOG_INFO, "%s: strange ... we were called to receive a new io, "
                     "but no new io was there to receive (%s)",
                     function, strerror(errno));
   else
      proctitleupdate();
}

/* ARGSUSED */
static void
siginfo(sig, sip, scp)
   int sig;
   siginfo_t *sip;
   void *scp;
{
   const char *function = "siginfo()";
   unsigned long seconds, days, hours, minutes;
   struct timeval tnow;
   size_t i;

   if (sig > 0) {
      sockd_pushsignal(sig, sip);
      return;
   }

   sig = -sig;

   slog(LOG_DEBUG, "%s: running due to previously received signal: %d",
   function, sig);

   gettimeofday(&tnow, NULL);
   seconds = ROUNDFLOAT(difftime(time((time_t *)&tnow.tv_sec),
                                 sockscf.stat.boot));
   seconds2days(&seconds, &days, &hours, &minutes);

   slog(LOG_INFO, "io-child up %lu day%s, %lu:%.2lu:%.2lu",
                  days, days == 1 ? "" : "s", hours, minutes, seconds);

   for (i = 0; i < ioc; ++i) {
      char srcstring[MAX_IOLOGADDR], dststring[MAX_IOLOGADDR],
           timeinfo[64], idlestr[64];

      if (!iov[i].allocated)
         continue;

#if BAREFOOTD
      if (iov[i].state.protocol == SOCKS_TCP) {
#endif /* BAREFOOTD */
         BUILD_ADDRSTR_SRC(&iov[i].src.host,
                           NULL,
                           NULL,
                           &iov[i].src.laddr,
                           &iov[i].src.auth,
                           NULL,
                           srcstring,
                           sizeof(srcstring));

         BUILD_ADDRSTR_DST(&iov[i].dst.laddr,
                           iov[i].state.proxyprotocol == PROXY_DIRECT
                              ? NULL : &iov[i].state.proxychain.server,
                           iov[i].state.proxyprotocol == PROXY_DIRECT
                              ? NULL : &iov[i].state.proxychain.extaddr,
                           &iov[i].dst.host,
                           &iov[i].dst.auth,
                           NULL,
                           dststring,
                           sizeof(dststring));
#if BAREFOOTD
      }
#endif /* BAREFOOTD */

      if (iov[i].state.protocol == SOCKS_UDP
      ||  iov[i].dst.state.connected)
         snprintfn(idlestr, sizeof(idlestr), "%lds",
                   (long)(tnow.tv_sec - iov[i].iotime.tv_sec));
      else
         snprintfn(idlestr, sizeof(idlestr),
                   "%lds (waiting for connect to complete)",
                   (long)(  tnow.tv_sec
                          - iov[i].state.time.established.tv_sec));

#if BAREFOOTD
      if (iov[i].state.protocol == SOCKS_TCP)
#endif /* BAREFOOTD */
      snprintf(timeinfo, sizeof(timeinfo),
               "age: %lds, idle: %s",
               (long)(tnow.tv_sec - iov[i].state.time.accepted.tv_sec),
               idlestr);

      if (iov[i].state.protocol == SOCKS_UDP) {
#if BAREFOOTD
         struct udpclient *client;
         size_t srci;

         for (srci = 1; srci < iov[i].dstc; ++srci) {
            struct sockshost_t host;
            client = &iov[i].dstv[srci];

            BUILD_ADDRSTR_SRC(sockaddr2sockshost(&client->client, &host),
                              NULL,
                              NULL,
                              &iov[i].src.laddr,
                              &iov[i].src.auth,
                              NULL,
                              srcstring,
                              sizeof(srcstring));

            SYNC_UDPDST(&iov[i].dst, client);
            BUILD_ADDRSTR_DST(&iov[i].dst.laddr,
                              iov[i].state.proxyprotocol == PROXY_DIRECT
                                 ? NULL : &iov[i].state.proxychain.server,
                              iov[i].state.proxyprotocol == PROXY_DIRECT
                                 ? NULL : &iov[i].state.proxychain.extaddr,
                              &iov[i].dst.host,
                              &iov[i].dst.auth,
                              NULL,
                              dststring,
                              sizeof(dststring));

            snprintf(timeinfo, sizeof(timeinfo),
                     "age: %lds, idle: %lds",
                      (long)(tnow.tv_sec
                             - (srci == 0 ? iov[i].state.time.established.tv_sec
                                          : client->iotime.tv_sec)),
                     (long)(tnow.tv_sec
                             - (srci == 0 ? iov[i].state.time.established.tv_sec
                                          : client->firstio.tv_sec)));

            slog(LOG_INFO, "%s: %s <-> %s: %s, "
                           "bytes transferred: %"PRIu64" <-> %"PRIu64", "
                           "packets: %"PRIu64" <-> %"PRIu64"",
                           protocol2string(iov[i].state.protocol),
                           srcstring, dststring,
                           timeinfo,
                           client->dst_written.bytes,
                           client->src_written.bytes,
                           client->dst_written.packets,
                           client->src_written.packets);
         }
#else /* !BAREFOOTD */
         slog(LOG_INFO, "%s: %s <-> %s: %s, "
                        "bytes transferred: %"PRIu64" <-> %"PRIu64", "
                        "packets: %"PRIu64" <-> %"PRIu64"",
              protocol2string(iov[i].state.protocol),
              srcstring, dststring,
              timeinfo,
              iov[i].dst.written.bytes, iov[i].src.written.bytes,
              iov[i].dst.written.packets, iov[i].src.written.packets);
#endif /* !BAREFOOTD */
      }
      else { /* TCP */
         slog(LOG_INFO,
              "%s: %s <-> %s: %s, bytes transferred: %"PRIu64" <-> %"PRIu64"",
              protocol2string(iov[i].state.protocol),
              srcstring, dststring,
              timeinfo,
              iov[i].dst.written.bytes, iov[i].src.written.bytes);
      }
   }
}

static void
freebuffers(io)
   const struct sockd_io_t *io;
{
   if (io->control.s != -1 && io->control.s != io->src.s)
      socks_freebuffer(io->control.s);

   socks_freebuffer(io->src.s);
   socks_freebuffer(io->dst.s);
}

static int
checkconnectstatus(io, badfd)
   struct sockd_io_t *io;
   int *badfd;
{
   const char *function = "checkconnectstatus()";
   const int controlfd = (io->control.s == -1 ? io->src.s : io->control.s);
   socklen_t len;
   int secondtry = 0;

again:
   SASSERTX(io->state.command == SOCKS_CONNECT && !io->dst.state.connected);
   SASSERTX(io->dst.state.err == 0);

   /*
    * Check if the socket connected successfully.
    */
   len = sizeof(io->dst.raddr);
   if (getpeername(io->dst.s, &io->dst.raddr, &len) == 0) { 
      slog(LOG_DEBUG, "%s: connect to %s on socket %d completed successfully",
           function, sockshost2string(&io->dst.host, NULL, 0), io->dst.s);

      io->dst.state.connected = 1;

#if HAVE_NEGOTIATE_PHASE
      if (SOCKS_SERVER || io->reqflags.httpconnect) {
         struct response_t response;
         struct sockshost_t host;

         if (SOCKS_SERVER)
            sockaddr2sockshost(&io->dst.laddr, &host);
         else
            host = io->dst.host;

         create_response(&host,
                         &io->src.auth,
                         io->state.version,
                         SOCKS_SUCCESS,
                         &response);

         if (send_response(controlfd, &response) != 0) {
            *badfd = controlfd;
            return -1;
         }
      }
#endif /* HAVE_NEGOTIATE_PHASE */

      iolog(&io->rule,
            &io->state,
            OPERATION_CONNECT,
            &io->src.laddr,
            &io->src.host,
            &io->src.auth,
            NULL,
            NULL,
            NULL,
            &io->dst.laddr,
            &io->dst.host,
            &io->dst.auth,
            io->state.proxyprotocol == PROXY_DIRECT ?
               NULL : &io->state.proxychain.server,
            io->state.proxyprotocol == PROXY_DIRECT ?
               NULL : &io->state.proxychain.extaddr,
            NULL,
            NULL,
            0);

      return 0;
   }
   else { /* connect(2) failed. */
      char src[MAXSOCKSHOSTSTRING], dst[MAXSOCKSHOSTSTRING];

      slog(LOG_DEBUG, "%s: getpeername(%d) failed: %s",
           function, io->dst.s, strerror(errno));

      len               = sizeof(errno);
      getsockopt(io->dst.s, SOL_SOCKET, SO_ERROR, &errno, &len);
      io->dst.state.err = errno;

      slog(LOG_DEBUG, "%s: connect-attempt to %s on socket %d, on behalf of "
                      "client %s has completed.  Status is: %s, errno = %d",
                      function,
                      sockshost2string(&io->dst.host, src, sizeof(src)),
                      io->dst.s,
                      sockshost2string(&io->src.host, dst, sizeof(dst)),
                      strerror(errno),
                      errno);

      if (errno == 0) {
         /* 
          * still in progress?  Can't be, we should only be called when the 
          * connect(2) has finished, as indicated by e.g. select(2).
          * Could also be a race-condition however, if the connect(2) 
          * completed successfully between our getpeername(2) call above and 
          * now.  Very unlikely, but have to check none the less.
          */ 
          char src[MAXSOCKADDRSTRING * 3], dst[MAXSOCKADDRSTRING * 3];

          if (!secondtry) { 
            secondtry = 1;
            goto again;
          }

          SASSERTX(0);

          /* ... if asserts are disabled. */
          swarnx("%s: XXX busy-loop bug detected regarding request %s "
                 "from client %s, destination %s.  " 
                 "Please report this to \"dante-bugs@inet.no\".",
                 function,
                 sockshost2string(&io->dst.host, NULL, 0),
                 socket2string(io->src.s, src, sizeof(src)),
                 socket2string(io->dst.s, dst, sizeof(dst)));

#if 1 /* workaround */
         *badfd = io->dst.s;
         return -1;
#else
#warning "change before release"
         SERRX(0);
#endif
      }

#if HAVE_NEGOTIATE_PHASE
      if (SOCKS_SERVER || io->reqflags.httpconnect) {
         struct response_t response;

         create_response(NULL,
                         &io->src.auth,
                         io->state.version,
                         errno2reply(errno, io->state.version),
                         &response);

         if (send_response(controlfd, &response) != 0)
            /* want errno to be the previous errno, from connect(2). */
            errno = io->dst.state.err;
      }
#endif /* HAVE_NEGOTIATE_PHASE */

      *badfd = io->dst.s;
      return -1;
   }
}

static void
rawsocket_send(s, peer, local, dst, code)
   const int s;
   const struct sockshost_t *peer;
   const struct sockshost_t *local;
   const struct sockshost_t *dst;
   const int code;
{
   const char *function = "rawsocket_send()";
   char peer_str[MAXSOCKSHOSTSTRING], local_str[MAXSOCKSHOSTSTRING],
        dst_str[MAXSOCKSHOSTSTRING];

   slog(LOG_DEBUG,
        "%s: XXX should send an icmp error with code %d to %s, concerning "
        "packet for %s received on %s, but that is not implemented yet",
        function,
        code,
        sockshost2string(peer, peer_str, sizeof(peer_str)),
        dst == NULL ?
           "0.0.0.0" : sockshost2string(dst, dst_str, sizeof(dst_str)),
        sockshost2string(local, local_str, sizeof(local_str)));

}

static void 
io_update(timenow, bwused, rule, lock)                          
   const struct timeval *timenow;
   const size_t bwused;
   struct rule_t *rule;
   const int lock;
{                                                                           
   const int needattach = (rule->bw == NULL);

   if (rule->bw_shmid == 0 || bwused == 0)
      return;

   if (needattach)
      sockd_shmat(rule, SHMEM_BW);
                                                                   
   bw_update(rule->bw, bwused, timenow, lock);                       
                                                                  
   if (needattach)
      sockd_shmdt(rule, SHMEM_BW);
}


#if BAREFOOTD

static size_t
io_udpclients(void)
{
   const char *function = "io_udpclients()";
   size_t i, allocated;

   for (i = 0, allocated = 0; i < ioc; ++i) {
      if (!iov[i].allocated)
         continue;

      if (iov[i].state.protocol != SOCKS_UDP)
         continue;

      SASSERTX(iov[i].dstc >= 1);

      allocated += (iov[i].dstc - 1); /* first index is dummy slot. */
   }

   slog(LOG_DEBUG, "%s: have %lu udp client%s",
   function, (unsigned long)allocated, allocated == 1 ? "" : "s");

   return allocated;
}

static struct udpclient *
udpclientofsocket(s, udpclientc, udpclientv)
   const int s;
   const size_t udpclientc;
   struct udpclient *udpclientv;
{
   static size_t i;

   if (i < udpclientc && udpclientv[i].s == s)
      return &udpclientv[i];

   for (i = 0; i < udpclientc; ++i)
      if (udpclientv[i].s == s)
         return &udpclientv[i];

   return NULL;
}

static struct udpclient *
udpclientofladdr(addr, udpclientc, udpclientv)
   const struct sockaddr *addr;
   const size_t udpclientc;
   struct udpclient *udpclientv;
{
   size_t i;

   for (i = 0; i < udpclientc; ++i) {
      if (udpclientv[i].s == -1)
         continue;

      if (sockaddrareeq(addr, &udpclientv[i].laddr))
         return &udpclientv[i];
   }

   return NULL;
}

static struct udpclient *
udpclientofclientaddr(addr, udpclientc, udpclientv)
   const struct sockaddr *addr;
   const size_t udpclientc;
   struct udpclient *udpclientv;
{
   size_t i;

   for (i = 0; i < udpclientc; ++i) {
      if (udpclientv[i].s == -1)
         continue;

      if (sockaddrareeq(addr, &udpclientv[i].client))
         return &udpclientv[i];
   }

   return NULL;
}

static struct udpclient *
addudpclient(client, clientc, maxclientc, clientv)
   const struct udpclient *client;
   size_t *clientc;
   size_t *maxclientc;
   struct udpclient **clientv;
{
   const char *function = "addudpclient()";
   char laddr[MAXSOCKADDRSTRING], raddr[MAXSOCKADDRSTRING];

   slog(LOG_DEBUG,
        "%s: adding udp client for socket %d, clientaddr %s, bound addr %s",
        function, client->s,
        sockaddr2string(&client->raddr, raddr, sizeof(raddr)),
        sockaddr2string(&client->laddr, laddr, sizeof(laddr)));

   if (*clientc >= *maxclientc) {
      struct udpclient *p;

      if ((p = realloc(*clientv, (*maxclientc + UDP_MEMBLOCK) * sizeof(*p)))
      == NULL) {
         swarn("%s: failed to allocate memory for new udp client", function);
         return NULL;
      }

      *clientv     = p;
      *maxclientc += UDP_MEMBLOCK;

      slog(LOG_DEBUG, "%s: reallocated memory for udp clients, have memory "
                      "for %lu clients now",
                      function, (unsigned long)*maxclientc);

   }

   (*clientv)[*clientc] = *client;
   return &(*clientv)[(*clientc)++];
}

static int
removeudpclient(s, clientc, clientv)
   const int s;
   size_t *clientc;
   struct udpclient *clientv;
{
   const char *function = "removeudpclient()";
   size_t i;

   for (i = 1; i < *clientc; ++i)
      if (clientv[i].s == s) {
         close(clientv[i].s);
         ++freefds;

         slog(LOG_DEBUG, "%s: removing client %s on socket %d, free fds: %d",
                         function,
                         sockaddr2string(&clientv[i].raddr, NULL, 0), s,
                         freefds);

         SHMEM_UNUSE(&clientv[i].crule,
                     &(TOIN(&clientv[i].raddr)->sin_addr),
                     sockscf.shmemfd);

         memmove(&clientv[i],
                 &clientv[i + 1],
                 sizeof(*clientv) * (*clientc - (i + 1)));
         --(*clientc);

         return 0;
      }

   /* NOTREACHED */
   SERRX(0);
}

static rawsocketstatus_t
rawsocket_recv(s)
   const int s;
{
   const char *function = "rawsocket_recv()";
   struct icmp *icmp;
   struct ip *ip;
   struct udphdr *udp;
   struct sockaddr addr;
   socklen_t addrlen;
   ssize_t r;
   size_t i;
   char packet[MAX_ICMPLEN], fromstr[16];

   addrlen = sizeof(addr);
   if ((r = recvfrom(s, packet, sizeof(packet), 0, &addr, &addrlen)) == -1) {
      swarn("%s: recvfrom() on raw socket %d failed", function, s);
      return RAWSOCKET_NOP;
   }

   if (r < MIN_ICMPLEN)    {
      swarn("%s: short read on recvfrom() from raw socket: %ld/%lu",
            function, (long)r, (unsigned long)MIN_ICMPLEN);

      return RAWSOCKET_NOP;
   }

   ip   = (struct ip *)packet;
   icmp = (struct icmp *)(packet + (ip->ip_hl << 2));

   if (r < (ip->ip_hl << 2)) {
      swarn("%s: strange ... kernel says ip hl is %d, but packet size is %ld",
      function, ip->ip_hl << 2, (long)r);

      return RAWSOCKET_NOP;
   }

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: received raw packet from %s, type %d/%d, length %ld",
                      function,
                      inet_ntop(AF_INET, &(TOIN(&addr)->sin_addr), fromstr,
                                sizeof(fromstr)),
                      icmp->icmp_type, icmp->icmp_code, (long)r);

   if (icmp->icmp_type != ICMP_UNREACH)
      return RAWSOCKET_NOP;

   /* ip-packet the icmp error is in reply to. */
   ip = (struct ip *)(icmp->icmp_data);

   if (ip->ip_p != IPPROTO_UDP)
      return RAWSOCKET_NOP;

   udp = (struct udphdr *)(icmp->icmp_data + (ip->ip_hl << 2));

   TOIN(&addr)->sin_addr = ip->ip_src;
   TOIN(&addr)->sin_port = udp->uh_sport;

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: icmp packet is in response to udp packet to %s",
      function, sockaddr2string(&addr, NULL, 0));

   /*
    * Now we need to go through all io-objects with udp clients and
    * see if the address matches a client there.
    */
   for (i = 0; i < ioc; ++i) {
      const struct udpclient *client;

      if (!iov[i].allocated)
         continue;

      if (iov[i].state.protocol != SOCKS_UDP)
         continue;

      if ((client = udpclientofladdr(&addr, iov[i].dstc, iov[i].dstv)) != NULL){
         slog(LOG_DEBUG, "%s: removing client associated with %s from iov #%lu",
         function, sockaddr2string(&addr, NULL, 0), (unsigned long)i);

         SYNC_UDPDST(&iov[i].dst, client);
         delete_io(-1 /* nothing to ack */, &iov[i], client->s, IO_CLOSE);

         return RAWSOCKET_IO_DELETED;
      }
   }

   return RAWSOCKET_NOP;
}

#elif COVENANT /* !BAREFOOT */

int
recv_resentclient(s, client)
   int s;
   struct sockd_client_t *client;
{
   const char *function = "recv_resentclient()";
   struct iovec iov[2];
   struct msghdr msg;
   int ioc, fdexpect, fdreceived, r;
   CMSG_AALLOC(cmsg, sizeof(int));

   ioc = 0;
   bzero(iov, sizeof(iov));
   iov[ioc].iov_base = client;
   iov[ioc].iov_len  = sizeof(*client);
   ++ioc;

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iov;
   msg.msg_iovlen  = ioc;
   msg.msg_name    = NULL;
   msg.msg_namelen = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));

   if ((r = recvmsgn(s, &msg, 0)) < (ssize_t)sizeof(*client)) {
      switch (r) {
         case -1:
            swarn("%s: recvmsg() failed", function);
            break;

         case 0:
            slog(LOG_DEBUG, "%s: recvmsg(): other side closed connection",
            function);
            break;

         default:
            swarnx("%s: recvmsg(): unexpected short read: %d/%ld",
            function, r, (long)sizeof(*client));
      }

      return -1;
   }

   if (socks_msghaserrors(function, &msg))
      return -1;

   r       -= sizeof(*client);
   fdexpect = 1; /* client. */

   CMSG_VERIFY_RCPTLEN(msg, sizeof(int) * fdexpect);

   fdreceived = 0;
   if (fdexpect > 0) {
      SASSERTX(fdexpect == 1);
      CMSG_GETOBJECT(client->s, cmsg, sizeof(client->s) * fdreceived++);

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: received socket %d (%s) ...",
         function, client->s, socket2string(client->s, NULL, 0));
   }

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: received %d descriptors for client",
                      function, fdreceived);

   return 0;
}
#endif /* COVENANT */
