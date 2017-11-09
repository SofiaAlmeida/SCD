.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

compilador:=g++

pcl: prodcons-LIFO_exe
	./$<

pcf: prodcons-FIFO_exe
	./$<

fu: fumadores_exe
	./$<

fu2: fumadores2_exe
	./$<

%_exe: %.cpp
		$(compilador) -std=c++11 -I. -o $@ $< Semaphore.cpp -lpthread

clean:
	rm -f *_exe
