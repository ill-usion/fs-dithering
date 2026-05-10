dither: dither.c
	cc dither.c -o dither -I./include -lm -O2


.PHONY: clean
clean:
	rm dither
