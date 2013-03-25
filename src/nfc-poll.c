/*-
 * Copyright (C) 2013 Niall Smart
 * Copyright (C) 2011 Romain Tartiere
 * Copyright (C) 2010, 2011 Romuald Conty
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  1) Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  2 )Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Note that this license only applies on the examples, NFC library itself is under LGPL
 *
 */

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#include <nfc/nfc.h>
#include <nfc/nfc-types.h>

#define MAX_DEVICE_COUNT 16

static nfc_device *pnd = NULL;

static void stop_polling(int sig)
{
  (void) sig;
  if (pnd)
    nfc_abort_command(pnd);
  else
    exit(EXIT_FAILURE);
}

static void
print_usage(const char *progname)
{
  printf("usage: %s [-v]\n", progname);
  printf("  -v\t verbose display\n");
}

static void
nfclog(const char* fmt, ...)
{
  va_list argp;
  va_start(argp, fmt);
  fprintf(stderr, "nfc-poll: ");
  vfprintf(stderr, fmt, argp);
  fprintf(stderr, "\n");
  va_end(argp);
}

int
main(int argc, const char *argv[])
{
  bool verbose = false;

  signal(SIGINT, stop_polling);

  nfclog("using libnfc %s", nfc_version());

  if (argc != 1) {
    if ((argc == 2) && (0 == strcmp("-v", argv[1]))) {
      verbose = true;
    } else {
      print_usage(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  const uint8_t uiPollNr = 5;
  const uint8_t uiPeriod = 2;
  const nfc_modulation nmModulations[] = {
    { .nmt = NMT_ISO14443A, .nbr = NBR_106 },
    { .nmt = NMT_ISO14443B, .nbr = NBR_106 } /*
    { .nmt = NMT_FELICA, .nbr = NBR_212 },
    { .nmt = NMT_FELICA, .nbr = NBR_424 },
    { .nmt = NMT_JEWEL, .nbr = NBR_106 }, */
  };
  const size_t szModulations = sizeof(nmModulations) / sizeof(nmModulations[0]);

  nfc_target nt;
  int res = 0;
  int tries = 2000000000;
  //int tries = 3;

  nfc_context *context;
  nfc_init(&context);

  pnd = nfc_open(context, NULL);

  if (pnd == NULL) {
    nfclog("unable to open NFC device");
    exit(EXIT_FAILURE);
  }

  if (nfc_initiator_init(pnd) < 0) {
    nfc_perror(pnd, "nfc_initiator_init");
    exit(EXIT_FAILURE);
  }

  nfclog("opened %s", nfc_device_get_name(pnd));

  do { 
    nfclog("polling for %ld ms (%u pollings of %lu ms for %zd modulations)",
      (unsigned long) uiPollNr * szModulations * uiPeriod * 150,
      uiPollNr,
      (unsigned long) uiPeriod * 150,
      szModulations
    );
    res = nfc_initiator_poll_target(pnd, nmModulations, szModulations, uiPollNr, uiPeriod, &nt);

    if (res > 0) {
      char* buf;
      str_nfc_target(&buf, nt, verbose);
      printf("%s\n", buf);
      printf("going to flush\n");
      fflush(NULL);
      free(buf);
      sleep(1.0);
    }
  } while ( (res == NFC_ECHIP || res > 0) && --tries > 0);

  if (res < 0 && res != NFC_ECHIP) {
    nfc_perror(pnd, "nfc_initiator_poll_target");
    nfc_close(pnd);
    nfc_exit(context);
    exit(EXIT_FAILURE);
  }
  nfc_close(pnd);
  nfc_exit(context);
  exit(EXIT_SUCCESS);
}
