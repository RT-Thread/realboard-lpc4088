#ifndef __RTGUI_FILEDIALOG_H__
#define __RTGUI_FILEDIALOG_H__

#include <rtgui/widgets/container.h>

typedef enum
{
    RTGUI_FILEDIALOG_OPEN,
    RTGUI_FILEDIALOG_SAVE,
} rtgui_filedialog_mode_t;

struct rtgui_filedialog
{
    struct rtgui_win *win;
    /* file list */
    struct rtgui_filelist_view *file_list;
    /* rename win dialog*/
    struct rtgui_win *rename_win;
    /* dir name textbox */
    rtgui_textbox_t *dirname_textbox;
    /* path name textbox */
    rtgui_textbox_t *path_textbox;
    /* file name textbox */
    rtgui_textbox_t *filename_textbox;
    /* open button */
    rtgui_button_t *open_btn;
    /* mode */
    rtgui_filedialog_mode_t mode;
    /* full file name */
    char *full_file_name;
    /* status */
    rt_bool_t status;
};
typedef struct rtgui_filedialog rtgui_filedialog_t;

rtgui_filedialog_t *rtgui_filedialog_create(rtgui_win_t *parent, const char *tile, rtgui_rect_t *rect);
void rtgui_filedialog_set_mode(rtgui_filedialog_t *dialog, rtgui_filedialog_mode_t mode);
void rtgui_filedialog_set_directory(rtgui_filedialog_t *dialog, const char *current_directory);
void rtgui_filedialog_set_filename(rtgui_filedialog_t *dialog, const char *filename);
rt_bool_t rtgui_filedialog_show(rtgui_filedialog_t *dialog);
char *rtgui_filedialog_get_full_filename(rtgui_filedialog_t *dialog);

char *rtgui_filedialog_save_file(rtgui_win_t *parent, const char *tile, char *current_directory, char *filename, rtgui_rect_t *rect);
char *rtgui_filedialog_open_file(rtgui_win_t *parent, const char *tile, char *current_directory, char *filename, rtgui_rect_t *rect);

void rtgui_filedialog_destroy(rtgui_filedialog_t *dialog);

#endif /*__RTGUI_FILEDIALOG_H__*/

