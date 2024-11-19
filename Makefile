all: cfs

cfs: src/*.c
	gcc src/*.c -lm -o cfs

runcmd: cfs
	./cfs C 0 0 exponential 200 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 300 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 400 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 500 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 600 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 700 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 900 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 1000 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 1100 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 1200 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 1300 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 1400 100 1500 exponential 800 100 1500 200 200 1
	
	./cfs C 0 0 exponential 800 100 1500 exponential 200 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 300 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 400 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 500 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 600 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 700 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 800 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 900 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 1000 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 1100 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 1200 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 1300 100 1500 200 200 1
	./cfs C 0 0 exponential 800 100 1500 exponential 1400 100 1500 200 200 1

runfile: cfs
	./cfs F 5 12 2 infile.txt out.txt

leak-runcmd: cfs
	valgrind --tool=memcheck --leak-check=full ./cfs C -10 10 uniform 0 100 1000 exponential 600 100 1500 10 200 2 out.txt

leak-runfile:
	valgrind --tool=memcheck --leak-check=full ./cfs F 20 5 2 infile.txt out.txt

clean:
	rm cfs out.txt