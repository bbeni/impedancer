#ifndef S2P_H_
#define S2P_H_

#include "stddef.h"

struct Complex {
    double r;
    double i;
};

struct Noise_Data {
    double Rn;      
    double Fmin;
    double GammaOptdB;
    double GammaOptAngle;
};

struct Noise_Array {
    struct Noise_Data *items;
    size_t count;
    size_t capacity;
};

struct Double_Array {
    double *items;
    size_t count;
    size_t capacity;
};

struct Complex_Array {
    struct Complex *items;
    size_t count;
    size_t capacity;
};

struct S2P_Info {
    struct Double_Array freq;
    struct Complex_Array s11;
    struct Complex_Array s12;
    struct Complex_Array s21;
    struct Complex_Array s22;
    struct Noise_Array noise;
    char file_name[512];
    char full_path[512];
    char* file__content;
    size_t file__content_size;
};

struct S2P_Info_Array {
    struct S2P_Info* items;
    size_t count;
    size_t capacity;
};


int read_s2p_files(const char* dir, struct S2P_Info_Array *infos);
int parse_s2p_files(struct S2P_Info_Array *infos);

#endif // S2P_H_