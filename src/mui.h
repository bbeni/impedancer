/* My Immediate Ui

Copyright 2024 Benjamin Froelich

stb style header only library use #define MY_UI_IMPLEMENTATION at
point you want the implementation.
for usage see end of this file.
*/

#ifndef MUI_H_
#define MUI_H_

#include "math.h"
//#define STB_SPRINTF_IMPLEMENTATION
//#include "stb_sprintf.h"
#include "raylib.h"
#include "nob.h"

// globals need to be set by user!
float mui_time_now;
Vector2 mui_mouse_pos;

typedef struct {
    Color global_background_color;
    Color background_color;
    Color background_hover_color;
    float roundedness;

    Color text_color;
    Color text_hover_color;
    float text_size;

    Color border_color;
    Color border_hover_color;
    float border_thickness;

    Color label_background_color;
    Color label_text_color;
    float label_text_size;

    Color textinput_background_color;
    Color textinput_text_color;
    Color textinput_hint_text_color;
    float textinput_text_size;

    Font font;
    Font label_font;
    Font textinput_font;

    float animation_speed_to_hover;
    float animation_speed_to_normal;

} Mui_Theme;

// default theme
Mui_Theme mui_protos_theme;


typedef struct {
    float hover_t; // from 0 to 1 representing the hover state (animation)
    float last_time;

    Mui_Theme *theme; // can be NULL
} Mui_Button_State;

typedef struct {
    float hover_t; // from 0 to 1 representing the hover state (animation)
    float last_time;
    bool checked;

    Mui_Theme *theme; // can be NULL
} Mui_Checkbox_State;

#ifndef MUI_TEXTINPUT_CAP
#define MUI_TEXTINPUT_CAP 35
#endif
typedef struct {
    char buf[MUI_TEXTINPUT_CAP + 1];
    size_t count;
    size_t cursor;
    bool active; // wheter it was clicked and it is writable

    Mui_Theme *theme;
} Mui_Textinput_State;

//
// mui.h utility API
//

Color mui_interpolate_color(Color a, Color b, float t);
void mui_move_towards(float *x, float target, float speed, float dt);
Rectangle mui_shrink(Rectangle r, float amount);
Rectangle mui_cut_left(Rectangle r, float amount, Rectangle *out_left);
Rectangle mui_cut_right(Rectangle r, float amount, Rectangle *out_right);
Rectangle mui_cut_top(Rectangle r, float amount, Rectangle *out_top);

//
// my_ui.h API
//

bool mui_load_ttf_font(const char *font_file);
bool mui_button(Mui_Button_State *state, const char* text, Rectangle place);
void mui_checkbox(Mui_Checkbox_State *state, const char *text, Rectangle place);
void mui_label(Mui_Theme *theme, char *text, Rectangle place);
void mui_load();

//
// Some easing functions from easings.net
//

float mui_ease_in_quad(float t);
float mui_ease_out_quad(float t);
float mui_ease_in_out_quad(float t);
float mui_ease_in_cubic(float t);
float mui_ease_out_cubic(float t);
float mui_ease_in_out_cubic(float t);
float mui_ease_in_sin(float t);
float mui_ease_out_sin(float t);
float mui_ease_in_out_sin(float t);


#endif // MUI_H_

#ifdef MUI_IMPLEMENTATION

bool mui_load_resource_from_file(const char *file_path, int *out_size, void **data) {
	Nob_String_Builder sb = {0};
	if (!nob_read_entire_file(file_path, &sb)) {
		return false;
	}
	*data = sb.items;
	*out_size = sb.count;
	return true;
}


bool mui_load_ttf_font(const char *font_file) {

    int size;
    void *data;
    bool success = mui_load_resource_from_file(font_file, &size, &data);
    if (!success) {
        printf("My Error: loading resource (%s) so we are not loading fonts..\n", font_file);
        return false;
    }

    mui_protos_theme.font = LoadFontFromMemory(".ttf", data, size, mui_protos_theme.text_size, 0, 250);
    mui_protos_theme.label_font = LoadFontFromMemory(".ttf", data, size, mui_protos_theme.label_text_size, 0, 250);
    mui_protos_theme.textinput_font = LoadFontFromMemory(".ttf", data, size, mui_protos_theme.textinput_text_size, 0, 250);

    return true;    
}

Mui_Theme mui_protos_theme = {
    .global_background_color = {50, 57, 60, 255},
    .background_color = {142, 142, 142, 255},
    .background_hover_color = {1, 2, 3, 255},

    .text_color = {211, 211, 211, 255},
    .text_hover_color = {252, 252, 252, 255},

    .border_color = {224, 224, 224, 255},
    .border_hover_color = {244, 142, 255, 255},

    .label_background_color = {40, 47, 50, 255},
    .label_text_color = {230, 240, 225, 255},

    .textinput_background_color = {120, 205, 160, 255},
    .textinput_text_color = {224, 224, 224, 255},
    .textinput_hint_text_color = {224, 124, 224, 255},

    .border_thickness = 2.0f,
    .text_size = 28.0f,
    .label_text_size = 28.0f,
    .textinput_text_size = 28.0f,

    .animation_speed_to_hover = 18.0f,
    .animation_speed_to_normal = 11.0f,
    .roundedness = 0.15f,
};

Color mui_interpolate_color(Color a, Color b, float t) {
    
    a.r = (1.0f - t) * a.r + t * b.r;
    a.g = (1.0f - t) * a.g + t * b.g;
    a.b = (1.0f - t) * a.b + t * b.b;
    a.a = (1.0f - t) * a.a + t * b.a;

    return a;
}

void mui_move_towards(float *x, float target, float speed, float dt) {
    if (*x == target) {
        return;
    }
    if (*x > target) {
        *x -= speed * dt;
        if (*x < target) {
            *x = target;
        }
    } else {
        *x += speed * dt;
        if (*x > target) {
            *x = target;
        }
    }
}

// Shrink Rectangle by amount in all directions
Rectangle mui_shrink(Rectangle r, float amount) {
    r.width  -= 2*amount;
    r.height -= 2*amount;
    r.x      += amount;
    r.y      += amount;
    return r;
}


// Cut Rectangle from left side by amount - out_left can be NULL.
Rectangle mui_cut_left(Rectangle r, float amount, Rectangle *out_left) {
    if (out_left != NULL) {
        out_left->x = r.x;
        out_left->y = r.y;
        out_left->width = amount;
        out_left->height = r.height;
    }
    r.x += amount;
    r.width -= amount;
    return r;
}


// Cut Rectangle from right side by amount - out_right can be NULL.
Rectangle mui_cut_right(Rectangle r, float amount, Rectangle *out_right) {
    if (out_right != NULL) {
        out_right->x = r.x + r.width - amount;
        out_right->y = r.y;
        out_right->width = amount;
        out_right->height = r.height;
    }
    r.width -= amount;
    return r;
}


// Cut Rectangle from the top by amount - out_top can be NULL.
Rectangle mui_cut_top(Rectangle r, float amount, Rectangle *out_top) {
    if (out_top != NULL) {
        out_top->x = r.x;
        out_top->y = r.y;
        out_top->height = amount;
        out_top->width = r.width;
    }
    r.y += amount;
    r.height -= amount;
    return r;
}

// Cut Rectangle from the bot by amount - out_bot can be NULL.
Rectangle cut_bot(Rectangle r, float amount, Rectangle *out_bot) {
    if (out_bot != NULL) {
        out_bot->x = r.x;
        out_bot->y = r.y + r.height - amount;
        out_bot->height = amount;
        out_bot->width = r.width;
    }
    r.height -= amount;
    return r;
}

void mui_label(Mui_Theme *theme, char *text, Rectangle place) {
    if (theme == NULL) {
        theme = &mui_protos_theme;
    }

    DrawRectangleRounded(place, theme->roundedness, 10, theme->label_background_color);
    Vector2 position = {place.x+20, place.y + place.height/2 - theme->label_text_size/2};
    DrawTextEx(theme->label_font, text, position, theme->label_text_size, 0.1, theme->label_text_color);
}

void mui_checkbox(Mui_Checkbox_State *state, const char *text, Rectangle place) {

    Mui_Theme *theme = state->theme;
    if (theme == NULL) theme = &mui_protos_theme;

    // Update the time
    float dt = mui_time_now - state->last_time;
    state->last_time = mui_time_now;

    if (CheckCollisionPointRec(mui_mouse_pos, place)) {
        if (IsMouseButtonPressed(0)) {
            state->checked = !state->checked;
        }
        mui_move_towards(&(state->hover_t), 1, theme->animation_speed_to_hover, dt);
    } else {
        mui_move_towards(&(state->hover_t), 0, theme->animation_speed_to_normal, dt);
    }


    Rectangle area;     // a square for the mui_checkbox
    place = mui_cut_left(place, place.height, &area); // place of text

    Color border_color = mui_interpolate_color(theme->border_color, theme->border_hover_color, state->hover_t);
    DrawRectangleRounded(area, theme->roundedness, 10, border_color);
    area = mui_shrink(area, theme->border_thickness);

    Color bg = mui_interpolate_color(theme->background_color, theme->background_hover_color, state->hover_t);
    DrawRectangleRounded(area, theme->roundedness-0.25f*theme->roundedness, 10, bg);

    if (state->checked) {
        area = mui_shrink(area, area.height/6);
        DrawRectangleRounded(area, theme->roundedness, 10, border_color);
    }

    Color text_color = mui_interpolate_color(theme->text_color, theme->text_hover_color, state->hover_t);

    Vector2 position = {place.x+20, place.y + place.height/2 - theme->text_size/2};
    DrawTextEx(theme->label_font, text, position, theme->text_size, 0, text_color);
}

bool mui_button(Mui_Button_State *state, const char* text, Rectangle place) {

    Mui_Theme *theme = state->theme;
    if (theme == NULL) {
        theme = &mui_protos_theme;
    }

    // Update the time
    float dt = mui_time_now - state->last_time;
    state->last_time = mui_time_now;

    bool returnstate = false;

    if (CheckCollisionPointRec(mui_mouse_pos, place)) {
        if (IsMouseButtonPressed(0)) {
            returnstate = true;
        }
        mui_move_towards(&(state->hover_t), 1, theme->animation_speed_to_hover, dt);
    } else {
        mui_move_towards(&(state->hover_t), 0, theme->animation_speed_to_normal, dt);
    }

    Color bg;

    if (theme->border_thickness > 0) {
        bg = mui_interpolate_color(theme->border_color, theme->border_hover_color, state->hover_t);
        DrawRectangleRounded(place, theme->roundedness, 10, bg);
        place = mui_shrink(place, theme->border_thickness);
    }

    bg = mui_interpolate_color(theme->background_color, theme->background_hover_color, state->hover_t);
    DrawRectangleRounded(place, theme->roundedness-0.25f*theme->roundedness, 10, bg);


    Color text_color = mui_interpolate_color(theme->text_color, theme->text_hover_color, state->hover_t);

    float offset = place.height/2 - theme->text_size/2;
    Vector2 position = {place.x+offset, place.y + place.height/2 - theme->text_size/2};
    DrawTextEx(theme->label_font, text, position, theme->text_size, 0, text_color);
    return returnstate;
}

void mui_textinput(Mui_Textinput_State *state, const char *hint, Rectangle place) {

    Mui_Theme *theme = state->theme;
    if (theme == NULL) {
        theme = &mui_protos_theme;
    }

    if (IsMouseButtonPressed(0)) {
        if (CheckCollisionPointRec(mui_mouse_pos, place)) {
            state->active = true;
        } else {
            state->active = false;
        }
    }

    if (state->active) {
        int unicode_char = GetCharPressed();
        if (unicode_char != 0) {
            unsigned char c = unicode_char & 0xff; // TODO make it better 
            printf("Input %c\n", unicode_char & 0xff);
            if (state->count < MUI_TEXTINPUT_CAP) {
                state->buf[state->count] = c;
                state->buf[state->count+1] = 0;
                state->count++;
            }
        }
        
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
            if (state->count > 0) {
                state->count--;
                state->buf[state->count] = 0;
            }
        }
    }


    DrawRectangleRounded(place, theme->roundedness, 10, theme->border_color);
    place = mui_shrink(place, theme->border_thickness);
    DrawRectangleRounded(place, theme->roundedness, 10, theme->textinput_background_color);

    float text_offset_left = place.height/2 - theme->text_size/2;


    if (state->active) {
        // cursor
        Vector2 offset = MeasureTextEx(theme->textinput_font, state->buf, theme->textinput_text_size, 0);
        int pos = place.x + text_offset_left + offset.x;
        DrawRectangle(pos, place.y + place.height/2 - theme->textinput_text_size/2, 3, theme->textinput_text_size, theme->textinput_text_color);
    }
    

    if (state->count != 0 || state->active) {
        Vector2 position = {place.x + text_offset_left, place.y + place.height/2 - theme->textinput_text_size/2};
        DrawTextEx(theme->textinput_font, state->buf, position, theme->textinput_text_size, 0.1, theme->textinput_text_color);
    } else {
        //hint text
        Vector2 position = {place.x+ text_offset_left, place.y + place.height/2 - theme->textinput_text_size/2};
        DrawTextEx(theme->textinput_font, hint, position, theme->textinput_text_size, 0.1, theme->textinput_hint_text_color);
    }
}



//
// Some easing functions: See https://easings.net/
//

float mui_ease_in_quad(float t)      { return t * t; }
float mui_ease_out_quad(float t)     { return 1 - (1 - t) * (1 - t); }
float mui_ease_in_out_quad(float t)  { return t < 0.5f ? 2 * t * t: 1 - 2*(1 - t)*(1 - t); }
float mui_ease_in_cubic(float t)     { return t * t * t; }
float mui_ease_out_cubic(float t)    { return 1 - (1 - t) * (1 - t) * (1 - t); }
float mui_ease_in_out_cubic(float t) { return t < 0.5f ? 4 * t * t * t : 1 - 4*(1 - t)*(1 - t)*(1 - t); }
float mui_ease_in_sin(float t)       { return 1 - cosf((t * PI) * 0.5f); }
float mui_ease_out_sin(float t)      { return sinf((t * PI) *0.5f); }
float mui_ease_in_out_sin(float t)   { return - (cosf((t * PI) / 2) - 1) * 0.5f; }


#endif // MY_UI_IMPLEMENTATION


/* Usage example: mui_showcase.c:

#define MUI_IMPLEMENTATION
#include "mui.h"
#define NOB_IMPLEMENTATION
#include "nob.h"
#include "raylib.h"
#include "rlgl.h"

float mui_time_now;
Vector2 mui_mouse_pos;

int main(void)
{
    InitWindow(1270, 920, "Bada Bada Boom!");
    SetTargetFPS(200);
    mui_load_ttf_font("resources/fonts/UbuntuMono-Regular.ttf");
    Mui_Checkbox_State checkbox_state = {0};
    Mui_Button_State button_states[10] = {0};   

    bool active[5] = {false};

    while (!WindowShouldClose())
    {
        mui_time_now = GetTime();
        mui_mouse_pos = GetMousePosition();

        BeginDrawing();
            ClearBackground(RAYWHITE);

            Rectangle r = {130, 40, 710, 80};

            mui_label(&protos_theme, "Finally we have a working immediate UI! <3", r);

            Rectangle r2 = {130, 220, 410, 50};

            mui_checkbox(&checkbox_state, "Check me out pls!", r2);

            if (checkbox_state.checked) {
                for (int i = 0; i < 5; i++) {
                    Rectangle r = {130,400+60*i,410,50};
                    if (mui_button(&button_states[i], "Finally we bada bada boom", r)) {
                        active[i] = !active[i];
                    }
                }

                for (int i = 0; i < 5; i++) {
                    Rectangle r = {630,200+60*i,410,50};
                    if (active[i]) {
                        if (mui_button(&button_states[i+5], "Hello From This Side :)", r)) {
                            active[i] = false;
                        }
                    }
                }
            }

        DrawFPS(20,20);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

*/