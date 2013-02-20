#!/bin/sh

BACKUPHOST=ananke.laggygamerz.com
export LC_ALL=C LANG=C
cd $HOME/dev/facetracknoir-posix || exit 1
touch ./last-snapshot.rev
read LASTREV < ./last-snapshot.rev
CURREV="$(git rev-parse HEAD)"
TMP=/tmp
WINHOST=Administrator@172.18.2.69
WINCMD=/home/Administrator/build-ftnoir.sh
WINPATH=/C/build/ftnoir-posix-msvc2010/install.7z
WINSH=/mingw/msys/1.0/bin/sh
NEKO=/mingw/msys/1.0/bin/cat
ORIGIN="$(pwd)"
FNAME="ftnoir-posix-$(date +%Y%m%d-%H_%M_%S)"-"$CURREV".7z

ssh -o BatchMode=yes $WINHOST $WINSH $WINCMD || exit 1
ssh -o BatchMode=yes $WINHOST $WINSH -c \'$NEKO $WINPATH\' > $TMP/$FNAME || exit 1
scp -o BatchMode=yes $TMP/$FNAME $USER@$BACKUPHOST:/var/www/ftnoir/
rm -f $TMP/$FNAME
