#ifndef S2P_H_
#define S2P_H_

#include "stddef.h"
#include "stdbool.h"

struct Complex {
    double r;
    double i;
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

struct Noise_Data {
    struct Double_Array Rn;      
    struct Double_Array Fmin;
    struct Complex_Array GammaOpt;
};

struct S2P_Info {
    // must be here values
    double R_ref;
    struct Double_Array freq;
    struct Complex_Array s11;
    struct Complex_Array s12;
    struct Complex_Array s21;
    struct Complex_Array s22;
    char file_name[512];
    char full_path[512];
    char* file__content;
    size_t file__content_size;

    // maybe here values
    struct Noise_Data noise;

    // derived / calculated data
    struct Complex_Array z11;
    struct Complex_Array z12;
    struct Complex_Array z21;
    struct Complex_Array z22;
    struct Complex_Array zGopt;
};

struct S2P_Info_Array {
    struct S2P_Info* items;
    size_t count;
    size_t capacity;
};


int read_s2p_files(const char* dir, struct S2P_Info_Array *infos);
int parse_s2p_files(struct S2P_Info_Array *infos, bool calc_z);

#endif // S2P_H_