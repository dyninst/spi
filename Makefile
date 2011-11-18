include make.config
all:
	cd $(PLATFORM); sh build.sh
	cd test/Agent/$(PLATFORM); sh build.sh
	cd test/Injector; sh build.sh

clean:
	cd $(PLATFORM); make clean
	cd test/Agent/$(PLATFORM); make clean
	cd test/Injector; make clean
