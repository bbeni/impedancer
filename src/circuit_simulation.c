#include "circuit.h"

#include "stdlib.h"
#include "assert.h"
#include "string.h"
#include "stdio.h"

void calc_t_from_s_array(struct Complex_2x2_SoA *s, struct Complex_2x2_SoA *t_out, size_t length) {
    for (size_t i = 0; i < length; i++) {

        struct Complex s1[2][2];
        s1[0][0].r = s->r11[i];
        s1[0][1].r = s->r12[i];
        s1[1][0].r = s->r21[i];
        s1[1][1].r = s->r22[i];

        s1[0][0].i = s->i11[i];
        s1[0][1].i = s->i12[i];
        s1[1][0].i = s->i21[i];
        s1[1][1].i = s->i22[i];

        struct Mma_Complex_2x2 t = calc_t_from_s(s1);

        t_out->r11[i] = t.v[0][0].r;
        t_out->r12[i] = t.v[0][1].r;
        t_out->r21[i] = t.v[1][0].r;
        t_out->r22[i] = t.v[1][1].r;

        t_out->i11[i] = t.v[0][0].i;
        t_out->i12[i] = t.v[0][1].i;
        t_out->i21[i] = t.v[1][0].i;
        t_out->i22[i] = t.v[1][1].i;
    }
}

void calc_s_from_t_array(struct Complex_2x2_SoA *t, struct Complex_2x2_SoA *s_out, size_t length) {
    for (size_t i = 0; i < length; i++) {

        struct Complex t1[2][2];
        t1[0][0].r = t->r11[i];
        t1[0][1].r = t->r12[i];
        t1[1][0].r = t->r21[i];
        t1[1][1].r = t->r22[i];

        t1[0][0].i = t->i11[i];
        t1[0][1].i = t->i12[i];
        t1[1][0].i = t->i21[i];
        t1[1][1].i = t->i22[i];

        struct Mma_Complex_2x2 t = calc_s_from_t(t1);

        s_out->r11[i] = t.v[0][0].r;
        s_out->r12[i] = t.v[0][1].r;
        s_out->r21[i] = t.v[1][0].r;
        s_out->r22[i] = t.v[1][1].r;

        s_out->i11[i] = t.v[0][0].i;
        s_out->i12[i] = t.v[0][1].i;
        s_out->i21[i] = t.v[1][0].i;
        s_out->i22[i] = t.v[1][1].i;
    }
}

#define Z0 50.0
bool simulation_interpolate_sparams_circuit_component(struct Circuit_Component *component, double *frequencies, struct Complex_2x2_SoA *s_out, size_t n_frequencies) {
    switch (component->kind) {
    case CIRCUIT_COMPONENT_RESISTOR_IDEAL:
        struct Circuit_Component_Resistor_Ideal r = component->as.resistor_ideal;
        for (size_t i = 0; i < n_frequencies; i++) {
            struct Complex s11 = mma_complex(r.R / (r.R + 2 * Z0), 0);
            struct Complex s12 = mma_complex(2 * Z0 / (r.R + 2 * Z0), 0);
            struct Complex s21 = mma_complex(2 * Z0 / (r.R + 2 * Z0), 0);
            struct Complex s22 = mma_complex(r.R / (r.R + 2 * Z0), 0);
            s_out->r11[i] = s11.r;
            s_out->r12[i] = s12.r;
            s_out->r21[i] = s21.r;
            s_out->r22[i] = s22.r;
            s_out->i11[i] = s11.i;
            s_out->i12[i] = s12.i;
            s_out->i21[i] = s21.i;
            s_out->i22[i] = s22.i;
        }
    break;
    case CIRCUIT_COMPONENT_CAPACITOR_IDEAL:
        assert(false && "TODO: impement me");
    break;
    case CIRCUIT_COMPONENT_INDUCTOR_IDEAL:
        assert(false && "TODO: impement me");
    break;
    case CIRCUIT_COMPONENT_STAGE:
        struct Circuit_Component_Stage* stage = &component->as.stage;
        struct S2P_Info *info = &stage->s2p_infos[stage->selected_setting];
        mma_spline_cubic_natural_complex_2(info->freq, info->s11, info->data_length, s_out->r11, s_out->i11, frequencies, n_frequencies);
        mma_spline_cubic_natural_complex_2(info->freq, info->s12, info->data_length, s_out->r12, s_out->i12, frequencies, n_frequencies);
        mma_spline_cubic_natural_complex_2(info->freq, info->s21, info->data_length, s_out->r21, s_out->i21, frequencies, n_frequencies);
        mma_spline_cubic_natural_complex_2(info->freq, info->s22, info->data_length, s_out->r22, s_out->i22, frequencies, n_frequencies);
        break;
    case CIRCUIT_COMPONENT_RESISTOR_IDEAL_PARALLEL:
        assert(false && "TODO: impement me");
    break;
    case CIRCUIT_COMPONENT_CAPACITOR_IDEAL_PARALLEL:
        assert(false && "TODO: impement me");
    break;
    case CIRCUIT_COMPONENT_INDUCTOR_IDEAL_PARALLEL:
        assert(false && "TODO: impement me");
    break;
    case CIRCUIT_COMPONENT_KIND_COUNT:
        assert(false && "TODO: impement me");
    break;
    }

    return true;
}

bool circuit_simulation_setup(struct Circuit_Component *component_cascade, size_t n_components, struct Simulation_State *sim_state) {

    size_t n_frequencies = 1000;
    double start_f = 1.0f;
    double end_f = 50e9;
    double df = (end_f - start_f) / (n_frequencies - 1);

    sim_state->n_components = n_components;
    sim_state->n_frequencies = n_frequencies;
    sim_state->components_cascade = component_cascade;

    /// START MALLOC BUSINESS
    sim_state->frequencies = malloc(sizeof(*sim_state->frequencies) * n_frequencies);
    sim_state->s_result.r11 = malloc(8 * sizeof(*sim_state->s_result.r11) * sim_state->n_frequencies);
    sim_state->s_result.r12 = &sim_state->s_result.r11[1 * sim_state->n_frequencies];
    sim_state->s_result.r21 = &sim_state->s_result.r11[2 * sim_state->n_frequencies];
    sim_state->s_result.r22 = &sim_state->s_result.r11[3 * sim_state->n_frequencies];
    sim_state->s_result.i11 = &sim_state->s_result.r11[4 * sim_state->n_frequencies];
    sim_state->s_result.i12 = &sim_state->s_result.r11[5 * sim_state->n_frequencies];
    sim_state->s_result.i21 = &sim_state->s_result.r11[6 * sim_state->n_frequencies];
    sim_state->s_result.i22 = &sim_state->s_result.r11[7 * sim_state->n_frequencies];

    sim_state->t_result.r11 = malloc(8 * sizeof(*sim_state->t_result.r11) * sim_state->n_frequencies);
    sim_state->t_result.r12 = &sim_state->t_result.r11[1 * sim_state->n_frequencies];
    sim_state->t_result.r21 = &sim_state->t_result.r11[2 * sim_state->n_frequencies];
    sim_state->t_result.r22 = &sim_state->t_result.r11[3 * sim_state->n_frequencies];
    sim_state->t_result.i11 = &sim_state->t_result.r11[4 * sim_state->n_frequencies];
    sim_state->t_result.i12 = &sim_state->t_result.r11[5 * sim_state->n_frequencies];
    sim_state->t_result.i21 = &sim_state->t_result.r11[6 * sim_state->n_frequencies];
    sim_state->t_result.i22 = &sim_state->t_result.r11[7 * sim_state->n_frequencies];
    sim_state->intermediate_states = malloc(sizeof(*sim_state->intermediate_states) * n_components);
    for (size_t i_comp = 0; i_comp < n_components; i_comp++) {
        sim_state->intermediate_states[i_comp].s.r11 = malloc(8 * sizeof(*sim_state->intermediate_states[i_comp].s.r11) * sim_state->n_frequencies);
        sim_state->intermediate_states[i_comp].s.r12 = &sim_state->intermediate_states[i_comp].s.r11[1 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].s.r21 = &sim_state->intermediate_states[i_comp].s.r11[2 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].s.r22 = &sim_state->intermediate_states[i_comp].s.r11[3 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].s.i11 = &sim_state->intermediate_states[i_comp].s.r11[4 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].s.i12 = &sim_state->intermediate_states[i_comp].s.r11[5 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].s.i21 = &sim_state->intermediate_states[i_comp].s.r11[6 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].s.i22 = &sim_state->intermediate_states[i_comp].s.r11[7 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].t.r11 = malloc(8 * sizeof(*sim_state->intermediate_states[i_comp].t.r11) * sim_state->n_frequencies);
        sim_state->intermediate_states[i_comp].t.r12 = &sim_state->intermediate_states[i_comp].t.r11[1 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].t.r21 = &sim_state->intermediate_states[i_comp].t.r11[2 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].t.r22 = &sim_state->intermediate_states[i_comp].t.r11[3 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].t.i11 = &sim_state->intermediate_states[i_comp].t.r11[4 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].t.i12 = &sim_state->intermediate_states[i_comp].t.r11[5 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].t.i21 = &sim_state->intermediate_states[i_comp].t.r11[6 * sim_state->n_frequencies];
        sim_state->intermediate_states[i_comp].t.i22 = &sim_state->intermediate_states[i_comp].t.r11[7 * sim_state->n_frequencies];
    }
    /// END MALLOC BUSINESS

    // generate frequency
    for (size_t i = 0; i < n_frequencies; i++) {
        sim_state->frequencies[i] = start_f + i * df;
    }

    for (size_t i_comp = 0; i_comp < n_components; i_comp++) {
        simulation_interpolate_sparams_circuit_component(
            &sim_state->components_cascade[i_comp], sim_state->frequencies,
            &sim_state->intermediate_states[i_comp].s, sim_state->n_frequencies
        );

        calc_t_from_s_array(
            &sim_state->intermediate_states[i_comp].s,
            &sim_state->intermediate_states[i_comp].t,
            sim_state->n_frequencies
        );
    }

    return true;
}

bool circuit_simulation_destroy(struct Simulation_State *sim_state) {

    /// START FREE BUSINESS
    free(sim_state->frequencies);
    free(sim_state->intermediate_states);
    free(sim_state->s_result.r11);
    free(sim_state->t_result.r11);
    for (size_t i_comp = 0; i_comp < sim_state->n_components; i_comp++) {
        free(sim_state->intermediate_states[i_comp].s.r11);
        free(sim_state->intermediate_states[i_comp].t.r11);
    }
    /// END FREE BUSINESS

    return true;
}

bool circuit_simulation_do(struct Simulation_State *sim_state) {

    printf("simulation started with %lld components and %lld frequencies\n", sim_state->n_components, sim_state->n_frequencies);


    if (sim_state->n_components < 1) {
        printf("ERROR: need at least one component_cascade in sim_state\n");
        return false;
    }

    size_t n_f = sim_state->n_frequencies;
    size_t byte_size_total = 8 * n_f * sizeof(*sim_state->t_result.r11);

    // T_0 T_1 T_2 ...
    // ---------------
    // copy T_f = T_0
    //
    // copy T_t = T_f
    // T_f = T_t T_1
    //
    // copy T_t = T_f
    // T_f = T_t T_2
    //
    // ...

    // copy T_f = T_0
    memcpy(
        sim_state->t_result.r11,
        sim_state->intermediate_states[0].t.r11,
        byte_size_total
    );

    // reserve some temporary space (T_t)
    struct Complex_2x2_SoA t_intermediate;
    mma_temp_set_restore_point();
    t_intermediate.r11 = mma_temp_alloc(byte_size_total);
    t_intermediate.r12 = &t_intermediate.r11[1 * n_f];
    t_intermediate.r21 = &t_intermediate.r11[2 * n_f];
    t_intermediate.r22 = &t_intermediate.r11[3 * n_f];
    t_intermediate.i11 = &t_intermediate.r11[4 * n_f];
    t_intermediate.i12 = &t_intermediate.r11[5 * n_f];
    t_intermediate.i21 = &t_intermediate.r11[6 * n_f];
    t_intermediate.i22 = &t_intermediate.r11[7 * n_f];

    for (size_t i_comp = 1; i_comp < sim_state->n_components; i_comp++) {

        // copy T_t = T_f
        memcpy(
            t_intermediate.r11,
            sim_state->t_result.r11,
            byte_size_total
        );

        struct Complex_2x2_SoA *tx = &t_intermediate; // T_t
        struct Complex_2x2_SoA *ty = &sim_state->intermediate_states[i_comp].t;
        struct Complex_2x2_SoA *tf = &sim_state->t_result;

        // T_f = T_x * T_y
        // (A+iB)(C+iD) = (AC − BD) + i (AD + BC)
        for (size_t i = 0; i < n_f; i++) {
            // AC - BD     real real - imag imag
            tf->r11[i] = tx->r11[i] * ty->r11[i] + tx->r12[i] * ty->r21[i]  -  (tx->i11[i] * ty->i11[i] + tx->i12[i] * ty->i21[i]);
            tf->r21[i] = tx->r21[i] * ty->r11[i] + tx->r22[i] * ty->r21[i]  -  (tx->i21[i] * ty->i11[i] + tx->i22[i] * ty->i21[i]);
            tf->r12[i] = tx->r11[i] * ty->r12[i] + tx->r12[i] * ty->r22[i]  -  (tx->i11[i] * ty->i12[i] + tx->i12[i] * ty->i22[i]);
            tf->r22[i] = tx->r21[i] * ty->r12[i] + tx->r22[i] * ty->r22[i]  -  (tx->i21[i] * ty->i12[i] + tx->i22[i] * ty->i22[i]);
            // i (AD + BC) realx imagy + imagx realy
            tf->r11[i] = tx->r11[i] * ty->i11[i] + tx->r12[i] * ty->i21[i]  +  (tx->i11[i] * ty->r11[i] + tx->i12[i] * ty->r21[i]);
            tf->r21[i] = tx->r21[i] * ty->i11[i] + tx->r22[i] * ty->i21[i]  +  (tx->i21[i] * ty->r11[i] + tx->i22[i] * ty->r21[i]);
            tf->r12[i] = tx->r11[i] * ty->i12[i] + tx->r12[i] * ty->i22[i]  +  (tx->i11[i] * ty->r12[i] + tx->i12[i] * ty->r22[i]);
            tf->r22[i] = tx->r21[i] * ty->i12[i] + tx->r22[i] * ty->i22[i]  +  (tx->i21[i] * ty->r12[i] + tx->i22[i] * ty->r22[i]);
        }
    }

    calc_s_from_t_array(
        &sim_state->t_result,
        &sim_state->s_result,
        sim_state->n_frequencies
    );

    printf("simulation finished.\n");

    mma_temp_restore();

    return true;
}
