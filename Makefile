com:
	gcc -o smf sem_multi_threads.c -lpthread
	./smf datos.dat
	rm smf

compilePing:
	gcc -o smf semafobroping.c -lpthread
	./smf datos.dat

compile1:
	gcc -o pcthreads semafobro.c -lpthread -lrt
	./pcthreads datos.dat

compile:
	gcc -o consumidor Consumidor.c
	gcc -o productor Productor.c
	gnome-terminal --command=./consumidor
	./productor datos.dat

clean:
	rm productor
	rm consumidor