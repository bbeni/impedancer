#include "mui.h"
#include "math.h"

// globals need to be updated by calling mui_update_input()
float mui_global_time = 0.0f;
Mui_Vector2 mui_global_mouse_position = {0};

double mui_get_time() {return mui_global_time;}
Mui_Vector2 mui_get_mouse_position() {return mui_global_mouse_position;}
void mui_update_core() {
    mui_global_mouse_position = mui_get_mouse_position_now();
    mui_global_time = mui_get_time_now();
}

bool mui_load_resource_from_file(const char *file_path, int *out_size, void **data) {
	Nob_String_Builder sb = {0};
	if (!nob_read_entire_file(file_path, &sb)) {
		return false;
	}
	*data = sb.items;
	*out_size = sb.count;
	return true;
}

bool mui_load_ttf_font_for_theme(const char *font_file, Mui_Theme* theme) {

    int size;
    void *data;
    bool success = mui_load_resource_from_file(font_file, &size, &data);
    if (!success) {
        printf("My Error: loading resource (%s) so we are not loading fonts..\n", font_file);
        return false;
    }

    theme->font = mui_load_font_ttf(data, size, theme->text_size);
    theme->label_font = mui_load_font_ttf(data, size, theme->label_text_size);
    theme->textinput_font = mui_load_font_ttf(data, size, theme->textinput_text_size);

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

    .slider_thickness = 10.0f,
    .slider_wagon_width = 14.0f,
    .slider_wagon_height = 40.0f,
    .slider_wagon_corner_radius = 4.0f,
    .slider_wagon_border_thickness = 2.0f,

    .animation_speed_to_hover = 18.0f,
    .animation_speed_to_normal = 11.0f,
    .corner_radius = 10.0f,
};

Mui_Color mui_interpolate_color(Mui_Color a, Mui_Color b, float t) {
    
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
Mui_Rectangle mui_shrink(Mui_Rectangle r, float amount) {
    r.width  -= 2*amount;
    r.height -= 2*amount;
    r.x      += amount;
    r.y      += amount;
    return r;
}


// Cut Rectangle from left side by amount - out_left can be NULL.
Mui_Rectangle mui_cut_left(Mui_Rectangle r, float amount, Mui_Rectangle *out_left) {
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
Mui_Rectangle mui_cut_right(Mui_Rectangle r, float amount, Mui_Rectangle *out_right) {
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
Mui_Rectangle mui_cut_top(Mui_Rectangle r, float amount, Mui_Rectangle *out_top) {
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
Mui_Rectangle mui_cut_bot(Mui_Rectangle r, float amount, Mui_Rectangle *out_bot) {
    if (out_bot != NULL) {
        out_bot->x = r.x;
        out_bot->y = r.y + r.height - amount;
        out_bot->height = amount;
        out_bot->width = r.width;
    }
    r.height -= amount;
    return r;
}

bool mui_is_inside_rectangle(Mui_Vector2 pos, Mui_Rectangle rect) {
    return pos.x >= rect.x &&
           pos.x <  rect.x + rect.width &&
           pos.y >= rect.y &&
           pos.y <  rect.y + rect.height;
}

size_t mui_text_len(const char* text, size_t size) {
    // TODO: implement unicode, for now only ascii
    return size;
}

void mui_label(Mui_Theme *theme, char *text, Mui_Rectangle place) {
    if (theme == NULL) {
        theme = &mui_protos_theme;
    }

    mui_draw_rectangle_rounded(place, theme->corner_radius, theme->label_background_color);
    Mui_Vector2 position = {place.x+20, place.y + place.height/2 - theme->label_text_size/2};
    int l = mui_text_len(text, strlen(text));
    mui_draw_text_line(theme->label_font, position, 0.1, theme->label_text_size, text, theme->label_text_color, l);
}



void mui_checkbox(Mui_Checkbox_State *state, const char *text, Mui_Rectangle place) {

    Mui_Theme *theme = state->theme;
    if (theme == NULL) theme = &mui_protos_theme;

    // Update the time
    float dt = mui_get_time() - state->last_time;
    state->last_time = mui_get_time();

    if (mui_is_inside_rectangle(mui_get_mouse_position(), place)) {
        if (mui_is_mouse_button_pressed(0)) {
            state->checked = !state->checked;
        }
        mui_move_towards(&(state->hover_t), 1, theme->animation_speed_to_hover, dt);
    } else {
        mui_move_towards(&(state->hover_t), 0, theme->animation_speed_to_normal, dt);
    }


    Mui_Rectangle area;     // a square for the mui_checkbox
    place = mui_cut_left(place, place.height, &area); // place of text

    Mui_Color bg = mui_interpolate_color(theme->background_color, theme->background_hover_color, state->hover_t);
    Mui_Color border_color = mui_interpolate_color(theme->border_color, theme->border_hover_color, state->hover_t);
    Mui_Color text_color = mui_interpolate_color(theme->text_color, theme->text_hover_color, state->hover_t);

    mui_draw_rectangle_rounded(area, theme->corner_radius, bg);
    mui_draw_rectangle_rounded_lines(area, theme->corner_radius, border_color, theme->border_thickness);

    if (state->checked) {
        area = mui_shrink(area, area.height/6);
        mui_draw_rectangle_rounded(area, theme->corner_radius, border_color);
    }

    Mui_Vector2 position = {place.x+20, place.y + place.height/2 - theme->text_size/2};
    size_t l = mui_text_len(text, strlen(text));
    mui_draw_text_line(theme->font, position, 0, theme->text_size, text, text_color, l);
}

float mui_simple_slider(Mui_Slider_State *state, bool vertical, Mui_Rectangle place) {

    Mui_Theme *theme = state->theme;
    if (theme == NULL) theme = &mui_protos_theme;

    // Update the time
    float dt = mui_get_time() - state->last_time;
    state->last_time = mui_get_time();
    Mui_Vector2 mpos = mui_get_mouse_position();

    if (mui_is_inside_rectangle(mpos, place)) {
        mui_move_towards(&(state->hover_t), 1, theme->animation_speed_to_hover, dt);
        if (mui_is_mouse_button_pressed(0)) {
            state->grabbed = true;
        }
    } else {
        mui_move_towards(&(state->hover_t), 0, theme->animation_speed_to_normal, dt);
    }

    if (mui_is_mouse_button_up(0)) {state->grabbed = false;}

    if (state->grabbed) {
        
        float s;
        if (vertical) s = min(max(0.0f, (mpos.y - place.y) / place.height), 1.0f);
        else          s = min(max(0.0f, (mpos.x - place.x) / place.width), 1.0f);
        state->value = s;
        mui_move_towards(&(state->hover_t), 1, theme->animation_speed_to_hover, dt);
    }

    Mui_Color bg = mui_interpolate_color(theme->background_color, theme->background_hover_color, state->hover_t);
    Mui_Color border_color = mui_interpolate_color(theme->border_color, theme->border_hover_color, state->hover_t);
    Mui_Color text_color = mui_interpolate_color(theme->text_color, theme->text_hover_color, state->hover_t);

    if (!vertical) { // horizontal
        // [..]...........
        // ...........[..]
        // 0           -> place at 0
        // palce.width -> place at place.width - theme.slider
        float w = place.width - theme->slider_wagon_width;

        //rail
        Mui_Rectangle rail = mui_cut_top(place, place.height*0.5f - 0.5f*theme->slider_thickness, NULL);
        rail = mui_cut_bot(rail, place.height*0.5f - 0.5f*theme->slider_thickness, NULL);
        rail = mui_cut_left(rail, theme->slider_wagon_width*0.5f, NULL); 
        rail = mui_cut_right(rail, theme->slider_wagon_width*0.5f, NULL);
        mui_draw_rectangle_rounded(rail, theme->slider_thickness*0.5f, text_color);

        Mui_Rectangle wagon = {
            .x = state->value * w + place.x,
            .y = place.y + 0.5f*place.height - 0.5f*theme->slider_wagon_height,
            .width = theme->slider_wagon_width,
            .height = theme->slider_wagon_height,
        };

        mui_draw_rectangle_rounded(wagon, theme->slider_wagon_corner_radius, bg);
        mui_draw_rectangle_rounded_lines(wagon, theme->slider_wagon_corner_radius, border_color, theme->slider_wagon_border_thickness);
    } else { // verical
        float w = place.height - theme->slider_wagon_width;

        //rail
        Mui_Rectangle rail = mui_cut_left(place, place.width*0.5f - 0.5f*theme->slider_thickness, NULL);
        rail = mui_cut_right(rail, place.width*0.5f - 0.5f*theme->slider_thickness, NULL);
        rail = mui_cut_top(rail, theme->slider_wagon_width*0.5f, NULL); 
        rail = mui_cut_bot(rail, theme->slider_wagon_width*0.5f, NULL);
        mui_draw_rectangle_rounded(rail, theme->slider_thickness*0.5f, text_color);

        Mui_Rectangle wagon = {
            .x = place.x + 0.5f*place.width - 0.5f*theme->slider_wagon_height,
            .y = state->value * w + place.y,
            .width = theme->slider_wagon_height,
            .height = theme->slider_wagon_width,
        };

        mui_draw_rectangle_rounded(wagon, theme->slider_wagon_corner_radius, bg);
        mui_draw_rectangle_rounded_lines(wagon, theme->slider_wagon_corner_radius, border_color, theme->slider_wagon_border_thickness);
    }
}


bool mui_button(Mui_Button_State *state, const char* text, Mui_Rectangle place) {

    Mui_Theme *theme = state->theme;
    if (theme == NULL) {
        theme = &mui_protos_theme;
    }

    // Update the time
    float dt = mui_get_time() - state->last_time;
    state->last_time = mui_get_time();

    bool returnstate = false;

    if (mui_is_inside_rectangle(mui_get_mouse_position(), place)) {
        if (mui_is_mouse_button_pressed(0)) {
            returnstate = true;
        }
        mui_move_towards(&(state->hover_t), 1, theme->animation_speed_to_hover, dt);
    } else {
        mui_move_towards(&(state->hover_t), 0, theme->animation_speed_to_normal, dt);
    }

    Mui_Color bg;

    if (theme->border_thickness > 0) {
        bg = mui_interpolate_color(theme->border_color, theme->border_hover_color, state->hover_t);
        mui_draw_rectangle_rounded(place, theme->corner_radius, bg);
        place = mui_shrink(place, theme->border_thickness);
    }

    bg = mui_interpolate_color(theme->background_color, theme->background_hover_color, state->hover_t);
    mui_draw_rectangle_rounded(place, theme->corner_radius-0.25f*theme->corner_radius, bg);

    Mui_Color text_color = mui_interpolate_color(theme->text_color, theme->text_hover_color, state->hover_t);

    float offset = place.height/2 - theme->text_size/2;
    Mui_Vector2 position = {place.x+offset, place.y + place.height/2 - theme->text_size/2};
    size_t l = mui_text_len(text, strlen(text));
    mui_draw_text_line(theme->label_font, position, 0, theme->text_size, text, text_color, l);
    return returnstate;
}

void mui_textinput_multiline(Mui_Textinput_Multiline_State *state, const char *hint, Mui_Rectangle place) {


    Mui_Theme *theme = state->theme;
    if (theme == NULL) {
        theme = &mui_protos_theme;
    }

    if (mui_is_mouse_button_pressed(0)) {
        if (mui_is_inside_rectangle(mui_get_mouse_position(), place)) {
            state->active = true;
        } else {
            state->active = false;
        }
    }

    if (state->buffer.count < 1) {
        nob_da_append(&(state->buffer), '\0');
    }

    if (state->active) {
        int unicode_char = mui_get_char_pressed();
        if (unicode_char != 0) {
            unsigned char c = unicode_char & 0xff; // TODO make it better 
        
            // add at the end and then swap
            nob_da_append(&(state->buffer), '\0');
            assert(state->cursor < state->buffer.count-1);

            for( int i = state->buffer.count-2; i > state->cursor; i--) {
                state->buffer.items[i] = state->buffer.items[i-1];
            }

            printf("Input %c\n", unicode_char & 0xff);
            state->buffer.items[state->cursor] = c;
            state->cursor++;
        }
        
    }


    mui_draw_rectangle_rounded(place, theme->corner_radius, theme->border_color);
    place = mui_shrink(place, theme->border_thickness);
    mui_draw_rectangle_rounded(place, theme->corner_radius, theme->textinput_background_color);

    float text_offset_left = theme->textinput_text_size*0.1f;
    float text_offset_top = theme->textinput_text_size*0.1f;

    // cursor
    Mui_Rectangle rect_cursor = {
        .x=place.x + text_offset_left, 
        .y=place.y + text_offset_top,
        .width=3,
        .height=theme->textinput_text_size
    };

    if (state->buffer.count > 1 || state->active) {
        const float line_spacing = 1.1f;

        int line_nr = 0;
        char* line_start = state->buffer.items;

        Mui_Vector2 line_size = {0};

        for (int i=0; i < state->buffer.count; i++) {
            char* buf = &state->buffer.items[i];
            line_size = mui_measure_text(theme->textinput_font, line_start, theme->textinput_text_size, 0.1f, buf-line_start);

            // draw it
            if (line_size.x >= place.width - text_offset_left || *buf == '\n' || i==state->buffer.count-1) {
                char* orig = buf;
                if(i!=state->buffer.count-1) {
                    buf--; // we overshot, so we do backtracking
                }
                    
                Mui_Vector2 position = {place.x + text_offset_left, place.y + text_offset_top + line_nr*theme->textinput_text_size*line_spacing};
                size_t l = buf-line_start;
                mui_draw_text_line(theme->textinput_font, position, 0.1, theme->textinput_text_size, line_start, theme->textinput_text_color, l);

                if (*orig == "\n") buf += 2; // skip new line and backtracking
                line_start = buf;
                line_nr++;
            }

            // check for cursor
            if (i == state->cursor) {
                rect_cursor.x += line_size.x;
                rect_cursor.y += (line_nr-1)*theme->textinput_text_size*line_spacing;
            }

            nob_temp_reset();
        }

        // cursor edge case at the end of buffer
        if (state->cursor == state->buffer.count) {
            rect_cursor.x += line_size.x;
            rect_cursor.y += line_nr*theme->textinput_text_size*line_spacing;
        }

    } else {
        //hint text
        size_t l = mui_text_len(hint, strlen(hint));
        Mui_Vector2 position = {place.x+ text_offset_left, place.y + text_offset_top};
        mui_draw_text_line(theme->textinput_font, position, 0.1f, theme->textinput_text_size, hint, theme->textinput_text_color, l);
    }

    // draw cursor
    if (state->active) {
        mui_draw_rectangle(rect_cursor, theme->textinput_text_color);
    }
}


void mui_textinput(Mui_Textinput_State *state, const char *hint, Mui_Rectangle place) {

    Mui_Theme *theme = state->theme;
    if (theme == NULL) {
        theme = &mui_protos_theme;
    }

    if (mui_is_mouse_button_pressed(0)) {
        if (mui_is_inside_rectangle(mui_get_mouse_position(), place)) {
            state->active = true;
        } else {
            state->active = false;
        }
    }

    if (state->active) {
        int unicode_char = mui_get_char_pressed();
        if (unicode_char != 0) {
            unsigned char c = unicode_char & 0xff; // TODO make it better 
            printf("Input %c\n", unicode_char & 0xff);
            if (state->count < MUI_TEXTINPUT_CAP) {
                state->buf[state->count] = c;
                state->buf[state->count+1] = 0;
                state->count++;
            }
        }
        
        if (mui_is_key_pressed(MUI_KEY_BACKSPACE) || mui_is_key_pressed_repeat(MUI_KEY_BACKSPACE)) {
            if (state->count > 0) {
                state->count--;
                state->buf[state->count] = 0;
            }
        }
    }


    mui_draw_rectangle_rounded(place, theme->corner_radius, theme->border_color);
    place = mui_shrink(place, theme->border_thickness);
    mui_draw_rectangle_rounded(place, theme->corner_radius, theme->textinput_background_color);

    float text_offset_left = place.height/2 - theme->text_size/2;


    if (state->active) {
        // cursor
        Mui_Vector2 offset = mui_measure_text(theme->textinput_font, state->buf, theme->textinput_text_size, 0, state->count-1);
        int pos = place.x + text_offset_left + offset.x;
        Mui_Rectangle rc = {
            .x=pos, 
            .y=place.y + place.height/2 - theme->textinput_text_size/2,
            .width=3,
            .height=theme->textinput_text_size
        };

        mui_draw_rectangle(rc, theme->textinput_text_color);
    }
    

    if (state->count != 0 || state->active) {
        size_t l = mui_text_len(state->buf, strlen(state->buf));
        Mui_Vector2 position = {place.x + text_offset_left, place.y + place.height/2 - theme->textinput_text_size/2};
        mui_draw_text_line(theme->textinput_font, position, 0.1, theme->textinput_text_size, state->buf, theme->textinput_text_color, l);
    } else {
        //hint text
        size_t l = mui_text_len(hint, strlen(hint));
        Mui_Vector2 position = {place.x+ text_offset_left, place.y + place.height/2 - theme->textinput_text_size/2};
        mui_draw_text_line(theme->textinput_font, position, 0.1, theme->textinput_text_size, hint, theme->textinput_text_color, l);
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
float mui_ease_in_sin(float t)       { return 1 - cosf((t * M_PI) * 0.5f); }
float mui_ease_out_sin(float t)      { return sinf((t * M_PI) *0.5f); }
float mui_ease_in_out_sin(float t)   { return - (cosf((t * M_PI) / 2) - 1) * 0.5f; }