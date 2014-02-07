/* xor.c -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * xor takes a list of filenames, or - for stdin, xor's them all together,
 * and sends the result to stdout.
 * The first file listed controls the length.  It is an error for the
 * first file to be longer than any of the others.
 * Useful for onetime pad encryption, or generation if you have more than
 * one random source.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

void usage() {
  fprintf(stderr, "xor [-s max_size] file1 file2 ... \n");
}

int main(int argc, char *argv[]) {
  long long max_size = -1;
  int *fds;
  int i;
  unsigned char buff[4096];
  unsigned char tmp[4096];
  int opt;

  while ((opt = getopt(argc, argv, "s:")) != -1 ) {
    switch (opt) {
    case 's':
      max_size = atoll(optarg);
      if (max_size > 0) {
	break;
      }
      /* fall through */

    case '?':
    default:
      usage();
      return 1;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc < 2) {
    usage();
    return 7;
  }

  fds = (int *)alloca(sizeof(*fds) * argc);
  if (fds == NULL) {
    perror ("alloca can't fail");
    return 2;
  }

  for (i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "-") == 0) {
      fds[i] = 0; /* stdin */
    } else {
      fds[i] = open(argv[i], O_RDONLY);
      if (fds[i] < 0) {
	perror(argv[i]);
	return 3;
      }
    }
  }

  while (max_size == -1 || max_size > 0) {
    int j;
    int red;
    int first_red;
    if (max_size == -1) {
      first_red = sizeof(buff);
    } else {
      first_red = (max_size > sizeof(buff))?sizeof(buff):max_size;
    }
    first_red = read(fds[0], buff, first_red);
    if (first_red == 0) {
      break;
    }
    if (first_red < 0) {
      perror (argv[0]);
      return 4;
    }

    for (i = 1; i < argc; ++i) {
      red = read(fds[i], tmp, first_red);
      if (red < 0) {
	perror(argv[i]);
	return 5;
      }
      /* 
       * XXXXX FIXME, some systems can return
       * a short read if the syscall is interrupted.
       */
      if (red != first_red) {
	fprintf(stderr, "%s file too short.\n", argv[i]);
	return 6;
      }

      for (j = 0; j < first_red; ++j) {
	buff[j] ^= tmp[j];
      }
    }

    if (write(1, buff, first_red) != first_red) {
      perror("output");
      return 7;
    }

    if (max_size != -1) {
      max_size = max_size - first_red;
    }
  }

  for (i = 0; i < argc; ++i) {
    close(fds[i]);
  }

  return 0;
}
