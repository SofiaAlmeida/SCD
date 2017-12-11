.SUFFIXES:
.PHONY:    pc,pc2,pcm,fp,f,fi,fc,clean

compilador := mpicxx -std=c++11
flagsc     := -g -Wall

pcm: prodcons-mu_exe
	mpirun -np  10 ./$<

fi: filosofos-interb_exe
	mpirun -np  10 ./$<

f: filosofos_exe
	mpirun -np  10 ./$<

fc: filosofos-cam_exe
	mpirun -np  11 ./$<

%_exe: %.cpp
	$(compilador) $(flagsc) -o $@ $<

clean:
	rm -rf *_exe *.dSYM
