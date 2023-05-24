CC=gcc

build: ising.c
	$(CC) -o x_ising ising.c -lgsl -lgslcblas -lm -O3

clean:
	rm -f x_ising mag frames/*.png output.gif

live: build 
	./x_ising 2 | gnuplot

movie: build
	./x_ising 0 | gnuplot
	ffmpeg -framerate 60 -i frames/frame%09d.png -vf "palettegen" -y palette.png
	ffmpeg -framerate 60 -i frames/frame%09d.png -i palette.png -lavfi "paletteuse" output.gif

magnetization: build
	./x_ising 1 | gnuplot
