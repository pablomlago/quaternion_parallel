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
int printQuaternion(quaternion a);
void randQuaternion(quaternion* a, int N);

int main(int argc, char** argv){
  //Indicamos ao compilador que mantena preferiblemente a variable nun rexistro
  register int i = 0;

  //Vectores de quaternions
  //__restrict__ indica ao compilador que as direccions apuntadas van ser accedidas unicamente por dito punteiro
  quaternion*__restrict__ a = NULL;
  quaternion*__restrict__ b = NULL;
  quaternion*__restrict__ dp = NULL;

  double ck; //Variable para contar os ciclos

  int N = 0; //Tamano de a e b
  int p = 0; //Usase no calculo de N

  size_t tamano = 0; //Usase para evitar moitas chamadas a sizeof

  if(argc == 2 && (p = atoi(argv[1])) > 1) { //Comprobamos que o parametro e valido
      N = pow(10,p); //Inicializamos N a 10 elevado a p
  } else {
    printf("El programa debe recibir como unico argumento un entero positivo\n");
    return (EXIT_FAILURE); //Abortase a execucion
  }

  srand(1); //Fixamos a semilla para que o experimento sexa reproducible

  tamano = N*sizeof(quaternion); //Inicializamos tamano

  //Reserva de memoria aliñada para os quaternions
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

  //Inicializacion aleatoria de a e b
  randQuaternion(a,N);
  randQuaternion(b,N);

  mhz(1,1); //Fixamos a frecuencia
  start_counter(); //Iniciamos o contador de ciclos

  //Establecemos a cero todas as componentes dos seguintes vectores
  __m256d dw=_mm256_setzero_pd();
  __m256d dx=_mm256_setzero_pd();
  __m256d dy=_mm256_setzero_pd();
  __m256d dz=_mm256_setzero_pd();

  //Realizamos as operacions indicadas
  //N sera multiplo de 4 pues es un multiplo de 10 mayor o igual que 100
  for(i = 0; i < N;i+=4){//LOOP UNROLLING
    //Carga de datos
    __m256d aw = _mm256_set_pd(a[i+3].w, a[i+2].w, a[i+1].w, a[i].w);
    __m256d ax = _mm256_set_pd(a[i+3].x, a[i+2].x, a[i+1].x, a[i].x);
    __m256d ay = _mm256_set_pd(a[i+3].y, a[i+2].y, a[i+1].y, a[i].y);
    __m256d az = _mm256_set_pd(a[i+3].z, a[i+2].z, a[i+1].z, a[i].z);

    __m256d bw = _mm256_set_pd(b[i+3].w, b[i+2].w, b[i+1].w, b[i].w);
    __m256d bx = _mm256_set_pd(b[i+3].x, b[i+2].x, b[i+1].x, b[i].x);
    __m256d by = _mm256_set_pd(b[i+3].y, b[i+2].y, b[i+1].y, b[i].y);
    __m256d bz = _mm256_set_pd(b[i+3].z, b[i+2].z, b[i+1].z, b[i].z);

    __m256d cw;
    __m256d cx;
    __m256d cy;
    __m256d cz;

    //Calculo do produto dos quaternions contidos nos vectores a e b
    cw = _mm256_mul_pd(ax, bx);
    cw=_mm256_fmadd_pd(ay, by, cw);
    cw=_mm256_fmadd_pd(az, bz, cw);
    cw=_mm256_fmsub_pd(aw, bw, cw);

    cx=_mm256_mul_pd(az,by);
    cx=_mm256_fmsub_pd(aw, bx, cx);
    cx=_mm256_fmadd_pd(ax, bw, cx);
    cx=_mm256_fmadd_pd(ay, bz, cx);

    cy=_mm256_mul_pd(ax,bz);
    cy=_mm256_fmsub_pd(aw, by, cy);
    cy=_mm256_fmadd_pd(ay, bw, cy);
    cy=_mm256_fmadd_pd(az, bx, cy);

    cz=_mm256_mul_pd(ay,bx);
    cz=_mm256_fmsub_pd(aw, bz, cz);
    cz=_mm256_fmadd_pd(ax, by, cz);
    cz=_mm256_fmadd_pd(az, bw, cz);

    __m256d auxDw;
    __m256d auxDx;
    __m256d auxDy;
    __m256d auxDz;

    //Calculo das diferentes componentes de dp
    auxDw = _mm256_mul_pd(cx,cx);
    auxDw = _mm256_fmadd_pd(cy, cy, auxDw);
    auxDw = _mm256_fmadd_pd(cz, cz, auxDw);
    auxDw= _mm256_fmsub_pd(cw, cw, auxDw);
    dw = _mm256_add_pd(auxDw, dw);

    auxDx=_mm256_mul_pd(cw,cx);
    auxDx=_mm256_add_pd(auxDx,auxDx);
    dx = _mm256_add_pd(auxDx, dx);

    auxDy = _mm256_mul_pd(cw,cy);
    auxDy = _mm256_add_pd(auxDy, auxDy);
    dy= _mm256_add_pd(auxDy, dy);

    auxDz = _mm256_mul_pd(cw,cz);
    auxDz = _mm256_add_pd(auxDz, auxDz);
    dz= _mm256_add_pd(auxDz, dz);
  }

  //Temos que realizar catro sumas horizontais para obter o resultado final

  //Sumamos os pares de componentes consecutivas de dw e dx
  __m256d tempWx = _mm256_hadd_pd( dw, dx );
  //Sumamos os pares de componentes consecutivas de dy e dz
  __m256d tempYz = _mm256_hadd_pd( dy, dz );
  // Extraemos as sumas parciais correspondentes a cada componente
  __m256d parcial1 = _mm256_permute2f128_pd( tempWx, tempYz, 0x21 );
  // Extraemos as restantes sumas parciais correspondentes a cada componente
  __m256d parcial2 = _mm256_blend_pd(tempWx, tempYz, 0x0C);
  //Sumamos as sumas parcias obtendo o resultado final
  __m256d res = _mm256_add_pd(parcial1, parcial2);

  //Gardamos o resultado final en dp
  _mm256_store_pd((double *)dp, res);

  ck=get_counter(); //Rematamos a medicion dos ciclos
  printf("Ciclos: %lf\n", ck); //Imprimimos os ciclos

  printQuaternion(*dp); //Imprimimos o quaternion resultado

  //Liberamos a memoria aliñada
  _mm_free(a);
  a = NULL;
  _mm_free(b);
  b = NULL;
  _mm_free(dp);
  dp = NULL;

  //Fin do programa con exito
  return (EXIT_SUCCESS);
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
