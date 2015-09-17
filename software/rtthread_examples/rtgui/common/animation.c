#include <rtgui/dc.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/animation.h>

enum _anim_state
{
    _ANIM_STOPPED,
    _ANIM_RUNNING,
};

struct rtgui_animation
{
    struct rtgui_widget *parent;
    struct rtgui_timer  *timer;
    enum _anim_state state;

    struct rtgui_dc *bg_buf;
    struct rtgui_dc *fg_buf;
    int dc_cnt;

    unsigned int tick, tick_interval, max_tick;
    rtgui_anim_motion motion;

    rtgui_anim_engine engine;
    void *eng_ctx;

    rtgui_anim_onfinish on_finish;
	void *user_data;
};

int rtgui_anim_motion_linear(unsigned int tick, unsigned int max_tick)
{
    return tick * RTGUI_ANIM_TICK_RANGE / max_tick;
}
RTM_EXPORT(rtgui_anim_motion_linear);

int rtgui_anim_motion_insquare(unsigned int tick, unsigned int max_tick)
{
    /* Care about integer overflow. tick can within 0~(4G/RTGUI_ANIM_TICK_RANGE). */
    return tick * RTGUI_ANIM_TICK_RANGE / max_tick * tick / max_tick;
}
RTM_EXPORT(rtgui_anim_motion_insquare);

int rtgui_anim_motion_outsquare(unsigned int tick, unsigned int max_tick)
{
    /* Care about integer overflow. tick can within 0~(4G/RTGUI_ANIM_TICK_RANGE). */
    tick = max_tick - tick;
    return RTGUI_ANIM_TICK_RANGE - (tick * RTGUI_ANIM_TICK_RANGE / max_tick *
                                    tick / max_tick);
}
RTM_EXPORT(rtgui_anim_motion_outsquare);

static void _anim_timeout(struct rtgui_timer *timer, void *parameter)
{
    struct rtgui_dc *dc;
    struct rtgui_animation *anim = parameter;

    /* There maybe timeout event pending in the queue even if the timer has
     * been stopped. */
    if (anim->state != _ANIM_RUNNING)
        return;

    anim->tick += anim->tick_interval;
    if(anim->tick > anim->max_tick)
    {
        anim->tick = anim->max_tick;
    }

    RT_ASSERT(anim->parent);
    dc = rtgui_dc_begin_drawing(anim->parent);
    if (dc == RT_NULL)
        goto _end_draw;

    RT_ASSERT(anim->motion);
    RT_ASSERT(anim->engine);
    anim->engine(dc, anim->bg_buf, anim->fg_buf, anim->dc_cnt,
                 anim->motion(anim->tick, anim->max_tick),
                 anim->eng_ctx);

    rtgui_dc_end_drawing(dc);

_end_draw:
    if (anim->tick == anim->max_tick)
    {
        rtgui_anim_stop(anim);
        anim->tick = 0;
        if (anim->on_finish)
        {
            anim->on_finish(anim, anim->user_data);
        }
    }
}

static void _animation_default_finish(struct rtgui_animation* self, void* user_data)
{
	/* destroy animation in default */
	rtgui_anim_destroy(self);
}

struct rtgui_animation* rtgui_anim_create(struct rtgui_widget *parent,
                                          int interval)
{
    struct rtgui_animation *anim = rtgui_malloc(sizeof(*anim));

    if (anim == RT_NULL)
        return RT_NULL;

    anim->timer = rtgui_timer_create(interval, RT_TIMER_FLAG_PERIODIC,
                                     _anim_timeout, anim);
    if (anim->timer == RT_NULL)
    {
        rtgui_free(anim);
        return RT_NULL;
    }

    anim->parent = parent;

    anim->fg_buf = RT_NULL;
    anim->dc_cnt = 0;

    anim->tick = 0;
    anim->tick_interval = interval;
    anim->max_tick = 0;

    /* Set default handlers. */
    anim->motion = rtgui_anim_motion_linear;
    anim->engine = RT_NULL;
    anim->eng_ctx = RT_NULL;
    anim->on_finish = _animation_default_finish;
    anim->state = _ANIM_STOPPED;

    return anim;
}
RTM_EXPORT(rtgui_anim_create);

void rtgui_anim_destroy(struct rtgui_animation *anim)
{
    /* Only free animation and timer. If you want to free the dc_buffer,
     * overwrite the on_finish. */
    rtgui_timer_destory(anim->timer);
    rtgui_free(anim);
}
RTM_EXPORT(rtgui_anim_destroy);

struct rtgui_widget* rtgui_anim_get_owner(struct rtgui_animation *anim)
{
    RT_ASSERT(anim);

    return anim->parent;
}
RTM_EXPORT(rtgui_anim_get_owner);

void rtgui_anim_set_fg_buffer(struct rtgui_animation *anim,
                              struct rtgui_dc *dc,
                              int cnt)
{
    RT_ASSERT(anim);

    anim->fg_buf = dc;
    anim->dc_cnt = cnt;
}
RTM_EXPORT(rtgui_anim_set_fg_buffer);

struct rtgui_dc* rtgui_anim_get_fg_buffer(struct rtgui_animation *anim, int index)
{
	return &(anim->fg_buf[index]);
}
RTM_EXPORT(rtgui_anim_get_fg_buffer);

void rtgui_anim_set_bg_buffer(struct rtgui_animation *anim,
                              struct rtgui_dc *dc)
{
    RT_ASSERT(anim);

    anim->bg_buf = dc;
}
RTM_EXPORT(rtgui_anim_set_bg_buffer);

struct rtgui_dc* rtgui_anim_get_bg_buffer(struct rtgui_animation *anim)
{
    RT_ASSERT(anim);

	return anim->bg_buf;
}
RTM_EXPORT(rtgui_anim_get_bg_buffer);

void rtgui_anim_set_engine(struct rtgui_animation *anim,
                           rtgui_anim_engine engine,
                           void *ctx)
{
    RT_ASSERT(anim);

    anim->engine = engine;
    anim->eng_ctx = ctx;
}
RTM_EXPORT(rtgui_anim_set_engine);

void* rtgui_anim_get_engine_ctx(struct rtgui_animation *anim)
{
    RT_ASSERT(anim);

    return anim->eng_ctx;
}
RTM_EXPORT(rtgui_anim_get_engine_ctx);

unsigned int rtgui_anim_get_duration(struct rtgui_animation *anim)
{
    RT_ASSERT(anim);

    return anim->max_tick;
}
RTM_EXPORT(rtgui_anim_get_duration);

unsigned int rtgui_anim_get_cur_tick(struct rtgui_animation *anim)
{
    RT_ASSERT(anim);

    return anim->tick;
}
RTM_EXPORT(rtgui_anim_get_cur_tick);

void rtgui_anim_set_cur_tick(struct rtgui_animation *anim, unsigned int tick)
{
    RT_ASSERT(anim);

    if (tick > RTGUI_ANIM_TICK_RANGE)
        anim->tick = RTGUI_ANIM_TICK_RANGE;
    else
        anim->tick = tick;
}
RTM_EXPORT(rtgui_anim_set_cur_tick);

void rtgui_anim_set_onfinish(struct rtgui_animation *anim,
                             rtgui_anim_onfinish on_finish, void* user_data)
{
    RT_ASSERT(anim);

    anim->on_finish = on_finish;
	anim->user_data = user_data;
}
RTM_EXPORT(rtgui_anim_set_onfinish);

void rtgui_anim_set_motion(struct rtgui_animation *anim,
                           rtgui_anim_motion motion)
{
    RT_ASSERT(anim);

    anim->motion = motion;
}
RTM_EXPORT(rtgui_anim_set_motion);

void rtgui_anim_set_duration(struct rtgui_animation *anim,
                             unsigned int tick)
{
    RT_ASSERT(anim);

    anim->max_tick = tick;
}
RTM_EXPORT(rtgui_anim_set_duration);

void rtgui_anim_start(struct rtgui_animation *anim)
{
    RT_ASSERT(anim);

    if (anim->state == _ANIM_STOPPED)
    {
        anim->state = _ANIM_RUNNING;
        RTGUI_WIDGET_FLAG(anim->parent) |= RTGUI_WIDGET_FLAG_IN_ANIM;
        rtgui_timer_start(anim->timer);
    }
}
RTM_EXPORT(rtgui_anim_start);

void rtgui_anim_stop(struct rtgui_animation *anim)
{
    RT_ASSERT(anim);

    anim->state = _ANIM_STOPPED;
    RTGUI_WIDGET_FLAG(anim->parent) &= ~RTGUI_WIDGET_FLAG_IN_ANIM;
    rtgui_timer_stop(anim->timer);
}
RTM_EXPORT(rtgui_anim_stop);
