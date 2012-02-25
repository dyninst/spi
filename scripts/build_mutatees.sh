source ../config.mk

TMPDIR=$SP_DIR/$PLATFORM/tmp
BINDIR=$SP_DIR/$PLATFORM/test_mutatee
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