build:
	gcc process_generator.c -o process_generator.out `pkg-config --cflags --libs gtk+-3.0`
	gcc clk.c -o clk.out `pkg-config --cflags --libs gtk+-3.0`
	gcc scheduler.c -o scheduler.out -lm `pkg-config --cflags --libs gtk+-3.0`
	gcc process.c -o process.out `pkg-config --cflags --libs gtk+-3.0`
	gcc test_generator.c -o test_generator.out `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out
