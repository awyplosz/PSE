# TP5 : Fichier Makefile

EXE = client serveur

all: ${EXE}

${EXE): ${PSE_PROJECT}

clean:
	rm -f *.o *~ ${EXE} journal.log


