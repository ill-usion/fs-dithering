dither: dither.c
	cc dither.c -o dither -I./include -lm


.PHONY: clean
clean:
	rm dither
