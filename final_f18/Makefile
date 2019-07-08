CC = g++ -g

CC_DEBUG = @$(CC) -std=c++11 -Wreturn-type
CC_RELEASE = @$(CC) -std=c++11 -O3 -DNDEBUG

G_SRC = src/*.cpp *.cpp

# need libpng to build
#
G_INC = -Iinclude -Iapps -I/usr/local/include -L/usr/local/lib

all: image

image : $(G_SRC) apps/image.cpp apps/image_recs.cpp
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/image.cpp apps/image_recs.cpp -o image

tests : $(G_SRC) apps/tests.cpp apps/tests_recs.cpp
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/tests.cpp apps/tests_recs.cpp -o tests

bench : $(G_SRC) apps/bench.cpp apps/bench_recs.cpp apps/GTime.cpp
	$(CC_RELEASE) $(G_INC) $(G_SRC) apps/GTime.cpp apps/bench.cpp apps/bench_recs.cpp -o bench

DRAW_SRC = apps/draw.cpp apps/GWindow.cpp apps/GTime.cpp
draw: $(DRAW_SRC) $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(DRAW_SRC) -lSDL2 -o draw

PAINT_SRC = apps/paint.cpp apps/GWindow.cpp apps/GTime.cpp
paint: $(PAINT_SRC) $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(PAINT_SRC) -lSDL2 -o paint

VIEWER_SRC = apps/viewer.cpp apps/GWindow.cpp apps/GTime.cpp apps/image_recs.cpp
viewer: $(VIEWER_SRC) $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(VIEWER_SRC) -lSDL2 -o viewer


BOUNCE_SRC = apps/bounce.cpp apps/GWindow.cpp apps/GTime.cpp
bounce: $(BOUNCE_SRC) $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(BOUNCE_SRC) -lSDL2 -o bounce


clean:
	@rm -rf image draw paint viewer bounce bench tests *.png *.dSYM

