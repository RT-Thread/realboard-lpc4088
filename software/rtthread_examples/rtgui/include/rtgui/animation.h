#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "rtgui.h"
 
struct rtgui_dc;
struct rtgui_widget;
struct rtgui_animation;

#define RTGUI_ANIM_TICK_RANGE   (1024)

/* Return value should within 0~RTGUI_ANIM_TICK_RANGE */
typedef int (*rtgui_anim_motion)(unsigned int tick, unsigned int max_tick);
/* @progress is from 0 to RTGUI_ANIM_TICK_RANGE. */
typedef void (*rtgui_anim_engine)(struct rtgui_dc *background,
                                  struct rtgui_dc *background_buffer,
                                  struct rtgui_dc *items,
                                  int item_cnt,
                                  int progress,
                                  void *ctx);
typedef void (*rtgui_anim_onfinish)(struct rtgui_animation *anim, void* user_data);

/* Some common motion functions. */
/* y = a*x
 *
 * |       *
 * |     *
 * |   *
 * | *
 * -------------
 */
int rtgui_anim_motion_linear(unsigned int tick, unsigned int max_tick);
/* y = a*x^2
 *
 * |        *
 * |       *
 * |     *
 * |  *
 * -------------
 */
int rtgui_anim_motion_insquare(unsigned int tick, unsigned int max_tick);
/* y = b - a*(x - max_tick)^2
 *
 * |        *
 * |    *
 * |  *
 * | *
 * -------------
 */
int rtgui_anim_motion_outsquare(unsigned int tick, unsigned int max_tick);

/* Some common engines. */
struct rtgui_anim_engine_move_ctx
{
    struct rtgui_point start, end;
};
void rtgui_anim_engine_move(struct rtgui_dc *background,
                            struct rtgui_dc *background_buffer,
                            struct rtgui_dc *items,
                            int item_cnt,
                            int progress,
                            void *ctx);
struct rtgui_anim_engine_fade_ctx
{
    /* Context used internally. */
    int plvl;
    int is_fade_out;
};
void rtgui_anim_engine_fade(struct rtgui_dc *background,
                            struct rtgui_dc *background_buffer,
                            struct rtgui_dc *items,
                            int item_cnt,
                            int progress,
                            void *ctx);

struct rtgui_anim_engine_roto_ctx
{
    struct rtgui_point pre_move, post_move;
    double from_degree, to_degree;
    int use_aa;
};
void rtgui_anim_engine_roto(struct rtgui_dc *background,
                            struct rtgui_dc *background_buffer,
                            struct rtgui_dc *items,
                            int item_cnt,
                            int progress,
                            void *param);
/* Animation API. */

/** Create an animation instance.
 *
 * @parent the widget it paint to
 * @interval intervals between frames
 *
 * @return RT_NULL on failure.
 */
struct rtgui_animation* rtgui_anim_create(struct rtgui_widget *parent,
                                          int interval);
void rtgui_anim_destroy(struct rtgui_animation *anim);

struct rtgui_widget* rtgui_anim_get_owner(struct rtgui_animation *anim);

void rtgui_anim_set_bg_buffer(struct rtgui_animation *anim,
                              struct rtgui_dc *dc);
struct rtgui_dc* rtgui_anim_get_bg_buffer(struct rtgui_animation *anim);
/** Set the animation buffer to an animation.
 *
 * The dc buffer could be an array and @cnt should be set to the length of the
 * array.
 */
void rtgui_anim_set_fg_buffer(struct rtgui_animation *anim,
                              struct rtgui_dc *dc,
                              int cnt);
struct rtgui_dc* rtgui_anim_get_fg_buffer(struct rtgui_animation *anim, int index);

void rtgui_anim_set_motion(struct rtgui_animation *anim,
                           rtgui_anim_motion motion);

void rtgui_anim_set_engine(struct rtgui_animation *anim,
                           rtgui_anim_engine engine,
                           void *ctx);
void* rtgui_anim_get_engine_ctx(struct rtgui_animation *anim);

/** Set the callback function upon the animation has finished.
 *
 * The default behavior is just destroy the animation. If you want other
 * things, set your own callback with this function.
 */
void rtgui_anim_set_onfinish(struct rtgui_animation *anim,
                             rtgui_anim_onfinish on_finish, void* user_data);

void rtgui_anim_set_duration(struct rtgui_animation *anim,
                             unsigned int tick);
unsigned int rtgui_anim_get_duration(struct rtgui_animation *anim);
unsigned int rtgui_anim_get_cur_tick(struct rtgui_animation *anim);
void rtgui_anim_set_cur_tick(struct rtgui_animation *anim, unsigned int tick);

/** Start an animation.
 *
 * A stopped animation can be started again. It will start from the beginning
 * if normally stopped or resume from the last state if manually stopped.
 */
void rtgui_anim_start(struct rtgui_animation *anim);
void rtgui_anim_stop(struct rtgui_animation *anim);

#endif /* end of include guard: __ANIMATION_H__ */
