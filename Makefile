all:
	cd $(PLATFORM); make
	cd test/Agent/$(PLATFORM); make
	cd test/Injector; make

clean:
	cd $(PLATFORM); make clean
	cd test/Agent/$(PLATFORM); make clean
	cd test/Injector; make clean
