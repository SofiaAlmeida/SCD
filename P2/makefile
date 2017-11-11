.SUFFIXES:
.PHONY: pc
.PRECIOUS: %_exe

compilador:=g++

fu: fumadores_exe
	./$<

bd: barbero_durmiente_exe
	./$<

bd2: barbero_durmiente2_exe
	./$<

%_exe: %.cpp
		$(compilador) -std=c++11 -I. -o $@ $< HoareMonitor.cpp -lpthread

clean:
	rm -f *_exe
