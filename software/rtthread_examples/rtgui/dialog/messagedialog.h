#ifndef __RTGUI_messagedialog_H__
#define __RTGUI_messagedialog_H__

#include <rtgui/widgets/container.h>

typedef enum
{
    RTGUI_MB_FLAG_OK       = 0x01,
    RTGUI_MB_FLAG_CANCEL   = 0x02,
    RTGUI_MB_FLAG_OKCANCEL = RTGUI_MB_FLAG_OK | RTGUI_MB_FLAG_CANCEL,
    RTGUI_MB_FLAG_YES      = 0x04,
    RTGUI_MB_FLAG_NO       = 0x08,
    RTGUI_MB_FLAG_YESNO    = RTGUI_MB_FLAG_YES | RTGUI_MB_FLAG_NO,
    RTGUI_MB_FLAG_RETRY    = 0x10,
    RTGUI_MB_FLAG_DEFAULT  = RTGUI_MB_FLAG_OK,
} rtgui_dialog_flag_t;

typedef enum
{
    RTGUI_DR_OK,
    RTGUI_DR_CANCEL,
    RTGUI_DR_YES,
    RTGUI_DR_NO,
    RTGUI_DR_RETRY,
} rtgui_dialog_result_t;

struct rtgui_messagedialog
{
    /* messagedialog window */
    struct rtgui_win *win;
    /* messagedialog iconbox */
    struct rtgui_iconbox *iconbox;
    /* messagedialog textview */
    struct rtgui_textview *text_view;
    /* messagedialog flag */
    rtgui_dialog_flag_t flag ;
};
typedef struct rtgui_messagedialog rtgui_messagedialog_t;

rtgui_messagedialog_t *rtgui_messagedialog_create(rtgui_win_t *parent, const char *text, const char *tile, rtgui_rect_t *rect, rtgui_dialog_flag_t flag, rtgui_image_t *icon);
rtgui_dialog_result_t rtgui_messagedialog_show(rtgui_messagedialog_t *msgbox);

void rtgui_messagedialog_destroy(rtgui_messagedialog_t *msgbox);

#endif /*__RTGUI_messagedialog_H__*/