#!/bin/bash

GREEN='\033[0;32m'
NC='\033[0m'

if ( ! test $# -eq 1)
then
	#Imprimimos unha mensaxe de erro en caso contrario
	echo "Parametros: (1) expoñente para tamano vectores"
	#Saimos do script indicando que tivo lugar un erro
	exit 1
fi

if ( ! test $1 -gt 1)
then
    #Imprimimos unha mensaxe de erro en caso contrario
    echo "O expoñente debe ser maior que un para que todas as versions funcionen"
    #Saimos do script indicando que tivo lugar un erro
    exit 1
fi

#compilacion
gcc quaternionO3.c -lm  -mavx -O3 rutinas_clock.c rutinas_clock.h -o quaternionO3double
gcc quaternion2.c -lm -O0  rutinas_clock.c rutinas_clock.h -o quaternion2
gcc quaternion31.c -mavx -mavx2 -mfma -lm -O0  rutinas_clock.c rutinas_clock.h -o quaternion31
gcc quaternion32.c -mavx -mavx2 -mfma -lm -O0  rutinas_clock.c rutinas_clock.h -o quaternion32
gcc quaternion4.c -lm -fopenmp -O0  rutinas_clock.c rutinas_clock.h -o quaternion4

#execucion
echo -e "${GREEN}Secuencial -03 -mavx sen estrutura Quaternion:${NC}"
./quaternionO3double $1
echo -e "${GREEN}Secuencial optimizado:${NC}"
./quaternion2 $1
echo -e "${GREEN}Vectorizado por operacions:${NC}"
./quaternion31 $1
echo -e "${GREEN}Vectorizado por iteracions:${NC}"
./quaternion32 $1
echo -e "${GREEN}Paralelizado OMP 1 fio:${NC}"
./quaternion4 $1 1
echo -e "${GREEN}Paralelizado OMP 2 fio:${NC}"
./quaternion4 $1 2
echo -e "${GREEN}Paralelizado OMP 4 fio:${NC}"
./quaternion4 $1 4

#eliminamos os executables
rm quaternionO3double
rm quaternion2
rm quaternion31
rm quaternion32
rm quaternion4
