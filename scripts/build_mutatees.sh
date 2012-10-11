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
# gcc
#--------------------------------------------
GCCPATH=$SP_DIR/scripts/test_data/$PLATFORM/test_exes/cc1.exe
GCCLIBPATH=$SP_DIR/scripts/test_data/$PLATFORM/test_libs/*

if [ ! -f $BINDIR/cc1.exe ]; then
  cp -f $GCCPATH $BINDIR/cc1.exe
  cp -f $GCCLIBPATH $SP_DIR/$PLATFORM/tmp/lib
fi

mkdir -p $DATADIR
if [ ! -f $DATADIR/200.i ]; then
  cp $SRC_DATADIR/200.i $DATADIR/200.i
fi