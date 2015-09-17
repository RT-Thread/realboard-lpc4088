#include <rtgui/rtgui_object.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_theme.h>
#include <rtgui/rtgui_app.h>

#include <rtgui/list.h>
#include <rtgui/image.h>
#include <rtgui/widgets/container.h>
#include <rtgui/widgets/filelist_view.h>
#include <rtgui/widgets/listbox.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/box.h>
#include <rtgui/widgets/panel.h>
#include "filedialog.h"


#if defined(RTGUI_USING_DFS_FILERW)
#ifdef _WIN32_NATIVE
#include <io.h>
#include <dirent.h>
#include <sys/stat.h>
#define PATH_SEPARATOR      '\\'
#else
#include <dfs_posix.h>
#define PATH_SEPARATOR      '/'
#endif

#include <string.h>

#define RTGUI_FILELIST_MARGIN       5
#define PATH_LABEL_WIDTH            50
#define NEWDIR_BTN_WIDTH            50
#define NAME_LABEL_WIDTH            50
#define FILEDIALOG_HEAD_HEIGHT      40
#define FILEDIALOG_FOOT_HEIGHT      40
static rtgui_filedialog_t *filedialog = RT_NULL;

static void open_btn_onbutton(struct rtgui_object *object, struct rtgui_event *event)
{
    if (filedialog != RT_NULL)
    {

        if (filedialog->file_list->items[filedialog->file_list->current_item].type == RTGUI_FITEM_DIR)
        {
            char *dir_ptr;
            dir_ptr = (char *) rtgui_malloc(256);
            rtgui_filelist_view_get_fullpath(filedialog->file_list, dir_ptr, 256);
            rtgui_filelist_view_set_directory(filedialog->file_list, dir_ptr);
            rtgui_textbox_set_value(filedialog->path_textbox, dir_ptr);
            rtgui_widget_update(RTGUI_WIDGET(filedialog->path_textbox));
            rtgui_free(dir_ptr);
        }
        else
        {
            const char *filename = rtgui_textbox_get_value(filedialog->filename_textbox);
            if (filename != RT_NULL)
            {
                filedialog->full_file_name = (char *) rtgui_malloc(256);

                if (filedialog->file_list->current_directory[strlen(filedialog->file_list->current_directory) - 1] != PATH_SEPARATOR)
                    rt_snprintf(filedialog->full_file_name, 256, "%s%c%s", filedialog->file_list->current_directory, PATH_SEPARATOR,
                                filename);
                else
                    rt_snprintf(filedialog->full_file_name, 256, "%s%s", filedialog->file_list->current_directory,
                                filename);
                rtgui_win_close(filedialog->win);
                filedialog->status = RT_TRUE;
            }
        }
    }
}
static void cancel_btn_onbutton(struct rtgui_object *object, struct rtgui_event *event)
{
    rtgui_win_close(filedialog->win);
    filedialog->status = RT_FALSE;
}
static void ok_btn_onbutton(struct rtgui_object *object, struct rtgui_event *event)
{
    char *dir_ptr = (char *) rtgui_malloc(256);
    if (filedialog->file_list->current_directory[strlen(filedialog->file_list->current_directory) - 1] != PATH_SEPARATOR)
        rt_snprintf(dir_ptr, 256, "%s%c%s", filedialog->file_list->current_directory, PATH_SEPARATOR,
                    filedialog->dirname_textbox->text);
    else
        rt_snprintf(dir_ptr, 256, "%s%s", filedialog->file_list->current_directory,
                    filedialog->dirname_textbox->text);
    if (mkdir(dir_ptr, DFS_O_RDWR) != -1)
    {
        rtgui_win_close(filedialog->rename_win);
        rtgui_filelist_view_set_directory(filedialog->file_list, filedialog->file_list->current_directory);
    }
    rtgui_free(dir_ptr);
}
static void close_btn_onbutton(struct rtgui_object *object, struct rtgui_event *event)
{
    rtgui_win_close(filedialog->rename_win);
}
static void newdir_btn_onbutton(struct rtgui_object *object, struct rtgui_event *event)
{
    rtgui_rect_t parent_rect, rect = {0, 0, 200, 80};
    rtgui_button_t *ok_btn, *close_btn;

    rtgui_box_t *box, *head_box, *foot_box;
    rtgui_panel_t *head_panel, *foot_panel, *panel;
    rtgui_label_t *label;
    rtgui_widget_get_rect(RTGUI_WIDGET(filedialog->win), &parent_rect);
    rtgui_rect_moveto_align(&parent_rect, &rect,  RTGUI_ALIGN_CENTER_HORIZONTAL | RTGUI_ALIGN_CENTER_VERTICAL);
    rtgui_widget_rect_to_device(RTGUI_WIDGET(filedialog->win), &rect);
#ifdef RTGUI_USING_FONTHZ
    filedialog->rename_win = rtgui_win_create(filedialog->win,
                             "新建文件夹", &rect, RTGUI_WIN_STYLE_DEFAULT | RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);
#else
    filedialog->rename_win = rtgui_win_create(filedialog->win,
                             "New Directory", &rect, RTGUI_WIN_STYLE_DEFAULT | RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);
#endif
    if (filedialog->rename_win != RT_NULL)
    {
        box = rtgui_box_create(RTGUI_VERTICAL, 5);
        rtgui_container_set_box(RTGUI_CONTAINER(filedialog->rename_win), box);
#ifdef RTGUI_USING_FONTHZ
        filedialog->dirname_textbox = rtgui_textbox_create("新建文件夹", RTGUI_TEXTBOX_SINGLE);
        label = rtgui_label_create("名称:");
        ok_btn = rtgui_button_create("确定");
        close_btn = rtgui_button_create("取消");
#else
        filedialog->dirname_textbox = rtgui_textbox_create("NewDirectory", RTGUI_TEXTBOX_SINGLE);
        label = rtgui_label_create("Name:");
        ok_btn = rtgui_button_create("OK");
        close_btn = rtgui_button_create("Cancel");
#endif
        RTGUI_WIDGET_ALIGN(filedialog->dirname_textbox) = RTGUI_ALIGN_STRETCH;

        rtgui_button_set_onbutton(ok_btn, ok_btn_onbutton);
        RTGUI_WIDGET_ALIGN(ok_btn) = RTGUI_ALIGN_CENTER;
        rtgui_widget_set_minwidth(RTGUI_WIDGET(ok_btn), 40);
        rtgui_widget_set_minheight(RTGUI_WIDGET(ok_btn), 24);

        rtgui_button_set_onbutton(close_btn, close_btn_onbutton);
        RTGUI_WIDGET_ALIGN(close_btn) = RTGUI_ALIGN_CENTER;
        rtgui_widget_set_minwidth(RTGUI_WIDGET(close_btn), 40);
        rtgui_widget_set_minheight(RTGUI_WIDGET(close_btn), 24);


        rtgui_widget_set_minwidth(RTGUI_WIDGET(label), 40);

        head_panel = rtgui_panel_create(RTGUI_BORDER_NONE);
        RTGUI_WIDGET_ALIGN(head_panel) = RTGUI_ALIGN_EXPAND | RTGUI_ALIGN_STRETCH;
        head_box = rtgui_box_create(RTGUI_HORIZONTAL, 5);
        rtgui_container_set_box(RTGUI_CONTAINER(head_panel), head_box);

        foot_panel = rtgui_panel_create(RTGUI_BORDER_NONE);
        RTGUI_WIDGET_ALIGN(foot_panel) = RTGUI_ALIGN_EXPAND | RTGUI_ALIGN_STRETCH;
        foot_box = rtgui_box_create(RTGUI_HORIZONTAL, 20);
        rtgui_container_set_box(RTGUI_CONTAINER(foot_panel), foot_box);

        panel = rtgui_panel_create(RTGUI_BORDER_NONE);
        rtgui_widget_set_minwidth(RTGUI_WIDGET(panel), 20);

        rtgui_container_add_child(RTGUI_CONTAINER(head_panel), RTGUI_WIDGET(label));
        rtgui_container_add_child(RTGUI_CONTAINER(head_panel), RTGUI_WIDGET(filedialog->dirname_textbox));

        rtgui_container_add_child(RTGUI_CONTAINER(foot_panel), RTGUI_WIDGET(panel));
        rtgui_container_add_child(RTGUI_CONTAINER(foot_panel), RTGUI_WIDGET(ok_btn));
        rtgui_container_add_child(RTGUI_CONTAINER(foot_panel), RTGUI_WIDGET(close_btn));

        rtgui_container_add_child(RTGUI_CONTAINER(filedialog->rename_win), RTGUI_WIDGET(head_panel));
        rtgui_container_add_child(RTGUI_CONTAINER(filedialog->rename_win), RTGUI_WIDGET(foot_panel));
        rtgui_container_layout(RTGUI_CONTAINER(filedialog->rename_win));
        rtgui_win_show(filedialog->rename_win, RT_TRUE);
    }
}
static void updir_btn_onbutton(struct rtgui_object *object, struct rtgui_event *event)
{
    char *new_path = (char *) rtgui_malloc(256);

    char *ptr;
    ptr = strrchr(filedialog->file_list->current_directory, PATH_SEPARATOR);

    if (ptr == RT_NULL)
        return;
    if (ptr == &(filedialog->file_list->current_directory[0]))
    {
        /* it's root directory */
        new_path[0] = PATH_SEPARATOR;
        new_path[1] = '\0';
    }
    else
    {
        strncpy(new_path, filedialog->file_list->current_directory, ptr - filedialog->file_list->current_directory + 1);
        new_path[ptr - filedialog->file_list->current_directory] = '\0';
    }
    rtgui_filelist_view_set_directory(filedialog->file_list, new_path);
    rtgui_textbox_set_value(filedialog->path_textbox, new_path);
    rtgui_widget_update(RTGUI_WIDGET(filedialog->path_textbox));
    rtgui_free(new_path);
}

static rt_bool_t filelist_onchaned(struct rtgui_object *object, struct rtgui_event *event)
{
    rtgui_filelist_view_t *view;
    view = (rtgui_filelist_view_t *)object;
    if (view->items[view->current_item].type != RTGUI_FITEM_DIR)
    {
        char *filename;
        filename = view->items[view->current_item].name;
        rtgui_textbox_set_value(filedialog->filename_textbox, filename);
        rtgui_widget_update(RTGUI_WIDGET(filedialog->filename_textbox));
        if (filedialog->mode == RTGUI_FILEDIALOG_SAVE)
        {
#ifdef RTGUI_USING_FONTHZ
            rtgui_label_set_text(&filedialog->open_btn->parent, "保存");
#else
            rtgui_label_set_text(&filedialog->open_btn->parent, "Save");
#endif
            rtgui_widget_update(RTGUI_WIDGET(filedialog->open_btn));
        }
    }
    else
    {
        if (filedialog->mode == RTGUI_FILEDIALOG_SAVE)
        {
#ifdef RTGUI_USING_FONTHZ
            rtgui_label_set_text(&filedialog->open_btn->parent, "打开");
#else
            rtgui_label_set_text(&filedialog->open_btn->parent, "Open");
#endif
            rtgui_widget_update(RTGUI_WIDGET(filedialog->open_btn));
        }
    }

	return RT_TRUE;
}

rtgui_filedialog_t *rtgui_filedialog_create(rtgui_win_t *parent, const char *tile, rtgui_rect_t *rect)
{
    rtgui_label_t *path_label, *filename_label;
    rtgui_button_t *cancel_btn, *up_btn, *newdir_btn;
    rtgui_box_t *main_box, *head_box, *foot_box, *btn_box;
    rtgui_panel_t *head_panel, *foot_panel, *button_group;
    rtgui_rect_t widget_rect;
    filedialog = (rtgui_filedialog_t *)rtgui_malloc(sizeof(struct rtgui_filedialog));
    filedialog->win = rtgui_win_create(parent, tile, rect, RTGUI_WIN_STYLE_DEFAULT | RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);
    main_box = rtgui_box_create(RTGUI_VERTICAL, 4);
    rtgui_container_set_box(RTGUI_CONTAINER(filedialog->win), main_box);
    widget_rect.x1 = rect->x1 + RTGUI_FILELIST_MARGIN;
    widget_rect.x2 = rect->x2 - RTGUI_FILELIST_MARGIN;
    widget_rect.y1 = rect->y1 + RTGUI_FILELIST_MARGIN * 2 + FILEDIALOG_HEAD_HEIGHT;
    widget_rect.y2 = rect->y2 - FILEDIALOG_FOOT_HEIGHT - 2 * RTGUI_FILELIST_MARGIN;
    filedialog->file_list = rtgui_filelist_view_create("/", "", &widget_rect);
    RTGUI_WIDGET_ALIGN(filedialog->file_list) = RTGUI_ALIGN_EXPAND | RTGUI_ALIGN_STRETCH;
    rtgui_filelist_view_set_onchanged(filedialog->file_list, filelist_onchaned);
    filedialog->path_textbox = rtgui_textbox_create("/", RTGUI_TEXTBOX_SINGLE);
    RTGUI_WIDGET_ALIGN(filedialog->path_textbox) = RTGUI_ALIGN_STRETCH;
    filedialog->filename_textbox = rtgui_textbox_create("", RTGUI_TEXTBOX_SINGLE);
    RTGUI_WIDGET_ALIGN(filedialog->filename_textbox) = RTGUI_ALIGN_STRETCH;
#ifdef RTGUI_USING_FONTHZ
    newdir_btn = rtgui_button_create("新建");
    up_btn = rtgui_button_create("↑");
    path_label = rtgui_label_create("地址:");
    filename_label = rtgui_label_create("文件名:");
    if (filedialog->mode == RTGUI_FILEDIALOG_SAVE)
    {
        filedialog->open_btn = rtgui_button_create("保存");
    }
    else
    {
        filedialog->open_btn = rtgui_button_create("打开");
    }
    cancel_btn = rtgui_button_create("取消");
#else
    newdir_btn = rtgui_button_create("New");
    up_btn = rtgui_button_create("Up");
    path_label = rtgui_label_create("Url:");
    filename_label = rtgui_label_create("Name:");
    if (filedialog->mode == RTGUI_FILEDIALOG_SAVE)
    {
        open_btn = rtgui_button_create("Save");
    }
    else
    {
        open_btn = rtgui_button_create("Open");
    }
    cancel_btn = rtgui_button_create("Cancel");
#endif
    rtgui_button_set_onbutton(newdir_btn, newdir_btn_onbutton);
    rtgui_widget_set_minwidth(RTGUI_WIDGET(newdir_btn), 40);
    rtgui_widget_set_minheight(RTGUI_WIDGET(newdir_btn), 22);

    rtgui_button_set_onbutton(up_btn, updir_btn_onbutton);
    rtgui_widget_set_minwidth(RTGUI_WIDGET(up_btn), 40);
    rtgui_widget_set_minheight(RTGUI_WIDGET(up_btn), 22);

    rtgui_widget_set_minwidth(RTGUI_WIDGET(path_label), 40);

    rtgui_widget_set_minwidth(RTGUI_WIDGET(filename_label), 60);


    rtgui_button_set_onbutton(filedialog->open_btn, open_btn_onbutton);
    rtgui_widget_set_minwidth(RTGUI_WIDGET(filedialog->open_btn), 40);
    rtgui_widget_set_minheight(RTGUI_WIDGET(filedialog->open_btn), 30);

    rtgui_button_set_onbutton(cancel_btn, cancel_btn_onbutton);
    rtgui_widget_set_minwidth(RTGUI_WIDGET(cancel_btn), 40);
    rtgui_widget_set_minheight(RTGUI_WIDGET(cancel_btn), 30);

    head_panel = rtgui_panel_create(RTGUI_BORDER_NONE);
    rtgui_widget_set_minheight(RTGUI_WIDGET(head_panel), 30);
    RTGUI_WIDGET_ALIGN(head_panel) = RTGUI_ALIGN_EXPAND;
    head_box = rtgui_box_create(RTGUI_HORIZONTAL, 4);
    rtgui_container_set_box(RTGUI_CONTAINER(head_panel), head_box);

    foot_panel = rtgui_panel_create(RTGUI_BORDER_NONE);
    rtgui_widget_set_minheight(RTGUI_WIDGET(foot_panel), 40);
    RTGUI_WIDGET_ALIGN(foot_panel) = RTGUI_ALIGN_EXPAND;
    foot_box = rtgui_box_create(RTGUI_HORIZONTAL, 4);
    rtgui_container_set_box(RTGUI_CONTAINER(foot_panel), foot_box);

    button_group = rtgui_panel_create(RTGUI_BORDER_NONE);
    rtgui_widget_set_minheight(RTGUI_WIDGET(button_group), 38);
    RTGUI_WIDGET_ALIGN(button_group) = RTGUI_ALIGN_RIGHT;
    btn_box = rtgui_box_create(RTGUI_HORIZONTAL, 4);
    rtgui_container_set_box(RTGUI_CONTAINER(button_group), btn_box);

    rtgui_container_add_child(RTGUI_CONTAINER(foot_panel), RTGUI_WIDGET(filename_label));
    rtgui_container_add_child(RTGUI_CONTAINER(foot_panel), RTGUI_WIDGET(filedialog->filename_textbox));

    rtgui_container_add_child(RTGUI_CONTAINER(foot_panel), RTGUI_WIDGET(cancel_btn));
    rtgui_container_add_child(RTGUI_CONTAINER(foot_panel), RTGUI_WIDGET(filedialog->open_btn));

    rtgui_container_add_child(RTGUI_CONTAINER(head_panel), RTGUI_WIDGET(path_label));
    rtgui_container_add_child(RTGUI_CONTAINER(head_panel), RTGUI_WIDGET(filedialog->path_textbox));
    rtgui_container_add_child(RTGUI_CONTAINER(head_panel), RTGUI_WIDGET(up_btn));
    rtgui_container_add_child(RTGUI_CONTAINER(head_panel), RTGUI_WIDGET(newdir_btn));

    rtgui_container_add_child(RTGUI_CONTAINER(filedialog->win), RTGUI_WIDGET(head_panel));
    rtgui_container_add_child(RTGUI_CONTAINER(filedialog->win), RTGUI_WIDGET(filedialog->file_list));
    rtgui_container_add_child(RTGUI_CONTAINER(filedialog->win), RTGUI_WIDGET(foot_panel));

    return filedialog;
}
void rtgui_filedialog_set_directory(rtgui_filedialog_t *dialog, const char *dir)
{
    rtgui_textbox_set_value(dialog->path_textbox, dir);
    rtgui_filelist_view_set_directory(dialog->file_list, dir);
}

void rtgui_filedialog_set_mode(rtgui_filedialog_t *dialog, rtgui_filedialog_mode_t mode)
{
    dialog->mode = mode;
    if (filedialog->mode == RTGUI_FILEDIALOG_SAVE)
    {
#ifdef RTGUI_USING_FONTHZ
        rtgui_label_set_text(&filedialog->open_btn->parent, "保存");
#else
        rtgui_label_set_text(&filedialog->open_btn->parent, "Save");
#endif
        rtgui_widget_update(RTGUI_WIDGET(filedialog->open_btn));
    }
}

void rtgui_filedialog_set_filename(rtgui_filedialog_t *dialog, const char *filename)
{
    rtgui_textbox_set_value(dialog->filename_textbox, filename);
}

rt_bool_t rtgui_filedialog_show(rtgui_filedialog_t *dialog)
{

    /* re-layout */
    rtgui_container_layout(RTGUI_CONTAINER(dialog->win));
    /* show windows */
    rtgui_win_show(dialog->win, RT_TRUE);

    return filedialog->status;
}
char *rtgui_filedialog_get_full_filename(rtgui_filedialog_t *dialog)
{
    return filedialog->full_file_name;
}
void rtgui_filedialog_destroy(rtgui_filedialog_t *dialog)
{
    if (dialog->win != RT_NULL)
    {
        rtgui_win_destroy(dialog->win);
    }
    rtgui_free(dialog);
}
char *rtgui_filedialog_save_file(rtgui_win_t *parent, const char *tile, char *current_directory, char *filename, rtgui_rect_t *rect)
{
    char *ret = RT_NULL;
    rtgui_filedialog_t *dialog;
    dialog = rtgui_filedialog_create(parent, tile, rect);
    rtgui_filedialog_set_filename(dialog, filename);
    rtgui_filedialog_set_directory(dialog, current_directory);
    rtgui_filedialog_set_mode(dialog, RTGUI_FILEDIALOG_SAVE);
    if (rtgui_filedialog_show(dialog) == RT_TRUE)
    {
        ret = dialog->full_file_name;
        rtgui_filedialog_destroy(dialog);
    }
    return ret;
}
char *rtgui_filedialog_open_file(rtgui_win_t *parent, const char *tile, char *current_directory, char *filename, rtgui_rect_t *rect)
{
    char *ret = RT_NULL;
    rtgui_filedialog_t *dialog;
    dialog = rtgui_filedialog_create(parent, tile, rect);
    rtgui_filedialog_set_filename(dialog, filename);
    rtgui_filedialog_set_directory(dialog, current_directory);
    if (rtgui_filedialog_show(dialog) == RT_TRUE)
    {
        ret = dialog->full_file_name;
        rtgui_filedialog_destroy(dialog);
    }
    return ret;
}

#endif
