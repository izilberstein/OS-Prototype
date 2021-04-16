mykernel: shell.o interpreter.o kernel.o shellmemory.o ram.o pcb.o cpu.o memorymanager.o diskdriver.o
	gcc -o mykernel shell.o interpreter.o kernel.o shellmemory.o ram.o pcb.o cpu.o memorymanager.o diskdriver.o
shell.o: shell.c interpreter.h
	gcc -c shell.c
interpeter.o: interpreter.c shellmemory.h shell.h kernel.h diskdriver.h
	gcc -c interpreter.c
memorymanager.o: memorymanager.c ram.h pcb.h kernel.h
	gcc -c memorymanager.c
cpu.o: cpu.c ram.h interpreter.h
	gcc -c cpu.c
kernel.o: kernel.c pcb.h ram.h shell.h cpu.h memorymanager.h diskdriver.h
	gcc -c kernel.c
shellmemory.o: shellmemory.c
	gcc -c shellmemory.c
ram.o: ram.c
	gcc -c ram.c
pcb.o: pcb.c
	gcc -c pcb.c
diskdriver.o: diskdriver.c
	gcc -c diskdriver.c
