#include <rtgui/rtgui.h>
#include <rtgui/dc.h>
#include <rtgui/image.h>
#include <rtgui/animation.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/window.h>
#include <rtgui/rtgui_app.h>

#include <dfs_posix.h>
#include <string.h>

#define PICTURE_DIR "/picture"

/* current picture file name */
rt_bool_t key_pressed = RT_FALSE;
static char current_fn[32] = {0};
static rtgui_timer_t *_the_timer;

/* One foreground and one background. */
static unsigned char _the_dc_buf_idx;
static unsigned char _anim_running;
static struct rtgui_dc_buffer *_the_dc_buf[2];
#define DCBUF_BG     ((struct rtgui_dc*)_the_dc_buf[_the_dc_buf_idx & 0x1])
#define DCBUF_FG     ((struct rtgui_dc*)_the_dc_buf[(_the_dc_buf_idx+1) & 0x1])

static struct rtgui_animation *_the_anim;
static struct rtgui_anim_engine_move_ctx _mv_engctx[8];
static struct rtgui_anim_engine_fade_ctx _mv_fadectx;
static struct rtgui_anim_engine_roto_ctx _rt_fadectx;
static struct
{
    rtgui_anim_engine eng;
    void *ctx;
} _eng_ctxs[24];

static void _picture_change(void)
{
    struct rtgui_rect rect;
    struct rtgui_image *image = RT_NULL;
    char fn[32];

    _the_dc_buf_idx++;

    if (_anim_running)
    {
        rtgui_anim_stop(_the_anim);
        rtgui_anim_set_cur_tick(_the_anim, 0);
    }
    _anim_running = 1;

    rect.x1 = rect.y1 = 0;
    rect.x2 = _the_dc_buf[0]->width;
    rect.y2 = _the_dc_buf[0]->height;

    /* open image */
    rt_snprintf(fn, sizeof(fn), "%s/%s", PICTURE_DIR, current_fn);
    //rt_kprintf("pic fn: %s\n", fn);
    image = rtgui_image_create(fn, RT_FALSE);
    if (image != RT_NULL)
    {
        /* blit image */
        rtgui_image_blit(image, DCBUF_FG, &rect);
        /* destroy image */
        rtgui_image_destroy(image);
    }
    else
    {
        rtgui_dc_fill_rect(DCBUF_FG, &rect);
        rtgui_dc_draw_text(DCBUF_FG, "没有文件被打开", &rect);
    }

    rtgui_anim_set_bg_buffer(_the_anim, DCBUF_BG);
    rtgui_anim_set_fg_buffer(_the_anim, DCBUF_FG, 1);

    rtgui_anim_start(_the_anim);
}

static void _on_anim_finish(struct rtgui_animation *anim, void *p)
{
    rtgui_anim_set_engine(_the_anim,
                          _eng_ctxs[_the_dc_buf_idx % (sizeof(_eng_ctxs)/sizeof(_eng_ctxs[0]))].eng,
                          _eng_ctxs[_the_dc_buf_idx % (sizeof(_eng_ctxs)/sizeof(_eng_ctxs[0]))].ctx);
    _anim_running = 0;
    rtgui_timer_start(_the_timer);
}

static void picture_show_prev(struct rtgui_widget *widget)
{
    DIR *dir;
    struct dirent *entry;
    rt_bool_t is_last;
    char fn[32];
    struct rtgui_image_engine *engine;

    fn[0] = '\0';
    is_last = RT_FALSE;

    dir = opendir(PICTURE_DIR);
    if (dir == RT_NULL)
    {
        rt_kprintf("open directory failed\n");
        return;
    }

    do {
        entry = readdir(dir);
        if (entry != RT_NULL)
        {
            engine = rtgui_image_get_engine_by_filename(entry->d_name);
            if (engine != RT_NULL)
            {
                if ((strcmp(entry->d_name, current_fn) == 0) &&
                    is_last != RT_TRUE)
                {
                    if (fn[0] == '\0')
                    {
                        /* it should be the last image */
                        is_last = RT_TRUE;
                    }
                    else
                    {
                        /* display image */
                        strncpy(current_fn, fn, sizeof(current_fn)-1);
                        _picture_change();
                        closedir(dir);
                        return;
                    }
                }
                strcpy(fn, entry->d_name);
            }
        }
    } while (entry != RT_NULL);

    /* close directory */
    closedir(dir);

    if ((is_last == RT_TRUE) && fn[0] != '\0')
    {
        strncpy(current_fn, fn, sizeof(current_fn)-1);
        _picture_change();
    }
}

static void picture_show_next(struct rtgui_widget *widget)
{
    DIR *dir;
    struct dirent *entry;
    rt_bool_t found, has_image;
    struct rtgui_image_engine *engine;

    found = RT_FALSE;
    has_image = RT_FALSE;

__restart:
    dir = opendir(PICTURE_DIR);
    if (dir == RT_NULL)
    {
        rt_kprintf("open directory failed\n");
        return;
    }

    do {
        entry = readdir(dir);
        if (entry != RT_NULL)
        {
            engine = rtgui_image_get_engine_by_filename(entry->d_name);
            if (engine != RT_NULL)
            {
                /* this directory includes image */
                has_image = RT_TRUE;

                if (found == RT_TRUE || current_fn[0] == '\0')
                {
                    strncpy(current_fn, entry->d_name, sizeof(current_fn)-1);
                    _picture_change();

                    closedir(dir);
                    return;
                }

                /* It's a current image. Mark @found so we will show next. */
                if (strcmp(entry->d_name, current_fn) == 0)
                    found = RT_TRUE;
            }
        }
    } while (entry != RT_NULL);

    /* close directory */
    closedir(dir);

    if (has_image != RT_TRUE)
        return;
    current_fn[0] = '\0';
    goto __restart;
}

static rt_bool_t onkey_handle(struct rtgui_object *object, struct rtgui_event *event)
{
    struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd *)event;

    if (ekbd->type == RTGUI_KEYDOWN)
    {
        if (ekbd->key == RTGUIK_RIGHT)
        {
            key_pressed = RT_TRUE;
            picture_show_next(RTGUI_WIDGET(object));
            return RT_TRUE;
        }
        else if (ekbd->key == RTGUIK_LEFT)
        {
            key_pressed = RT_TRUE;
            picture_show_prev(RTGUI_WIDGET(object));
            return RT_TRUE;
        }
    }
    return RT_TRUE;
}

static rt_bool_t picture_view_event_handler(rtgui_object_t *object, rtgui_event_t *event)
{
    if (event->type == RTGUI_EVENT_PAINT)
    {
        struct rtgui_dc *dc;
        struct rtgui_rect rect;
        struct rtgui_image *image = RT_NULL;
        char fn[32];

        if (_anim_running)
            return RT_FALSE;

        dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(object));
        if (dc == RT_NULL)
            return RT_FALSE;

        rtgui_widget_get_rect(RTGUI_WIDGET(object), &rect);

        /* open image */
        rt_snprintf(fn, sizeof(fn), "%s/%s", PICTURE_DIR, current_fn);
        //rt_kprintf("pic fn: %s\n", fn);
        image = rtgui_image_create(fn, RT_FALSE);

        if (image != RT_NULL)
        {
            /* blit image */
            rtgui_image_blit(image, dc, &rect);
            /* destroy image */
            rtgui_image_destroy(image);
        }
        else
        {
            rtgui_dc_fill_rect(dc, &rect);
            rtgui_dc_draw_text(dc, "没有文件被打开", &rect);
        }
        rtgui_dc_end_drawing(dc);

        return RT_FALSE;
    }

    return rtgui_win_event_handler(object, event);
}

static void _timeout(struct rtgui_timer *timer, void *parameter)
{
    struct rtgui_widget *widget;

    widget = (struct rtgui_widget *)parameter;

    if (key_pressed == RT_TRUE)
        key_pressed = RT_FALSE;
    else
        picture_show_next(widget);
}

void picture_show(void *parameter)
{
    /* create application */
    int i;
    struct rtgui_app *app;
    struct rtgui_win *win;
    struct rtgui_rect rect;

    app = rtgui_app_create("picture");
    if (app == RT_NULL)
    {
        rt_kprintf("Create application \"picture\" failed!\n");
        return;
    }

    /* create main window */
    win = rtgui_mainwin_create(RT_NULL, "main",
                               RTGUI_WIN_STYLE_NO_BORDER | RTGUI_WIN_STYLE_NO_TITLE);
    if (win == RT_NULL)
    {
        rt_kprintf("Create window \"main\" failed!\n");
        rtgui_app_destroy(app);
        return;
    }

    rtgui_widget_get_rect(RTGUI_WIDGET(win), &rect);
    _the_dc_buf[0] = (struct rtgui_dc_buffer*)rtgui_dc_buffer_create(rtgui_rect_width(rect),
                                                                     rtgui_rect_height(rect));
    if (!_the_dc_buf[0])
        goto _exit;
    _the_dc_buf[1] = (struct rtgui_dc_buffer*)rtgui_dc_buffer_create(rtgui_rect_width(rect),
                                                                     rtgui_rect_height(rect));
    if (!_the_dc_buf[1])
        goto _exit;

    _the_dc_buf_idx = 0;
    _anim_running = 0;

    //RTGUI_DC_FC(_the_dc_buf[DCBUF_BG]) = RTGUI_WIDGET_BACKGROUND(win);
    RTGUI_DC_BC(DCBUF_BG) = RTGUI_WIDGET_BACKGROUND(win);
    rtgui_dc_fill_rect(DCBUF_BG, &rect);
    RTGUI_DC_BC(DCBUF_FG) = RTGUI_WIDGET_BACKGROUND(win);
    rtgui_dc_fill_rect(DCBUF_FG, &rect);

    /* 25 frames per second should be reasonable. */
    _the_anim = rtgui_anim_create(RTGUI_WIDGET(win), RT_TICK_PER_SECOND/25);
    if (!_the_anim)
        goto _exit;

    rtgui_anim_set_bg_buffer(_the_anim, DCBUF_BG);
    rtgui_anim_set_fg_buffer(_the_anim, DCBUF_FG, 1);
    rtgui_anim_set_onfinish(_the_anim, _on_anim_finish, RT_NULL);
    rtgui_anim_set_duration(_the_anim, 1 * RT_TICK_PER_SECOND);
    rtgui_anim_set_motion(_the_anim, rtgui_anim_motion_insquare);
    rtgui_anim_set_engine(_the_anim, rtgui_anim_engine_move, &_mv_engctx[0]);

    _mv_fadectx.is_fade_out = 0;

    _rt_fadectx.pre_move.x = -rect.x2 / 2;
    _rt_fadectx.pre_move.y = -rect.y2 / 2;
    _rt_fadectx.from_degree = -180;
    _rt_fadectx.to_degree = 0;
    _rt_fadectx.post_move.x = rect.x2 / 2;
    _rt_fadectx.post_move.y = rect.y2 / 2;
    _rt_fadectx.use_aa = 0;

    _mv_engctx[0].start.x = -rtgui_rect_width(rect);
    _mv_engctx[0].start.y = 0;
    _mv_engctx[0].end.x   = 0;
    _mv_engctx[0].end.y   = 0;
    i = 0;
    _eng_ctxs[i].eng = rtgui_anim_engine_move;
    _eng_ctxs[i].ctx = &_mv_engctx[0];
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_fade;
    _eng_ctxs[i].ctx = &_mv_fadectx;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_roto;
    _eng_ctxs[i].ctx = &_rt_fadectx;

    _mv_engctx[1].start.x = 0;
    _mv_engctx[1].start.y = -rtgui_rect_height(rect);
    _mv_engctx[1].end.x   = 0;
    _mv_engctx[1].end.y   = 0;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_move;
    _eng_ctxs[i].ctx = &_mv_engctx[1];
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_fade;
    _eng_ctxs[i].ctx = &_mv_fadectx;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_roto;
    _eng_ctxs[i].ctx = &_rt_fadectx;

    _mv_engctx[2].start.x = -rtgui_rect_width(rect);
    _mv_engctx[2].start.y = -rtgui_rect_height(rect);
    _mv_engctx[2].end.x   = 0;
    _mv_engctx[2].end.y   = 0;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_move;
    _eng_ctxs[i].ctx = &_mv_engctx[2];
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_fade;
    _eng_ctxs[i].ctx = &_mv_fadectx;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_roto;
    _eng_ctxs[i].ctx = &_rt_fadectx;

    _mv_engctx[3].start.x = rtgui_rect_width(rect);
    _mv_engctx[3].start.y = 0;
    _mv_engctx[3].end.x   = 0;
    _mv_engctx[3].end.y   = 0;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_move;
    _eng_ctxs[i].ctx = &_mv_engctx[3];
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_fade;
    _eng_ctxs[i].ctx = &_mv_fadectx;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_roto;
    _eng_ctxs[i].ctx = &_rt_fadectx;

    _mv_engctx[4].start.x = 0;
    _mv_engctx[4].start.y = rtgui_rect_height(rect);
    _mv_engctx[4].end.x   = 0;
    _mv_engctx[4].end.y   = 0;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_move;
    _eng_ctxs[i].ctx = &_mv_engctx[4];
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_fade;
    _eng_ctxs[i].ctx = &_mv_fadectx;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_roto;
    _eng_ctxs[i].ctx = &_rt_fadectx;

    _mv_engctx[5].start.x = -rtgui_rect_width(rect);
    _mv_engctx[5].start.y = rtgui_rect_height(rect);
    _mv_engctx[5].end.x   = 0;
    _mv_engctx[5].end.y   = 0;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_move;
    _eng_ctxs[i].ctx = &_mv_engctx[5];
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_fade;
    _eng_ctxs[i].ctx = &_mv_fadectx;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_roto;
    _eng_ctxs[i].ctx = &_rt_fadectx;

    _mv_engctx[6].start.x = rtgui_rect_width(rect);
    _mv_engctx[6].start.y = -rtgui_rect_height(rect);
    _mv_engctx[6].end.x   = 0;
    _mv_engctx[6].end.y   = 0;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_move;
    _eng_ctxs[i].ctx = &_mv_engctx[6];
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_fade;
    _eng_ctxs[i].ctx = &_mv_fadectx;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_roto;
    _eng_ctxs[i].ctx = &_rt_fadectx;

    _mv_engctx[7].start.x = rtgui_rect_width(rect);
    _mv_engctx[7].start.y = rtgui_rect_height(rect);
    _mv_engctx[7].end.x   = 0;
    _mv_engctx[7].end.y   = 0;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_move;
    _eng_ctxs[i].ctx = &_mv_engctx[7];
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_fade;
    _eng_ctxs[i].ctx = &_mv_fadectx;
    i++;
    _eng_ctxs[i].eng = rtgui_anim_engine_roto;
    _eng_ctxs[i].ctx = &_rt_fadectx;

    _the_timer = rtgui_timer_create(RT_TICK_PER_SECOND / 2,
                                    RT_TIMER_FLAG_ONE_SHOT,
                                    _timeout, (void *)win);
    if (!_the_timer)
        goto _exit;

    rtgui_object_set_event_handler(RTGUI_OBJECT(win), picture_view_event_handler);
    //rtgui_win_set_onkey(win, onkey_handle);
    rtgui_win_show(win, RT_FALSE);

    /* show next picture */
    picture_show_next(RTGUI_WIDGET(win));

    rtgui_app_run(app);

_exit:
    if (_the_timer)
    {
        rtgui_timer_destory(_the_timer);
        _the_timer = RT_NULL;
    }
    if (win)
        rtgui_win_destroy(win);
    if (app)
        rtgui_app_destroy(app);
    if (_the_dc_buf[0])
    {
        rtgui_dc_destory((struct rtgui_dc*)_the_dc_buf[0]);
        _the_dc_buf[0] = RT_NULL;
    }
    if (_the_dc_buf[1])
    {
        rtgui_dc_destory((struct rtgui_dc*)_the_dc_buf[1]);
        _the_dc_buf[1] = RT_NULL;
    }
}

void picture_app_create(void *param)
{
    rt_thread_t tid;

    tid = rt_thread_create("pic", picture_show, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX - 2, 20);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
}

