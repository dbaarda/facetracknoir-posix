#!/bin/sh

BACKUPHOST=ananke.laggygamerz.com
export LC_ALL=C LANG=C
cd $HOME/dev/facetracknoir-posix || exit 1
touch ./last-snapshot.rev
read line < ./last-snapshot.rev
LASTREV="$line"
CURREV="$(git rev-parse HEAD)"

export WINEPREFIX=$HOME/dev/msvc WINEDEBUG=-all

wineserver -k

if test "$CURREV" != "$LASTREV"; then
	echo $CURREV > ./last-snapshot.rev
	cd $HOME/dev/msvc/drive_c/build/ftnoir-posix || exit 1
	wine $WINEPREFIX/drive_c/Program\ Files/Microsoft\ Visual\ Studio\ 9.0/Common7/IDE/mspdbsrv.exe -start -spawn -shutdowntime -1 &
	sleep 0.5
	rm -rf ./install/
	wine cmd /k $HOME/dev/facetracknoir-posix/compile-msvc.bat
	pushd $HOME/dev/msvc/drive_c/build/ftnoir-faceapi || exit 1
	rm -rf ./install/
	wine cmd /k $HOME/dev/facetracknoir-posix/compile-msvc80.bat
	rsync -rav ./install/faceapi $HOME/dev/msvc/drive_c/build/ftnoir-posix/install || exit 1
	popd || exit 1
	FNAME="ftnoir-posix-$(date +%Y%m%d-%H_%M_%S)"-"$CURREV".7z
	7z -y a -t7z -m0=lzma -mx=9 -mfb=128 -md=8m -ms=on "$FNAME" install || exit 1
	ssh -4o BatchMode=yes "$USER"@"$BACKUPHOST" find /var/www/ftnoir/ -type f -mtime 60 -delete
	scp -4vo BatchMode=yes "$FNAME" "$USER"@"$BACKUPHOST":/var/www/ftnoir/
	rm *.7z
fi
