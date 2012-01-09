source ../../make.config

mkdir -p mutatee
cd mutatee
MUTATEE_DIR=`pwd`
cd ..
echo $MUTATEE_DIR

cd ../src
mkdir -p mutatee
cd mutatee

#--------------------------------------------
# autoconf
#--------------------------------------------
if [ ! -d autoconf ]; then
		git clone --depth=1 git://git.sv.gnu.org/autoconf.git
fi


#--------------------------------------------
# coreutils
#--------------------------------------------

# Checkout
if [ ! -d coreutils ]; then
		git clone git://git.sv.gnu.org/coreutils
fi
