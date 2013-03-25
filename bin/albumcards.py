#!/usr/bin/env python

import sys
import os
import subprocess
from spotify_web.spotify import SpotifyAPI
from sys import stderr

def log(message):
	sys.stderr.write("%s: %s\n" % (os.path.basename(sys.argv[0]), message))

def playback_uri_callback(sp, result):
	log("found URI")
	print result["uri"]

#	sp.disconnect()

def fetch_playback_uri(track_uri, cb):
	track = sp.metadata_request(track_uri)
	log("fetching playback URI for %s" % track.name)
	sp.track_uri(track, cb)

def poll_nfc():
	poller = subprocess.POpen(os.path.join(baseDir, "nfc-poll"), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	log("spawned nfc-poll as process %d", poller.pid)
	while True:
		line = poller.stdout.readline()
		log(line)
	log("nfc-poll exited with %d", poller.returncode)

def login_callback(sp, logged_in):
	if logged_in:
		poll_nfc()
		#fetch_playback_uri("spotify:track:5yEPxDjbbzUzyauGtnmVEC", playback_uri_callback)
	else:
		print "%s: error logging in" % sys.argv[0]

baseDir = os.path.dirname(sys.argv[0])

log("loaded from %s" % baseDir)

if len(sys.argv) < 3:
	print "Usage: %s <username> <password>" % sys.argv[0]
else:
	log("connecting...")
	sp = SpotifyAPI(login_callback)
	sp.connect(sys.argv[1], sys.argv[2])
