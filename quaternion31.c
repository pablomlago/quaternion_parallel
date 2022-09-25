#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <pmmintrin.h>
#include <string.h>
#include <immintrin.h>
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
static inline void multQuaternion(quaternion *a, quaternion *b, quaternion *c) __attribute__((always_inline));
static inline void cuadradoYSumaQuaternion(quaternion *a, quaternion *b, quaternion *c) __attribute__((always_inline));

//Non usan inline porque non son parte dos calculos
int printQuaternion(quaternion a);
void randQuaternion(quaternion* a, int N);

int main(int argc, char** argv){
	//Indicamos ao compilador que mantena preferiblemente a variable nun rexistro
  register int i = 0;

  //Vectores de quaternions
  //__restrict__ indica ao compilador que as direccions apuntadas van ser accedidas unicamente por dito punteiro
  quaternion*__restrict__ a = NULL;
  quaternion*__restrict__ b = NULL;
  quaternion*__restrict__ c = NULL;
  quaternion*__restrict__ dp = NULL;

  double ck; //Variable para contar os ciclos

  int N = 0; //Tamano de a e b
  int p = 0; //Usase no calculo de N

  size_t tamano = 0; //Usase para evitar moitas chamadas a sizeof

  if(argc == 2 && (p = atoi(argv[1])) > 0) { //Comprobamos que o parametro e valido
      N = pow(10,p);//Inicializamos N a 10 elevado a p
  } else {
    printf("El programa debe recibir como unico argumento un entero positivo\n");
    return (EXIT_FAILURE);//Abortase a execucion
  }

  srand(1);//Fixamos a semilla para que o experimento sexa reproducible

  tamano = N*sizeof(quaternion);//Inicializamos tamano

  //Reserva de memoria aliñada para os quaternions
  if((a = (quaternion*)_mm_malloc(tamano,sizeof(quaternion))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  if((b = (quaternion*)_mm_malloc(tamano,sizeof(quaternion))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  if((c = (quaternion*)_mm_malloc(tamano,sizeof(quaternion))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  if((dp = (quaternion*)_mm_malloc(sizeof(quaternion),sizeof(quaternion))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  //Inicializacion aleatoria de a e b
  randQuaternion(a,N);
  randQuaternion(b,N);

  mhz(1,1);//Fixamos a frecuencia
  start_counter();//Iniciamos o contador de ciclos

  memset(dp,0,sizeof(quaternion)); //Establecemos dp a 0

  //Realizamos as operacions indicadas
  for(i = 0; i < N ; i += 5) { //LOOP UNROLLING
    multQuaternion(&a[i],&b[i],&c[i]);
    multQuaternion(&a[i+1],&b[i+1],&c[i+1]);
    multQuaternion(&a[i+2],&b[i+2],&c[i+2]);
    multQuaternion(&a[i+3],&b[i+3],&c[i+3]);
    multQuaternion(&a[i+4],&b[i+4],&c[i+4]);

    cuadradoYSumaQuaternion(&c[i],dp,dp);
    cuadradoYSumaQuaternion(&c[i+1],dp,dp);
    cuadradoYSumaQuaternion(&c[i+2],dp,dp);
    cuadradoYSumaQuaternion(&c[i+3],dp,dp);
    cuadradoYSumaQuaternion(&c[i+4],dp,dp);
  }

  ck=get_counter();//Rematamos a medicion dos ciclos
  printf("Ciclos: %lf\n", ck);//Imprimimos os ciclos

  printQuaternion(*dp);//Imprimimos o quaternion resultado

  //Liberamos a memoria aliñada
  _mm_free(a);
  a = NULL;
  _mm_free(b);
  b = NULL;
  _mm_free(c);
  c = NULL;
  _mm_free(dp);
  dp = NULL;

  //Fin do programa con exito
  return (EXIT_SUCCESS);
}

//Funcion para facer a multiplicacion de quaternions,inline explicase no prototipo
static inline void multQuaternion(quaternion *a, quaternion *b, quaternion *c) {
  //Cargamos os quaternions en sendas variables vectoriais
  __m256d vA = _mm256_load_pd((double *)a);
  __m256d vB = _mm256_load_pd((double *)b);
	//vC contera o resultado da multiplicacion
  __m256d vC;
  //Precisaremos cambiar de signo o vector excepto a sua componente menos significativa
  __m256d mask = _mm256_set_pd(-1.,-1.,-1.,1.);

  //Calculamos as permutacions e realizamos as multiplicacions adecuadas
  //Acumulamos os resultados en vC

  __m256d vAp = _mm256_permute4x64_pd(vA, 0xC9); //xywz 11001001
  __m256d vBp = _mm256_permute4x64_pd(vB, 0x2D); //xzyw 00101101

  vC = _mm256_mul_pd(vAp, vBp);

  vAp = _mm256_permute4x64_pd(vA, 0x36); //yxzw 00110110
  vBp = _mm256_permute4x64_pd(vB, 0xD2); //ywxz 11010010

  vC = _mm256_fmadd_pd(vAp, vBp, vC);

  vAp = _mm256_permute4x64_pd(vA, 0x63); //zwyx 01100011
  vBp = _mm256_permute4x64_pd(vB, 0x87); //zxwy 10000111

  vC = _mm256_fmadd_pd(vAp, vBp, vC);

  vAp = _mm256_permute4x64_pd(vA,0x9C);//wzxy 10011100
  vBp = _mm256_permute4x64_pd(vB,0x78);//wyzx 01111000

  vC = _mm256_fmsub_pd(vAp, vBp, vC);
  vC = _mm256_mul_pd(vC, mask);

  //Gardamos o resultado en c
  _mm256_store_pd((double *)c, vC);
}

//funcion para realizar o cadrado do quaternion a e sumarlle b
static inline void cuadradoYSumaQuaternion(quaternion* a, quaternion* b, quaternion* c) {
  //Cargamos os quaternions en sendas variables vectoriais
  __m256d vA = _mm256_load_pd((double *)a);
  __m256d vB = _mm256_load_pd((double *)b);
  //Creamos un vector que contera en todas as suas componentes a primeira de a
  __m256d vAw = _mm256_set1_pd(a->w);

  //neg contera o produto escalar de a por si mesmo
  __m256d neg = _mm256_mul_pd(vA,vA);

  //Extraemos os 128 bits menos significativos
  __m128d vlow  = _mm256_castpd256_pd128(neg);
  //Extraemos os 128 bits mais significativos
  __m128d vhigh = _mm256_extractf128_pd(neg, 1);
  //Realizamos a suma dos dous pares de doubles
  vlow  = _mm_add_pd(vlow, vhigh);
  //Extraemos os 64 bits mais significativos do resultado
  //Que se gardaran nas duas componentes dun vector
  __m128d high64 = _mm_unpackhi_pd(vlow, vlow);
  //Realizamos a suma e extraemos os 64 bits menos significativos  do resultado
  neg = _mm256_set_pd(0.,0.,0.,_mm_cvtsd_f64(_mm_add_sd(vlow, high64)));

  __m256d pos = _mm256_mul_pd(vAw,vA);
  pos = _mm256_add_pd(pos,pos);

  __m256d vC = _mm256_sub_pd(pos,neg);
  vC = _mm256_add_pd(vB, vC);

  //Gardamos o resultado en c
  _mm256_store_pd((double *)c, vC);

}

//Funcion para imprimir un quaternion con dous decimais
int printQuaternion(quaternion a) {
  return printf("%.2f + (%.2f)i + (%.2f)j + (%.2f)k\n", a.w,a.x,a.y,a.z);
}

//Funcion para inicialiar quaternions aleatorios
void randQuaternion(quaternion* a, int N) {
  int i = 0;
  for(i=0;i<N;i++){
      a[i].w=(double)rand()/(rand()+1);
      a[i].x=(double)rand()/(rand()+1);
      a[i].y=(double)rand()/(rand()+1);
      a[i].z=(double)rand()/(rand()+1);
  }
}
