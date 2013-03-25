#!/bin/bash

export LIBNFC_LOG_LEVEL=3
export LIBNFC_LOG_FILE=/dev/null

SP_USER=niallsmart
SP_PASSWORD=rnwk

bin=`dirname $0`

PlayAlbum() {
	echo "playing $1" >&2
	case $1 in
	ab2dd063)
		track='spotify:track:5yEPxDjbbzUzyauGtnmVEC'
		;;
	04a31c52bc2b80)
		track='spotify:track:1yFORmqWN4bKhboS8inzzv'
		;;
	*)
		echo "unknown tag: $1" >&2
		return
		;;
	esac
	echo "fetching track URI" >&2
	uri=`$bin/sp-track-uri $SP_USER $SP_PASSWORD $track`
	killall mpg123 2>/dev/null
	killall -9 mpg123 2>/dev/null
	echo "spawning mpg123" >&2
	mpg123 -q "$uri" &
}

PollForTag() {

	lastuid=""

	$bin/nfc-poll 2>/$LIBNFC_LOG_FILE | sed -nue '/UID/{s/.*:\(.*\)/\1/;s/ *//gp}' | while read uid; do
		if [ "$lastuid" != "$uid" ]; then
			PlayAlbum $uid
		fi
		lastuid=$uid
	done

	exit $?
}

PollForTag

