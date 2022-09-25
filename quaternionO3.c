#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pmmintrin.h>
#include <math.h>
#include "rutinas_clock.h"

//Prototipos das funcions
void sumQuaternion(double* a, double* b, double* c);
void multQuaternion(double* a, double* b, double* c);
int printQuaternion(double* a);
void randQuaternion(double** a, int N);

int main(int argc, char** argv){
  //Variable contador
  int i = 0;

  //a, b e c son vectores de quaternions
  double** a = NULL;
  double** b = NULL;
  double** c = NULL;
  //dp e un quaternion
  double* dp = NULL;
	
  double ck; //Variable para contar os ciclos

  int N = 0; //Tamano de a e b
  int p = 0; //Usase no calculo de N

  if(argc == 2 && (p = atoi(argv[1])) > 0) { //Comprobamos que o parametro e valido
      N = pow(10,p); //Inicializamos N a 10 elevado a p
  } else {
    printf("El programa debe recibir como unico argumento un entero positivo\n");
    return (EXIT_FAILURE);
  }

  srand(1); //Fixamos a semilla para que o experimento sexa reproducible

  //Reserva de memoria aliñada para os vectores de quaternions
  if((a = (double**)_mm_malloc(N*sizeof(double*),sizeof(double*))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  if((b = (double**)_mm_malloc(N*sizeof(double*),sizeof(double*))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  if((c = (double**)_mm_malloc(N*sizeof(double*),sizeof(double*))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  //Inicializamos cada unha das componentes
  for(i = 0; i < N; i++){
    a[i]=(double*)malloc(4*sizeof(double));
    b[i]=(double*)malloc(4*sizeof(double));
    c[i]=(double*)malloc(4*sizeof(double));
  }

  //Reserva de memoria para o quaternion dp
  if((dp = (double*)_mm_malloc(4*sizeof(double),sizeof(double))) == NULL) {
    perror("Imposible realizar a reserva de memoria");
    return(EXIT_FAILURE);
  }

  //Inicializacion aleatoria de a e b
  randQuaternion(a,N);
  randQuaternion(b,N);

  mhz(1,1); //Fixamos a frecuencia
  start_counter(); //Iniciamos o contador de ciclos

  //Realizamos as operacions indicadas
  for(i = 0; i < N; i++){
    multQuaternion(a[i],b[i],c[i]);
  }

  //Inicializamos a cero as componentes de dp
  dp[0]=0;
  dp[1]=0;
  dp[2]=0;
  dp[3]=0;

  for(i = 0; i < N; i++) {
    multQuaternion(c[i],c[i],c[i]);
    sumQuaternion(dp, c[i], dp);
  }

  ck=get_counter(); //Rematamos a medicion dos ciclos
  printf("Ciclos: %lf\n", ck);//Imprimimos os ciclos

  printQuaternion(dp); //Imprimimos o quaternion resultado

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

//Suma por componentes
void sumQuaternion(double* a, double* b, double* c) {
  c[0] = a[0]+b[0];
  c[1] = a[1]+b[1];
  c[2] = a[2]+b[2];
  c[3] = a[3]+b[3];
}

//Multiplicacion de quaternions
void multQuaternion(double* a, double* b, double* c) {
  double c0 = a[0]*b[0] - a[1]*b[1] - a[2]*b[2] - a[3]*b[3];
  double c1 = a[0]*b[1] + a[1]*b[0] + a[2]*b[3] - a[3]*b[2];
  double c2 = a[0]*b[2] - a[1]*b[3] + a[2]*b[0] + a[3]*b[1];
  double c3 = a[0]*b[3] + a[1]*b[2] - a[2]*b[1] + a[3]*b[0];

  c[0] = c0;
  c[1] = c1;
  c[2] = c2;
  c[3] = c3;
}

int printQuaternion(double* a) { //impresion dos quaternions con 2 decimais
  return printf("%.2f + (%.2f)i + (%.2f)j + (%.2f)k\n", a[0],a[1],a[2],a[3]);
}

void randQuaternion(double** a, int N) { //inicializacion de quaternions aleatorios
  int i = 0;
  for(i=0;i<N;i++){
      a[i][0]=(double)rand()/(rand()+1);
      a[i][1]=(double)rand()/(rand()+1);
      a[i][2]=(double)rand()/(rand()+1);
      a[i][3]=(double)rand()/(rand()+1);
  }
}
