source ../make.config
export LD_LIBRARY_PATH

#make $1
make SP_RELEASE=1 -j 4 $1 $2 $3 $4
