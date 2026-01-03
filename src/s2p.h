#ifndef S2P_H_
#define S2P_H_

#include "nob.h"

typedef struct Complex {
    double r;
    double i;
} Complex;

typedef struct {
    double Rn;      
    double Fmin;
    double GammaOptdB;
    double GammaOptAngle;
} Noise_Data;

typedef struct Noise_Array {
    Noise_Data *items;
    size_t count;
    size_t capacity;
} Noise_Array;

typedef struct Double_Array {
    double *items;
    size_t count;
    size_t capacity;
} Double_Array;

typedef struct Complex_Array {
    Complex *items;
    size_t count;
    size_t capacity;
} Complex_Array;


typedef struct S2P_Info {
    Double_Array freq;
    Complex_Array s11;
    Complex_Array s12;
    Complex_Array s21;
    Complex_Array s22;

    Noise_Array noise;
    char file_name[256];
    Nob_String_Builder file_content;
} S2P_Info;

typedef struct S2P_Infos {
    S2P_Info* items;
    size_t count;
    size_t capacity;
} S2P_Infos;


int read_s2p_files(const char* dir, S2P_Infos *infos);
int parse_s2p_files(S2P_Infos *infos);

#endif // S2P_H_