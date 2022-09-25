#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pmmintrin.h>
#include <math.h>
#include <string.h>
#include "rutinas_clock.h"

//Estructura quaternion formada por 4 doubles
struct Quaternion{
  double w;
  double x;
  double y;
  double z;
};
typedef struct Quaternion quaternion;

//Prototipos de funcions
//static inline indica ao compilador que cambie as chamadas ás funcións polo propio codigo das funcions
//__atribute__((always_inline)) indica ao compilador gcc que sempre debe facer caso da intruccion inline
static inline quaternion multQuaternion(quaternion a, quaternion b) __attribute__((always_inline));
static inline quaternion cuadradoYSumaQuaternion(quaternion a, quaternion b) __attribute__((always_inline));

//Non usan inline porque non son parte dos calculos
int printQuaternion(quaternion a);
void randQuaternion(quaternion* a, int N);

int main(int argc, char** argv){
  register int i = 0;//variable para iterar nos bucles

  //Vectores de quaternions
  //__restrict__ indica ao compilador que as direccions apuntadas van ser accedidas unicamente por dito punteiro
  quaternion*__restrict__ a = NULL;
  quaternion*__restrict__ b = NULL;
  quaternion*__restrict__ dp = NULL;

  double ck;//variable para contar os ciclos

  int N = 0;//tamano de a e b
  int p = 0;//usase no calculo de N

  size_t tamano = 0;//usase para evitar moitas chamadas a sizeof

  if(argc == 2 && (p = atoi(argv[1])) > 0) {//comprobamos que o parametro e valido
      N = pow(10,p);//inicializamos N a 10 elevado a p
  } else {
    printf("El programa debe recibir como unico argumento un entero positivo\n");
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

  if((dp = (quaternion*)_mm_malloc(sizeof(quaternion),sizeof(quaternion))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }
  
  //inicializacion aleatoria de a e b
  randQuaternion(a,N);
  randQuaternion(b,N);

  mhz(1,1);//fixamos a frecuencia
  start_counter();//iniciamos o contador de ciclos

  memset(dp,0,sizeof(quaternion));//establecemos dp a 0

  //Realizamos as operacions indicadas
  for(i = 0; i < N ; i+=5){//LOOP UNROLLING
    *dp = cuadradoYSumaQuaternion(multQuaternion(a[i],b[i]),*dp);
    *dp = cuadradoYSumaQuaternion(multQuaternion(a[i+1],b[i+1]),*dp);
    *dp = cuadradoYSumaQuaternion(multQuaternion(a[i+2],b[i+2]),*dp);
    *dp = cuadradoYSumaQuaternion(multQuaternion(a[i+3],b[i+3]),*dp);
    *dp = cuadradoYSumaQuaternion(multQuaternion(a[i+4],b[i+4]),*dp);
  }

  ck=get_counter();//rematamos a medicion dos ciclos

  printf("Ciclos: %lf\n", ck);//imprimimos os ciclos
  printQuaternion(*dp);//imprimimos o quaternion resultado

  //liberamos a memoria aliñada
  _mm_free(a);
  a = NULL;
  _mm_free(b);
  b = NULL;
  _mm_free(dp);
  dp = NULL;

  return (EXIT_SUCCESS);//fin do programa con exito
}

//funcion para facer a multiplicacion de quaternions,inline explicase no prototipo
static inline quaternion multQuaternion(quaternion a, quaternion b) {
  quaternion c;//resultado
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
