#include "circuit.h"

#include <float.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

double rand_from(double min, double max) {
    double range = (max - min);
    double div = RAND_MAX / range;
    return min + (rand() / div);
}

// values are assumed to be double if value_map is NULL
double circuit_optimizer_calculate_loss_hinge_lt(void* values, size_t value_count, double (*value_map)(size_t, void*), double goal) {
    double loss = 0.0;
    if (value_map != NULL) {
        for (size_t i = 0; i < value_count; i++) {
            loss += fmax(0.0, value_map(i, values) - goal);
        }
    } else {
        for (size_t i = 0; i < value_count; i++){
            loss += fmax(0.0, ((double*)values)[i] - goal);
        }
    }
    return loss;
}

// values are assumed to be double if value_map is NULL
double circuit_optimizer_calculate_loss_hinge_gt(void* values, size_t value_count, double (*value_map)(size_t, void*), double goal) {
    double loss = 0.0;
    if (value_map != NULL) {
        for (size_t i = 0; i < value_count; i++){
            loss += fmax(0.0, goal - (*value_map)(i, values));
        }
    } else {
        for (size_t i = 0; i < value_count; i++){
            loss += fmax(0.0, goal - ((double*)values)[i]);
        }
    }
    return loss;
}

double circuit_optimizer_evaluate_goal(const struct Optimization_Goal* goal, struct Simulation_State* sim_state) {

    double loss_value;
    void *target_values;
    size_t start_index = 0;
    size_t count = sim_state->n_frequencies;

    // find start and count
    assert(goal->f_min < goal->f_max);
    for (size_t i = 0; i < sim_state->n_frequencies; i++) {
        if (goal->f_min <= sim_state->frequencies[i]) {
            // we just over the min frequency
            start_index = i;
            break;
        }
    }

    for (size_t i = start_index; i < sim_state->n_frequencies; i++) {
        if (goal->f_max <= sim_state->frequencies[i]) {
            // we just over the max frequency
            count = i - start_index + 1;
            break;
        }
    }

    assert(start_index < sim_state->n_frequencies);
    assert(count <= sim_state->n_frequencies - start_index);
    assert(count > 0);

    switch(goal->target) {
    case OPTIMIZATION_TARGET_S11:
        target_values = sim_state->s11_result_plottable + start_index;
    break;
    case OPTIMIZATION_TARGET_S21:
        target_values = sim_state->s21_result_plottable + start_index;
    break;
    case OPTIMIZATION_TARGET_S12:
        target_values = sim_state->s12_result_plottable + start_index;
    break;
    case OPTIMIZATION_TARGET_S22:
        target_values = sim_state->s22_result_plottable + start_index;
    break;
    case OPTIMIZATION_TARGET_MU:
        target_values = sim_state->stab_mu + start_index;
    break;
    case OPTIMIZATION_TARGET_MU_PRIME:
        target_values = sim_state->stab_mu_prime + start_index;
    break;
    default:
        assert(false && "wrong or implement me");
    break;
    }

    switch(goal->type) {
    case OPTIMIZATION_TYPE_LESS_THAN:
        loss_value = circuit_optimizer_calculate_loss_hinge_lt(target_values, count, goal->value_map, goal->goal_value);
    break;
    case OPTIMIZATION_TYPE_MORE_THAN:
        loss_value = circuit_optimizer_calculate_loss_hinge_gt(target_values, count, goal->value_map, goal->goal_value);
    break;
    default:
        assert(false && "wrong or implement me");
    break;
    }

    return loss_value;
}

void circuit_random_tweak_cascade(struct Circuit_Component *component_cascade, size_t n_components) {
    double rand_double = rand_from(0.4, 1.5);
    int index = rand() % n_components;

    switch(component_cascade[index].kind) {
        case CIRCUIT_COMPONENT_RESISTOR_IDEAL:
            component_cascade[index].as.resistor_ideal.R *= rand_double;
        break;
        case CIRCUIT_COMPONENT_CAPACITOR_IDEAL:
            component_cascade[index].as.capacitor_ideal.C *= rand_double;
        break;
        case CIRCUIT_COMPONENT_INDUCTOR_IDEAL:
            component_cascade[index].as.inductor_ideal.L *= rand_double;
        break;
        case CIRCUIT_COMPONENT_STAGE:
            size_t i = rand() % component_cascade[index].as.stage.n_settings;
            component_cascade[index].as.stage.selected_setting = i;
        break;
        case CIRCUIT_COMPONENT_RESISTOR_IDEAL_PARALLEL:
            component_cascade[index].as.resistor_ideal_parallel.R *= rand_double;
        break;
        case CIRCUIT_COMPONENT_CAPACITOR_IDEAL_PARALLEL:
            component_cascade[index].as.capacitor_ideal_parallel.C *= rand_double;
        break;
        case CIRCUIT_COMPONENT_INDUCTOR_IDEAL_PARALLEL:
            component_cascade[index].as.inductor_ideal_parallel.L *= rand_double;
        break;
        case CIRCUIT_COMPONENT_KIND_COUNT:
            assert(false && "implement me (next component kind)");
        break;
        default:
            assert(false && "implement me (next component kinds?)");
        break;
    }
}

bool circuit_optimizer_setup(struct Optimizer_State* state, size_t max_iterations, struct Circuit_Component *intial_component_cascade, size_t n_components) {
    state->iteration = 0;
    state->max_iterations = max_iterations;
    state->best_total_loss_value = DBL_MAX;

    state->initial_component_cascade = malloc(sizeof(*intial_component_cascade) * n_components);
    state->temporary_component_cascade = malloc(sizeof(*intial_component_cascade) * n_components);
    state->best_component_cascade = malloc(sizeof(*intial_component_cascade) * n_components);

    memcpy(state->initial_component_cascade, intial_component_cascade, sizeof(*intial_component_cascade) * n_components);

    return true;
}

// returns true if done
bool circuit_optimizer_update_one_round(struct Optimizer_State* opt_state, struct Simulation_State* sim_state, const struct Simulation_Settings* sim_settings, const struct Optimization_Goal* goals, size_t goal_count, struct Circuit_Component *component_cascade, size_t n_components) {

    if (opt_state->iteration >= opt_state->max_iterations || opt_state->best_total_loss_value <= 1e-15) {
        return true;
    }

    printf("optimizer round %zu\n", opt_state->iteration);

    // back up the thing
    memcpy(opt_state->temporary_component_cascade, component_cascade, sizeof(*component_cascade) * n_components);

    circuit_random_tweak_cascade(opt_state->temporary_component_cascade, n_components);
    circuit_simulation_setup(opt_state->temporary_component_cascade, n_components, sim_state, sim_settings);
    circuit_simulation_do(sim_state, false);

    double total_loss = 0.0f;
    for (size_t i = 0; i < goal_count; i++) {
        total_loss += goals[i].weight * circuit_optimizer_evaluate_goal(&goals[i], sim_state);
    }

    if (total_loss < opt_state->best_total_loss_value) {
        opt_state->best_total_loss_value = total_loss;
        memcpy(component_cascade, opt_state->temporary_component_cascade, sizeof(*component_cascade) * n_components);
        memcpy(opt_state->best_component_cascade, opt_state->temporary_component_cascade, sizeof(*component_cascade) * n_components);
    }

    printf("optimizer best loss: %f, current loss: %f\n", opt_state->best_total_loss_value, total_loss);

    opt_state->iteration++;

    return false;
}
