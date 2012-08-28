# Parameters.

# Compiler flags.
LIBS = -lm 
CFLAGS	= -g -c 
RM = /bin/rm
CC = c++

PROGRAM = laminasID
LA-2D = LA-2D

SOURCES.C = bmp_io.C \
	util.C \
	laminasID.C 

SOURCES.H = bmp_io.H gray_2d.H variables.H

OBJECTS = bmp_io.o util.o laminasID.o 

all: $(PROGRAM) $(LA-2D) deploy clean

$(PROGRAM): $(SOURCES.C) $(SOURCE.H) $(OBJECTS)
	$(CC) -o  $(PROGRAM) $(OBJECTS) 

$(LA-2D): LA-2D.C
	$(CC) -o $(LA-2D) LA-2D.C 

deploy: 
	/bin/cp $(LA-2D) examples
	/bin/cp ${PROGRAM} examples

clean:
	$(RM) $(OBJECTS) 

bmp_io.o: $(SOURCES.H) bmp_io.C
	c++ -c  bmp_io.C

util.o: $(SOURCES.H) util.C
	c++ -c  util.C

laminasID.o: $(SOURCES.H) laminasID.C
	c++ -c laminasID.C
