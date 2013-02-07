#!/bin/sh

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
	7z -y a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on ftnoir-posix-$(date +%Y%m%d)-"$CURREV".7z install || exit 1
	scp -4vo BatchMode=yes ftnoir-posix-$(date +%Y%m%d)-"$CURREV".7z ananke.laggygamerz.com:/var/www/ftnoir/
	rm *.7z
fi
