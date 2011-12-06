include make.config
all:
	cd $(PLATFORM); sh build.sh
	cd test/Agent/$(PLATFORM); make -j 4
	cd test/Injector; make -j 4

clean:
	cd $(PLATFORM); make clean
	cd test/Agent/$(PLATFORM); make clean
	cd test/Injector; make clean
