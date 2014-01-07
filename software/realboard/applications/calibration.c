#include <rtgui/rtgui.h>
#include <rtgui/dc.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/widgets/window.h>
#include <calibration.h>

static rt_bool_t (*_cali_restore)(void);
void calibration_set_restore(rt_bool_t (*calibration_restore)(void))
{
    _cali_restore = calibration_restore;
}

static rt_bool_t (*_cali_after)(calibration_typedef *data);
void calibration_set_after(rt_bool_t (*calibration_after)(calibration_typedef *data))
{
    _cali_after = calibration_after;
}

calibration_typedef *cal_data;

#define CALIBRATION_STEP_LEFTTOP        0
#define CALIBRATION_STEP_RIGHTTOP       1
#define CALIBRATION_STEP_RIGHTBOTTOM    2
#define CALIBRATION_STEP_LEFTBOTTOM     3
#define CALIBRATION_STEP_CENTER         4

#define TOUCH_WIN_UPDATE    1
#define TOUCH_WIN_CLOSE     2

#define CALIBRATION_WIDTH   15
#define CALIBRATION_HEIGHT  15

struct calibration_session
{
    rt_uint8_t step;
    rt_uint16_t width;
    rt_uint16_t height;

    rtgui_win_t *win;

    rt_device_t device;

    struct rtgui_app *app;
};
static struct calibration_session *calibration_ptr = RT_NULL;

static int perform_calibration(calibration_typedef *cal)
{
    int j;
    float n, x, y, x2, y2, xy, z, zx, zy;
    float det, a, b, c, e, f, i;
    float scaling = 65536.0;

// Get sums for matrix
    n = x = y = x2 = y2 = xy = 0;
    for (j = 0; j < 5; j++)
    {
        n += 1.0f;
        x += (float)cal->x[j];
        y += (float)cal->y[j];
        x2 += (float)(cal->x[j] * cal->x[j]);
        y2 += (float)(cal->y[j] * cal->y[j]);
        xy += (float)(cal->x[j] * cal->y[j]);
    }

// Get determinant of matrix -- check if determinant is too small
    det = n * (x2 * y2 - xy * xy) + x * (xy * y - x * y2) + y * (x * xy - y * x2);
    if (det < 0.1f && det > -0.1)
    {
        return 0;
    }

// Get elements of inverse matrix
    a = (x2 * y2 - xy * xy) / det;
    b = (xy * y - x * y2) / det;
    c = (x * xy - y * x2) / det;
    e = (n * y2 - y * y) / det;
    f = (x * y - n * xy) / det;
    i = (n * x2 - x * x) / det;

// Get sums for x calibration
    z = zx = zy = 0;
    for (j = 0; j < 5; j++)
    {
        z += (float)cal->xfb[j];
        zx += (float)(cal->xfb[j] * cal->x[j]);
        zy += (float)(cal->xfb[j] * cal->y[j]);
    }

// Now multiply out to get the calibration for framebuffer x coord
    cal->a[2] = (int)((a * z + b * zx + c * zy) * (scaling));
    cal->a[0] = (int)((b * z + e * zx + f * zy) * (scaling));
    cal->a[1] = (int)((c * z + f * zx + i * zy) * (scaling));

// Get sums for y calibration
    z = zx = zy = 0;
    for (j = 0; j < 5; j++)
    {
        z += (float)cal->yfb[j];
        zx += (float)(cal->yfb[j] * cal->x[j]);
        zy += (float)(cal->yfb[j] * cal->y[j]);
    }

// Now multiply out to get the calibration for framebuffer y coord
    cal->a[5] = (int)((a * z + b * zx + c * zy) * (scaling));
    cal->a[3] = (int)((b * z + e * zx + f * zy) * (scaling));
    cal->a[4] = (int)((c * z + f * zx + i * zy) * (scaling));

// If we got here, we're OK, so assign scaling to a[6] and return
    cal->a[6] = (int)scaling;
    return 1;
}
rt_uint16_t  Calibrate_X(rt_uint16_t ad_x, rt_uint16_t ad_y)
{
    rt_uint16_t temp;
    temp = (rt_uint16_t)((ad_x * cal_data->a[0] + ad_y * cal_data->a[1] + cal_data->a[2]) / cal_data->a[6]);
    return temp;
}
rt_uint16_t  Calibrate_Y(rt_uint16_t ad_x, rt_uint16_t ad_y)
{
    rt_uint16_t temp;
    temp = (rt_uint16_t)((ad_x * cal_data->a[3] + ad_y * cal_data->a[4] + cal_data->a[5]) / cal_data->a[6]);
    return temp;
}
void calibration_set_data(calibration_typedef *data)
{
 cal_data=data;
}
static void calibration_data_post(rt_uint16_t x, rt_uint16_t y)
{
    if (calibration_ptr == RT_NULL)
        return;

    switch (calibration_ptr->step)
    {
    case CALIBRATION_STEP_LEFTTOP:
        cal_data->xfb[0] = CALIBRATION_WIDTH;
        cal_data->yfb[0] = CALIBRATION_HEIGHT;
        cal_data->x[0] = x;
        cal_data->y[0] = y;
        break;

    case CALIBRATION_STEP_RIGHTTOP:
        cal_data->xfb[1] = calibration_ptr->width - CALIBRATION_WIDTH;
        cal_data->yfb[1] =  CALIBRATION_HEIGHT;
        cal_data->x[1] = x;
        cal_data->y[1] = y;
        break;

    case CALIBRATION_STEP_LEFTBOTTOM:
        cal_data->xfb[3] = CALIBRATION_WIDTH;
        cal_data->yfb[3] = calibration_ptr->height - CALIBRATION_HEIGHT;
        cal_data->x[3] = x;
        cal_data->y[3] = y;
        break;

    case CALIBRATION_STEP_RIGHTBOTTOM:
        cal_data->xfb[2] = calibration_ptr->width - CALIBRATION_WIDTH;
        cal_data->yfb[2] = calibration_ptr->height - CALIBRATION_HEIGHT;
        cal_data->x[2] = x;
        cal_data->y[2] = y;
        break;

    case CALIBRATION_STEP_CENTER:
        /* calibration done */
    {
        rt_uint8_t i;
        struct rtgui_event_command ecmd;
        RTGUI_EVENT_COMMAND_INIT(&ecmd);
        ecmd.wid = calibration_ptr->win;
        ecmd.command_id = TOUCH_WIN_CLOSE;
        cal_data->xfb[4] = (calibration_ptr->width >> 1);
        cal_data->yfb[4] = (calibration_ptr->height >> 1);
        cal_data->x[4] = x;
        cal_data->y[4] = y;
        for (i = 0; i < 5; i++)
        {
            rt_kprintf("xfb[%d]:%d,yfb[%d]:%d,x[%d]:%d,y[%d]:%d\r\n", i, cal_data->xfb[i], i, cal_data->yfb[i], i, cal_data->x[i], i, cal_data->y[i]);
        }
        perform_calibration(cal_data);
        rtgui_send(calibration_ptr->app, &ecmd.parent, sizeof(struct rtgui_event_command));
    }
    calibration_ptr->step = 0;
    return;
    }

    calibration_ptr->step++;

    /* post command event */
    {
        struct rtgui_event_command ecmd;
        RTGUI_EVENT_COMMAND_INIT(&ecmd);
        ecmd.wid = calibration_ptr->win;
        ecmd.command_id = TOUCH_WIN_UPDATE;

        rtgui_send(calibration_ptr->app, &ecmd.parent, sizeof(struct rtgui_event_command));
    }
}

static rt_bool_t calibration_event_handler(struct rtgui_object *object, struct rtgui_event *event)
{
    struct rtgui_widget *widget = RTGUI_WIDGET(object);
    rtgui_rect_t label_rect = {30, 120, 420, 140};
    switch (event->type)
    {
    case RTGUI_EVENT_PAINT:
    {
        struct rtgui_dc *dc;
        struct rtgui_rect rect;

        dc = rtgui_dc_begin_drawing(widget);
        if (dc == RT_NULL) break;

        /* get rect information */
        rtgui_widget_get_rect(widget, &rect);

        /* clear whole window */
        RTGUI_WIDGET_BACKGROUND(widget) = white;
        rtgui_dc_fill_rect(dc, &rect);

        /* reset color */
        RTGUI_WIDGET_BACKGROUND(widget) = green;
        RTGUI_WIDGET_FOREGROUND(widget) = black;

        switch (calibration_ptr->step)
        {
        case CALIBRATION_STEP_LEFTTOP:
            rtgui_dc_draw_hline(dc,
                                0,
                                2 * CALIBRATION_WIDTH,
                                CALIBRATION_HEIGHT);
            rtgui_dc_draw_vline(dc,
                                CALIBRATION_WIDTH,
                                0,
                                2 * CALIBRATION_HEIGHT);
            rtgui_dc_draw_text(dc,
                               "Please touch the sight bead on the lift top to finish Calibration!",
                               &label_rect);
            RTGUI_WIDGET_FOREGROUND(widget) = red;
            rtgui_dc_fill_circle(dc,
                                 CALIBRATION_WIDTH,
                                 CALIBRATION_HEIGHT,
                                 4);
            break;

        case CALIBRATION_STEP_RIGHTTOP:
            rtgui_dc_draw_hline(dc,
                                calibration_ptr->width - 2 * CALIBRATION_WIDTH,
                                calibration_ptr->width,
                                CALIBRATION_HEIGHT);
            rtgui_dc_draw_vline(dc,
                                calibration_ptr->width - CALIBRATION_WIDTH,
                                0,
                                2 * CALIBRATION_HEIGHT);
            rtgui_dc_draw_text(dc,
                               "Please touch the sight bead on the right top to finish Calibration!",
                               &label_rect);
            RTGUI_WIDGET_FOREGROUND(widget) = red;
            rtgui_dc_fill_circle(dc,
                                 calibration_ptr->width - CALIBRATION_WIDTH,
                                 CALIBRATION_HEIGHT,
                                 4);
            break;

        case CALIBRATION_STEP_LEFTBOTTOM:
            rtgui_dc_draw_hline(dc,
                                0,
                                2 * CALIBRATION_WIDTH,
                                calibration_ptr->height - CALIBRATION_HEIGHT);
            rtgui_dc_draw_vline(dc,
                                CALIBRATION_WIDTH,
                                calibration_ptr->height - 2 * CALIBRATION_HEIGHT,
                                calibration_ptr->height);
            rtgui_dc_draw_text(dc,
                               "Please touch the sight bead on the left bottom to finish Calibration!",
                               &label_rect);
            RTGUI_WIDGET_FOREGROUND(widget) = red;
            rtgui_dc_fill_circle(dc,
                                 CALIBRATION_WIDTH,
                                 calibration_ptr->height - CALIBRATION_HEIGHT,
                                 4);
            break;

        case CALIBRATION_STEP_RIGHTBOTTOM:
            rtgui_dc_draw_hline(dc,
                                calibration_ptr->width - 2 * CALIBRATION_WIDTH,
                                calibration_ptr->width,
                                calibration_ptr->height - CALIBRATION_HEIGHT);
            rtgui_dc_draw_vline(dc,
                                calibration_ptr->width - CALIBRATION_WIDTH,
                                calibration_ptr->height - 2 * CALIBRATION_HEIGHT,
                                calibration_ptr->height);
            rtgui_dc_draw_text(dc,
                               "Please touch the sight bead on the right bottom to finish Calibration!",
                               &label_rect);
            RTGUI_WIDGET_FOREGROUND(widget) = red;
            rtgui_dc_fill_circle(dc,
                                 calibration_ptr->width - CALIBRATION_WIDTH,
                                 calibration_ptr->height - CALIBRATION_HEIGHT,
                                 4);
            break;

        case CALIBRATION_STEP_CENTER:
            rtgui_dc_draw_hline(dc,
                                calibration_ptr->width / 2 - CALIBRATION_WIDTH,
                                calibration_ptr->width / 2 + CALIBRATION_WIDTH,
                                calibration_ptr->height / 2);
            rtgui_dc_draw_vline(dc,
                                calibration_ptr->width / 2,
                                calibration_ptr->height / 2 - CALIBRATION_HEIGHT,
                                calibration_ptr->height / 2 + CALIBRATION_HEIGHT);
            rtgui_dc_draw_text(dc,
                               "Please touch the sight bead on the center to finish Calibration!",
                               &label_rect);
            RTGUI_WIDGET_FOREGROUND(widget) = red;
            rtgui_dc_fill_circle(dc,
                                 calibration_ptr->width / 2,
                                 calibration_ptr->height / 2,
                                 4);
            break;
        }
        rtgui_dc_end_drawing(dc);
    }
    break;

    case RTGUI_EVENT_COMMAND:
    {
        struct rtgui_event_command *ecmd = (struct rtgui_event_command *)event;

        switch (ecmd->command_id)
        {
        case TOUCH_WIN_UPDATE:
            rtgui_widget_update(widget);
            break;
        case TOUCH_WIN_CLOSE:
            rtgui_win_close(RTGUI_WIN(widget));
            break;
        }
    }
    return RT_TRUE;

    default:
        rtgui_win_event_handler(RTGUI_OBJECT(widget), event);
    }

    return RT_FALSE;
}

static void calibration_entry(void *parameter)
{
    rt_device_t device;
    struct rtgui_rect rect;

    device = rt_device_find("touch");
    if (device == RT_NULL)
    {
        rt_kprintf("RTGUI: no touch device to calibrate\n");
        return;
    }

    calibration_ptr = (struct calibration_session *)
                      rt_malloc(sizeof(*calibration_ptr));
    rt_memset(calibration_ptr, 0, sizeof(*calibration_ptr));
    calibration_ptr->device = device;

    rt_device_control(calibration_ptr->device, RT_TOUCH_CALIBRATION,
                      (void *)calibration_data_post);

    rtgui_graphic_driver_get_rect(rtgui_graphic_driver_get_default(), &rect);

    /* set screen rect */
    calibration_ptr->width = rect.x2;
    calibration_ptr->height = rect.y2;
    calibration_ptr->app = rtgui_app_create("calibration");
    if (calibration_ptr->app == RT_NULL)
    {
        rt_kprintf("RTGUI: no mem to create calibration app\n");
        goto __free_ptr;
    }
    /* create calibration window */
    calibration_ptr->win = rtgui_win_create(RT_NULL,
                                            "calibration", &rect,
                                            RTGUI_WIN_STYLE_NO_TITLE | RTGUI_WIN_STYLE_NO_BORDER |
                                            RTGUI_WIN_STYLE_ONTOP | RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);
    if (calibration_ptr->win != RT_NULL)
    {
        rtgui_object_set_event_handler(RTGUI_OBJECT(calibration_ptr->win),
                                       calibration_event_handler);
        rtgui_win_show(calibration_ptr->win, RT_TRUE);
    }

    rtgui_app_destroy(calibration_ptr->app);

    /* set calibration data */
    rt_device_control(calibration_ptr->device,
                      RT_TOUCH_CALIBRATION_DATA,
                      RT_NULL);

    if (_cali_after)
        _cali_after(cal_data);

    /* recover to normal */
    rt_device_control(calibration_ptr->device, RT_TOUCH_NORMAL, RT_NULL);

__free_ptr:
    /* release memory */
    rt_free(calibration_ptr);
    calibration_ptr = RT_NULL;
}

void calibration_init(void)
{
    rt_thread_t tid;

    rt_device_t device = rt_device_find("touch");

    if (device == RT_NULL)
    {
        rt_kprintf("RTGUI: no touch device to calibrate\n");
        return;
    }

    if (_cali_restore && (_cali_restore()==RT_TRUE))
    {
        return;
    }

    tid = rt_thread_create("cali", calibration_entry, RT_NULL, 1024, 20, 20);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void calibration(void)
{
    calibration_init();
}
FINSH_FUNCTION_EXPORT(calibration, perform touch calibration);
#endif
