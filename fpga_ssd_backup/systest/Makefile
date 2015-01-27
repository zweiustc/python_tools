MAKE        	 := make
SOURCE           := src
OUTOUT_TO	     := output/


.PHONY : all
all :	
	$(MAKE) -C src
	-mv src/output/ output_func/
	$(MAKE) -C src/pressure
	-mv src/pressure/output/ output_pres/
	$(MAKE) -C src/performance
	-mv src/performance/output/ output_perf/

.PHONY : clean
clean :
	$(MAKE) -C src clean
	$(MAKE) -C src/pressure clean
	$(MAKE) -C src/performance  clean
	-rm -rf output
	-rm -rf logs
	
