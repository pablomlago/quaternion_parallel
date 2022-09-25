#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pmmintrin.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include "rutinas_clock.h"

//estructura quaternion formada por 4 doubles
struct Quaternion{
  double w;
  double x;
  double y;
  double z;
};
typedef struct Quaternion quaternion;

//prototipos de funcions
//static inline indica ao compilador que cambie as chamadas ás funcións polo propio codigo das funcions
//__atribute__((always_inline)) indica ao compilador gcc que sempre debe facer caso da intruccion inline
static inline quaternion sumQuaternion(quaternion a, quaternion b) __attribute__((always_inline));
static inline quaternion multQuaternion(quaternion a, quaternion b) __attribute__((always_inline));
static inline quaternion cuadradoYSumaQuaternion(quaternion a, quaternion b) __attribute__((always_inline));

//non usan inline porque non son parte dos calculos
int printQuaternion(quaternion a);
void randQuaternion(quaternion* a, int N);

int main(int argc, char** argv){
  register int i = 0;//variable para iterar nos bucles
    
  //vectores de quaternions
  //__restrict__ indica ao compilador que as direccions apuntadas van ser apuntadas unicamente por dito punteiro
  quaternion*__restrict__ a = NULL;
  quaternion*__restrict__ b = NULL;
  quaternion*__restrict__ dp = NULL;

  double ck;//variable para contar os ciclos

  int N = 0;//tamano de a e b
  int tid = 0;//usarase na  seccion paralela
  int numFios = 0;//numero de fios do programa recibese por paramtero
  int p = 0;//usase no calculo de N

  size_t tamano = 0;//usase para evitar moitas chamadas a sizeof
    
    
  //comprobamos que os parametros son validos
  if(argc == 3 && (p = atoi(argv[1])) > 0 && (numFios = atoi(argv[2])) > 0) {
      N = pow(10,p);//inicializamos N a 10 elevado a p
  } else {
    printf("El programa debe recibir un entero positivo e o numero de fios\n");
    return (EXIT_FAILURE);//abortase a execucion
  }

  srand(1);//fixamos a semilla para que o experimento sexa reproducible

  tamano = N*sizeof(quaternion);//inicializamos tamano

  //reserva de memoria aliñada para os quaternions
  if((a = (quaternion*)_mm_malloc(tamano,sizeof(quaternion))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  if((b = (quaternion*)_mm_malloc(tamano,sizeof(quaternion))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }
  //reservamos en dp memoria para numFios quaternions para que cada fio use unha posicion para realizar o seu calculo parcial e evitar carreiras criticas
  if((dp = (quaternion*)_mm_malloc(numFios*sizeof(quaternion),sizeof(quaternion))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  //inicializacion aleatoria de a e b
  randQuaternion(a,N);
  randQuaternion(b,N);

  mhz(1,1);//fixamos a frecuencia
  start_counter();//iniciamos o contador de ciclos

  //Realizamos as operacions indicadas
  memset(dp,0,numFios*sizeof(quaternion));//establecemos dp a 0

  //inicio da seccion paralela
  #pragma omp parallel private(i,tid) num_threads(numFios)
  {
    //inicializamos tid para cada fio
    tid = omp_get_thread_num();
    //bucle for paralelizado, cada fio escribe nunha posicion distinta de dp
    #pragma omp for
      for(i = 0; i < N; i+=5){//LOOP UNROLLING
        dp[tid] = cuadradoYSumaQuaternion(multQuaternion(a[i],b[i]),dp[tid]);
        dp[tid] = cuadradoYSumaQuaternion(multQuaternion(a[i+1],b[i+1]),dp[tid]);
        dp[tid] = cuadradoYSumaQuaternion(multQuaternion(a[i+2],b[i+2]),dp[tid]);
        dp[tid] = cuadradoYSumaQuaternion(multQuaternion(a[i+3],b[i+3]),dp[tid]);
        dp[tid] = cuadradoYSumaQuaternion(multQuaternion(a[i+4],b[i+4]),dp[tid]);
      }
  }
  //fin da rexion paralela
    
  //cuando todos los hilos han calculado su parte el hilo principal las suma
  //o resultado final de dp acumulase na primeira posicion de dp
  for(i = 1; i<numFios; i++) {
    dp[0] = sumQuaternion(dp[i],dp[0]);
  }

  ck=get_counter();//rematamos a medicion do tempo

  printf("Ciclos: %lf\n", ck);//imprimimos os ciclos
  printQuaternion(*dp);//imprimimos o resultado (posicion 0 de dp)

  //liberamos a memoria aliñada
  _mm_free(a);
  a = NULL;
  _mm_free(b);
  b = NULL;
  _mm_free(dp);
  dp = NULL;

  return (EXIT_SUCCESS);//rematamos con exito
}

//funcion para facer sumar de quaternions compoñente a compoñente
//inline se explica no prototipo
static inline quaternion sumQuaternion(quaternion a, quaternion b) {
  quaternion c;
  c.w = a.w+b.w;
  c.x = a.x+b.x;
  c.y = a.y+b.y;
  c.z = a.z+b.z;
  return c;
}
//funcion para facer a multiplicacion de quaternions
static inline quaternion multQuaternion(quaternion a, quaternion b) {
  quaternion c; //resultado
  c.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
  c.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
  c.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x;
  c.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w;
  return c;
}

//funcion para realizar o cadrado do quaternion a e sumarlle b
static inline quaternion cuadradoYSumaQuaternion(quaternion a, quaternion b) {
  quaternion c;//resultado
  c.w = a.w*a.w - a.x*a.x - a.y*a.y - a.z*a.z + b.w;
  c.x = 2.*a.w*a.x + b.x;
  c.y = 2.*a.w*a.y + b.y;
  c.z = 2.*a.w*a.z + b.z;
  return c;
}

//funcion para imprimir un quaternion con dous decimais
int printQuaternion(quaternion a) {
  return printf("%.2f + (%.2f)i + (%.2f)j + (%.2f)k\n", a.w,a.x,a.y,a.z);
}

//funcion para inicialiar quaternions aleatorios
void randQuaternion(quaternion* a, int N) {
  int i = 0;
  for(i=0;i<N;i++){
      a[i].w=(double)rand()/(rand()+1);
      a[i].x=(double)rand()/(rand()+1);
      a[i].y=(double)rand()/(rand()+1);
      a[i].z=(double)rand()/(rand()+1);
  }
}
