#!/bin/sh

BACKUPHOST=ananke.laggygamerz.com
export LC_ALL=C LANG=C
cd $HOME/dev/facetracknoir-posix || exit 1
touch ./last-snapshot.rev
read line < ./last-snapshot.rev
LASTREV="$line"
CURREV="$(git rev-parse HEAD)"

if test "$CURREV" != "$LASTREV"; then
	echo $CURREV > ./last-snapshot.rev
	cd $HOME/dev/facetracknoir-posix-build-mingw32 || exit 1
	cmake . || exit 1
	nice -n 20 make all install || exit 1
	FNAME="ftnoir-posix-$(date +%Y%m%d-%H_%M_%S)"-"$CURREV".7z
	7z -y a -t7z -m0=lzma -mx=9 -mfb=128 -md=8m -ms=on "$FNAME" install || exit 1
	ssh -4o BatchMode=yes "$USER"@"$BACKUPHOST" find /var/www/ftnoir/ -type f -mtime 60 -delete
	scp -4vo BatchMode=yes "$FNAME" "$USER"@"$BACKUPHOST":/var/www/ftnoir/
	rm *.7z
fi
