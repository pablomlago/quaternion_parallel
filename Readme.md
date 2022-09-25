# Programación Multinúcleo e extensións SIMD

Indícase a continuación con que apartado se corresponde cada código

- Programa secuencial optimizado = quaternion2.c
- Programa vectorizado por operacións de quaternions = quaternion31.c
- Programa vectorizado por iteracións do bucle = quaternion32.c
- Programa paralelizado con OpenMP = quaternion4.c

- rutinas_clock.c = libreria creada coas rutinas de medida de ciclos proporcionadas polo profesor
- rutinas_clock.h = cabeceira de libreria anterior

- tester.sh = programa para facilitar a verificación do correcto funcionamento dos codigos

- quaterionO3.c = Tal e como explicamos no informe (antes de analizar os resultados do programa secuencial base con  -O3) realizaouse outro porgrama extra quaternionO3.c que se corresponde co porgrama secuencial base pero utilizando arrays de 4 doubles para representar os quaternions en lugar de unha estructura como no resto de programas.
