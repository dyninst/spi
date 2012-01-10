source ../../make.config

mkdir -p mutatee
cd mutatee
MUTATEE_DIR=`pwd`
cd ..
echo $MUTATEE_DIR

cd ../src
mkdir -p mutatee
cd mutatee
pwd

#--------------------------------------------
# coreutils
#--------------------------------------------

COREUTILS=coreutils-8.1
if [ ! -d $COREUTILS ]; then
  wget http://ftp.gnu.org/gnu/coreutils/$COREUTILS.tar.gz
  tar -xf $COREUTILS.tar.gz
fi

cd $COREUTILS
./configure --prefix=$MUTATEE_DIR
make install -j 2