compilePing:
	gcc -o smf semafobroping.c -lpthread
	./smf datos.dat
	rm smf

compileSem:
	gcc -o pcthreads semafobro.c -lpthread -lrt
	./pcthreads datos.dat
	rm pcthreads

compileMsgq:
	gcc -o consumidor Consumidor.c
	gcc -o productor Productor.c
	gnome-terminal --command=./consumidor
	./productor datos.dat

clean:
	rm productor
	rm consumidor