
INC_BULLET=-I/usr/include/bullet
INC_OSG=-I/usr/include/osg
LD_OSG=-losg -losgViewer -losgSim -losgDB -losgGA -losgShadow
LD_BULLET=-L /home/jml/bullet3/src/BulletDynamics -l BulletDynamics -l BulletCollision -l LinearMath

CFLAGS=-std=c++11 -Wall -O2 -Wno-uninitialized

all: carZ carY ball

carZ: carZ.o btosg.o
	cc -O2 -o $@ $^ ${LD_BULLET} ${LD_OSG} -l stdc++ -lm

carY: carY.o btosg.o
	cc -O2 -o $@ $^ ${LD_BULLET} ${LD_OSG} -l stdc++ -lm

ball: ball.o btosg.o
	cc -O2 -o $@ $^ ${LD_BULLET} ${LD_OSG} -l stdc++ -lm
	
carZ.o: car.cpp btosg.h btosgVehicle.h
	g++ ${CFLAGS}  -c ${INC_BULLET} ${INC_OSG} $< -o $@

carY.o: car.cpp btosg.h btosgVehicle.h
	g++ ${CFLAGS}  -c ${INC_BULLET} ${INC_OSG} $< -o $@ -D_UP_=0,1,0

ball.o: ball.cpp btosg.h 
	g++ ${CFLAGS} -c ${INC_BULLET} ${INC_OSG} $<

#-DBTOSG_SHADOW $<

btosg.o: btosg.cpp btosg.h
	g++ ${CFLAGS} -c ${INC_BULLET} ${INC_OSG} $<
	
clean:
	rm -f *.o 

push: *.cpp *.h Makefile README.md
	git add $^
	git commit -m "update"
	git push
