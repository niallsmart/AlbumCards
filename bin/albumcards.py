#!/usr/bin/env python2.7

import sys
import os
import subprocess
import signal
import atexit
import re
import ConfigParser

from spotify_web.spotify import SpotifyAPI
from sys import stderr

def log(message):
	sys.stderr.write("%s: %s\n" % (os.path.basename(__file__), message))

class AlbumCards:

	def kill_player(self):
		if self.mpg123:
			self.mpg123.send_signal(signal.SIGINT)	# KILL/QUIT hangs the RPi
			self.mpg123.wait()
			self.mpg123 = None

	def kill_poller(self):
		if self.poller:
			self.poller.send_signal(signal.SIGINT)	# KILL/QUIT hangs the RPi
			self.poller.wait()
			self.poller = None

	def play_stream(self, uri):
		self.kill_player()
		self.mpg123 = subprocess.Popen(["mpg123", "-q", uri])

	def play_track_uri(self, track_uri):
		log("fetching metadata for %s" % track_uri)
		track = self.sp.metadata_request(track_uri)
		log("fetching playback URI for %s" % track.name)
		self.sp.track_uri(track, lambda sp, result: self.play_stream(result["uri"]))

	def play_tag(self, tag):
		if self.now_playing == tag:
			return
		self.now_playing = tag
		try:
			track_uri = config.get("tags", tag)
			self.play_track_uri(track_uri)
			self.now_playing = tag
		except ConfigParser.Error:
			print "unknown tag: %s" % (tag)

	def poll_nfc(self):
		log("starting nfc-poll")
		self.poller = subprocess.Popen(self.config.defaults().get("poller"), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		log("spawned nfc-poll as process %d" % self.poller.pid)
		while True:
			line = self.poller.stdout.readline()
			if line == None or line == "": break
			m = re.search('UID.*:(.*)', line)
			if m:
				self.play_tag(m.group(1).replace(' ', ''))
		self.poller.wait()
		log("nfc-poll exited with %s" % self.poller.returncode)

	def login_callback(self, sp, logged_in):
		if not logged_in:
			print "%s: error logging in" % sys.argv[0]
			sys.exit(1)

	def __init__(self, config): 
		self.mpg123 = None
		self.poller = None
		self.config = config
		self.now_playing = None
		atexit.register(lambda: self.kill_player())
		atexit.register(lambda: self.kill_poller())

	def start(self):
		log("connecting...")
		#login_callback(None, True)
		self.sp = SpotifyAPI(lambda sp, logged_in: self.login_callback(sp, logged_in))
		self.sp.connect(config.defaults().get("username"), config.defaults().get("password"))
		self.poll_nfc()
		self.sp.disconnect()

def makePath(*parts):
	parts = (os.path.dirname(__file__), "..") + parts
	return os.path.join(*parts)

config = ConfigParser.RawConfigParser({
	"poller": makePath("bin", "nfc-poll")
})
config.read(makePath("conf", "albumcards.conf"))

ac = AlbumCards(config)
ac.start()
