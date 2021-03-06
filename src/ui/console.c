#include <assert.h>

#include "game/level.h"
#include "game/level/player/rigid_rect.h"
#include "script/gc.h"
#include "script/interpreter.h"
#include "script/parser.h"
#include "script/scope.h"
#include "sdl/renderer.h"
#include "system/error.h"
#include "system/lt.h"
#include "ui/console.h"
#include "ui/edit_field.h"
#include "ui/history.h"
#include "ui/log.h"

#define FONT_WIDTH_SCALE 3.0f
#define FONT_HEIGHT_SCALE 3.0f

#define LOG_CAPACITY 10
#define HISTORY_CAPACITY 20
#define PROMPT_HEIGHT (FONT_HEIGHT_SCALE * FONT_CHAR_HEIGHT)
#define LOG_HEIGHT (FONT_HEIGHT_SCALE * FONT_CHAR_HEIGHT * LOG_CAPACITY)

#define CONSOLE_HEIGHT (LOG_HEIGHT + PROMPT_HEIGHT)

#define SLIDE_DOWN_TIME 0.4f

#define CONSOLE_ALPHA (0.80f)
#define CONSOLE_BACKGROUND (color(0.20f, 0.20f, 0.20f, CONSOLE_ALPHA))
#define CONSOLE_FOREGROUND (color(0.80f, 0.80f, 0.80f, CONSOLE_ALPHA))
#define CONSOLE_ERROR (color(0.80f, 0.50f, 0.50f, CONSOLE_ALPHA))

#define CONSOLE_EVAL_RESULT_SIZE 256

struct Console
{
    Lt *lt;
    Gc *gc;
    struct Scope scope;
    Edit_field *edit_field;
    Log *log;
    Level *level;
    History *history;
    float a;
    char *eval_result;
};

/* TODO(#354): Console does not allow to travel the history by pressing up and down */
/* TODO(#355): Console does not support Emacs keybindings */
/* TODO(#356): Console does not support autocompletion */
/* TODO(#357): Console does not show the state of the GC of the script */
/* TODO(#358): Console does not support copy, cut, paste operations */

static struct EvalResult rect_apply_force(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    assert(param);

    /* TODO(#401): rect_apply_force doesn't sanitize it's input */

    Level *level = (Level*) param;
    const char *rect_id = CAR(args).atom->str;
    struct Expr vector_force_expr = CAR(CDR(args));
    const float force_x = (float) CAR(vector_force_expr).atom->num;
    const float force_y = (float) CDR(vector_force_expr).atom->num;

    print_expr_as_sexpr(stdout, args); printf("\n");

    Rigid_rect *rigid_rect = level_rigid_rect(level, rect_id);
    if (rigid_rect != NULL) {
        printf("Found rect `%s`\n", rect_id);
        printf("Applying force (%f, %f)\n", force_x, force_y);
        rigid_rect_apply_force(rigid_rect, vec(force_x, force_y));
    } else {
        fprintf(stderr, "Couldn't find rigid_rect `%s`", rect_id);
    }

    return eval_success(NIL(gc));
}

Console *create_console(Level *level,
                        const Sprite_font *font)
{
    Lt *lt = create_lt();

    if (lt == NULL) {
        return NULL;
    }

    Console *console = PUSH_LT(lt, malloc(sizeof(Console)), free);
    if (console == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    console->lt = lt;

    console->gc = PUSH_LT(lt, create_gc(), destroy_gc);
    if (console->gc == NULL) {
        RETURN_LT(lt, NULL);
    }

    console->scope.expr = CONS(console->gc,
                               NIL(console->gc),
                               NIL(console->gc));
    set_scope_value(
        console->gc,
        &console->scope,
        SYMBOL(console->gc, "rect-apply-force"),
        NATIVE(console->gc, rect_apply_force, level));

    console->edit_field = PUSH_LT(
        lt,
        create_edit_field(
            font,
            vec(FONT_WIDTH_SCALE, FONT_HEIGHT_SCALE),
            CONSOLE_FOREGROUND),
        destroy_edit_field);
    if (console->edit_field == NULL) {
        RETURN_LT(lt, NULL);
    }

    console->log = PUSH_LT(
        lt,
        create_log(
            font,
            vec(FONT_WIDTH_SCALE, FONT_HEIGHT_SCALE),
            LOG_CAPACITY),
        destroy_log);

    console->level = level;
    console->a = 0;

    console->eval_result = PUSH_LT(
        lt,
        malloc(sizeof(char) * CONSOLE_EVAL_RESULT_SIZE),
        free);
    if (console->eval_result == NULL) {
        RETURN_LT(lt, NULL);
    }
    memset(console->eval_result, 0, sizeof(char) * CONSOLE_EVAL_RESULT_SIZE);

    console->history = PUSH_LT(
        lt,
        create_history(HISTORY_CAPACITY),
        destroy_history);
    if (console->history == NULL) {
        RETURN_LT(lt, NULL);
    }

    return console;
}

void destroy_console(Console *console)
{
    assert(console);
    RETURN_LT0(console->lt);
}

int console_handle_event(Console *console,
                         const SDL_Event *event)
{
    switch(event->type) {
    case SDL_KEYDOWN:
        switch(event->key.keysym.sym) {
        case SDLK_RETURN: {
            const char *source_code = edit_field_as_text(console->edit_field);

            /* TODO(#387): console pushes empty strings to the history */
            if (history_push(console->history, source_code) < 0) {
                return -1;
            }

            if (log_push_line(console->log, source_code, CONSOLE_FOREGROUND) < 0) {
                return -1;
            }

            struct ParseResult parse_result = read_expr_from_string(console->gc,
                                                                    source_code);

            if (parse_result.is_error) {
                if (log_push_line(console->log, parse_result.error_message, CONSOLE_ERROR)) {
                    return -1;
                }

                edit_field_clean(console->edit_field);

                return 0;
            }

            struct EvalResult eval_result = eval(
                console->gc,
                &console->scope,
                parse_result.expr);

            if (expr_as_sexpr(
                    eval_result.expr,
                    console->eval_result,
                    CONSOLE_EVAL_RESULT_SIZE) < 0) {
                return -1;
            }

            if (log_push_line(console->log,
                              console->eval_result,
                              eval_result.is_error ?
                              CONSOLE_ERROR :
                              CONSOLE_FOREGROUND)) {
                return -1;
            }

            gc_collect(console->gc, console->scope.expr);
            edit_field_clean(console->edit_field);
        } return 0;

        case SDLK_UP: {
            edit_field_replace(
                console->edit_field,
                history_current(console->history));
            history_prev(console->history);
        } return 0;

        case SDLK_DOWN: {
            edit_field_replace(
                console->edit_field,
                history_current(console->history));
            history_next(console->history);
        } return 0;
        }
        break;
    }

    return edit_field_handle_event(console->edit_field, event);
}

int console_render(const Console *console,
                   SDL_Renderer *renderer)
{
    /* TODO(#364): console doesn't have any padding around the edit fields */
    SDL_Rect view_port;
    SDL_RenderGetViewport(renderer, &view_port);

    const float e = console->a * (2 - console->a);
    const float y = -(1.0f - e) * CONSOLE_HEIGHT;

    if (fill_rect(renderer,
                  rect(0.0f, y,
                       (float) view_port.w,
                       CONSOLE_HEIGHT),
                  CONSOLE_BACKGROUND) < 0) {
        return -1;
    }

    if (log_render(console->log,
                   renderer,
                   vec(0.0f, y)) < 0) {
        return -1;
    }

    if (edit_field_render(console->edit_field,
                          renderer,
                          vec(0.0f, y + LOG_HEIGHT)) < 0) {
        return -1;
    }

    return 0;
}

int console_update(Console *console, float delta_time)
{
    assert(console);

    /* TODO(#366): console slide down animation doesn't have any easing */

    if (console->a < 1.0f) {
        console->a += 1.0f / SLIDE_DOWN_TIME * delta_time;

        if (console->a > 1.0f) {
            console->a = 1.0f;
        }
    }

    return 0;
}

void console_slide_down(Console *console)
{
    assert(console);
    console->a = 0.0f;
}
