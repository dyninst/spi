source ../config.mk

TMPDIR=$SP_DIR/$PLATFORM/tmp
BINDIR=$SP_DIR/$PLATFORM/test_mutatee
DATADIR=$SP_DIR/$PLATFORM/test_data
SRC_DATADIR=$SP_DIR/scripts/test_data
mkdir -p $TMPDIR
cd $TMPDIR

#--------------------------------------------
# coreutils
#--------------------------------------------

COREUTILS=coreutils-8.1
if [ ! -d $COREUTILS ]; then
  wget http://ftp.gnu.org/gnu/coreutils/$COREUTILS.tar.gz
  tar -xf $COREUTILS.tar.gz
fi

if [ ! -f $BINDIR/ls.exe ]; then


		if [ ! -f $TMPDIR/bin/ls ]; then
				cd $COREUTILS
				./configure --prefix=$TMPDIR
				make install -j 2
		fi

		cp $TMPDIR/bin/ls $BINDIR/ls.exe
		cp $TMPDIR/bin/od $BINDIR/od.exe
		cp $TMPDIR/bin/sort $BINDIR/sort.exe
		cp $TMPDIR/bin/df $BINDIR/df.exe

fi


#--------------------------------------------
# IPC test: lighttpd and wget
#--------------------------------------------
cd $TMPDIR
LIGHTTPD=lighttpd-1.4.32
if [ ! -d $LIGHTTPD ]; then
  wget http://download.lighttpd.net/lighttpd/releases-1.4.x/$LIGHTTPD.tar.gz
  tar -xf $LIGHTTPD.tar.gz
fi

if [ ! -f $BINDIR/lighttpd.exe ]; then
		if [ ! -f $TMPDIR/sbin/lighttpd ]; then
				cd $LIGHTTPD
				./configure --prefix=$TMPDIR --without-bzip2
				make install -j 2
		fi
		cp $TMPDIR/sbin/lighttpd $BINDIR/lighttpd.exe
fi

WGET=wget-1.14
cd $TMPDIR
if [ ! -d $WGET ]; then
  wget http://ftp.gnu.org/gnu/wget/$WGET.tar.gz
  tar -xf $WGET.tar.gz
fi

if [ ! -f $BINDIR/wget.exe ]; then
		if [ ! -f $TMPDIR/bin/wget ]; then
				cd $WGET
				./configure --prefix=$TMPDIR  --without-ssl
				make install -j 2
		fi
		cp $TMPDIR/bin/wget $BINDIR/wget.exe
fi

cp -r $SRC_DATADIR/web $TMPDIR
cp $SRC_DATADIR/conf $TMPDIR
