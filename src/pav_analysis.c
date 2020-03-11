#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N, float fm){
    float numerador=0;
    float denominador=0;
    float potencia_total=0;
    float t_long=0.020;

    for(int n=0;n<(fm*t_long);n++){
        numerador += x[n]*0.07672*cos((2*M_PI*n)/((fm*t_long)-1))*
                     x[n]*0.07672*cos((2*M_PI*n)/((fm*t_long)-1));
        denominador += 0.07672*cos((2*M_PI*n)/((fm*t_long)-1))*
                       0.07672*cos((2*M_PI*n)/((fm*t_long)-1));
    }
    potencia_total=numerador/denominador;
    return 10*log10(potencia_total);
}

float compute_am(const float *x, unsigned int N){
    float acumulado=0;
    for (int n=0; n<N ; n++){
        acumulado += x[n];
    }
    acumulado =acumulado/N;
    return acumulado;
}

float compute_zcr(const float *x, unsigned int N, float fm){
    int contando=0;
    for (int n=1;n<N;n++){
        if(x[n-1]*x[n]<=0){
            if(x[n]!=0){
                contando++;
            }
        }
    }
    return contando;
}