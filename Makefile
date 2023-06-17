# TP5 : Fichier Makefile

EXE = client server

all: ${EXE}

${EXE): ${PSE_PROJECT}

clean:
	rm -f *.o *~ ${EXE} journal.log


