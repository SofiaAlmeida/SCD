.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

compilador:=g++

pcl: prodcons_SU_LIFO_exe
	./$<

pcf: prodcons_SU_FIFO_exe
	./$<

%_exe: %.cpp
		$(compilador) -std=c++11 -I. -o $@ $< HoareMonitor.cpp -lpthread

clean:
	rm -f *_exe
