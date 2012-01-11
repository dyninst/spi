source ../make.config

copy_makefile()
{
  LIBDIR=$1
	cp -f $LIBDIR/make.module.tmpl $DYNINST_DIR/$LIBDIR
	cp -f $LIBDIR/$PLATFORM/Makefile $DYNINST_DIR/$LIBDIR/$PLATFORM
	echo "Applying patch for $LIBDIR"
}

copy_makefile "common"
copy_makefile "instructionAPI"
copy_makefile "parseAPI"
copy_makefile "patchAPI"
copy_makefile "proccontrol"
copy_makefile "stackwalk"
copy_makefile "symtabAPI"