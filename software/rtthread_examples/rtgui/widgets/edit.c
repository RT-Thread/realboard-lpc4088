/*
 * File      : edit.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-06-04     amsl         firist version.
 * 2012-08-09     amsl         beta 0.1
 */
#include <rtgui/dc.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/edit.h>
#include <rtgui/widgets/scrollbar.h>
#include <rtgui/filerw.h>

#include <ctype.h>

#include "text_encoding.h"

#define RTGUI_EDIT_CARET_TIMEOUT (RT_TICK_PER_SECOND/2)

static void rtgui_edit_init_caret(struct rtgui_edit *edit, rtgui_point_t visual);
static void rtgui_edit_update(struct rtgui_edit *edit);
static rt_bool_t rtgui_edit_onfocus(struct rtgui_object *object, rtgui_event_t *event);
static rt_bool_t rtgui_edit_onunfocus(struct rtgui_object *object, rtgui_event_t *event);
#ifdef RTGUI_EDIT_USING_SCROLL
static rt_bool_t rtgui_edit_hscroll_handle(struct rtgui_widget *widget, rtgui_event_t *event);
static rt_bool_t rtgui_edit_vscroll_handle(struct rtgui_widget *widget, rtgui_event_t *event);
#endif

void _rtgui_edit_constructor(struct rtgui_edit *edit)
{
    rtgui_rect_t font_rect;
    RTGUI_WIDGET_FLAG(edit) |= RTGUI_WIDGET_FLAG_FOCUSABLE;

    rtgui_object_set_event_handler(RTGUI_OBJECT(edit), rtgui_edit_event_handler);
    rtgui_widget_set_onfocus(RTGUI_WIDGET(edit), rtgui_edit_onfocus);
    rtgui_widget_set_onunfocus(RTGUI_WIDGET(edit), rtgui_edit_onunfocus);

    RTGUI_WIDGET_FOREGROUND(edit) = black;
    RTGUI_WIDGET_BACKGROUND(edit) = white;
    /* set default text align */
    RTGUI_WIDGET_TEXTALIGN(edit) = RTGUI_ALIGN_CENTER_VERTICAL;
    rtgui_widget_set_border(RTGUI_WIDGET(edit), RTGUI_BORDER_SUNKEN);
    rtgui_caret_init(&edit->caret, RTGUI_WIDGET(edit));

    edit->tabsize = 4;
    edit->margin  = 1;
    edit->max_rows = edit->max_cols = 0;
    edit->visual.x = edit->visual.y = 0;
    edit->upleft.x = edit->upleft.y = 0;
    edit->row_per_page = edit->col_per_page = 0;

    edit->flag = RTGUI_EDIT_NONE;
#ifdef RTGUI_EDIT_USING_SCROLL
    edit->flag |= RTGUI_EDIT_VSCROLL;
    edit->flag |= RTGUI_EDIT_HSCROLL;
#endif
    /* allocate default line buffer */
    edit->bzsize = 16;

    rtgui_font_get_metrics(RTGUI_WIDGET_FONT(edit), "H", &font_rect);
    edit->font_width = rtgui_rect_width(font_rect);
    edit->font_height = rtgui_rect_height(font_rect);

    edit->head = RT_NULL;
    edit->on_delete_line = RT_NULL;
#ifdef RTGUI_EDIT_USING_SCROLL
    edit->hscroll = RT_NULL;
    edit->vscroll = RT_NULL;
#endif
}

void _rtgui_edit_deconstructor(struct rtgui_edit *edit)
{
    if (edit->max_rows > 0)
    {
        while (edit->max_rows > 0)
            rtgui_edit_delete_line(edit, edit->head);
        edit->max_rows = 0;
    }
    rtgui_caret_cleanup(&edit->caret);
}

DEFINE_CLASS_TYPE(edit, "edit",
                  RTGUI_PARENT_TYPE(widget),
                  _rtgui_edit_constructor,
                  _rtgui_edit_deconstructor,
                  sizeof(struct rtgui_edit));

#ifdef RTGUI_EDIT_USING_SCROLL
void rtgui_edit_adjust_scroll(rtgui_scrollbar_t *bar)
{
    struct rtgui_edit *edit;

    RT_ASSERT(bar != RT_NULL);

    if (bar->widget_link != RT_NULL)
    {
        rtgui_rect_t rect;
        rt_uint32_t _left = 0, _top = 0, _width = RTGUI_DEFAULT_SB_WIDTH, _len = 0;

        edit = bar->widget_link;
        rtgui_widget_get_rect(edit, &rect);
        rtgui_widget_rect_to_device(edit, &rect);
        if (bar->orient == RTGUI_HORIZONTAL)
        {
            if (RTGUI_WIDGET_IS_HIDE(edit->hscroll))
            {
                if (edit->max_rows > edit->row_per_page)
                {
                    RTGUI_WIDGET_SHOW(edit->hscroll);
                    rtgui_scrollbar_set_line_step(edit->hscroll, 1);
                    rtgui_scrollbar_set_page_step(edit->hscroll, edit->row_per_page);
                    rtgui_scrollbar_set_range(edit->hscroll, edit->max_rows);
                }
                else
                    RTGUI_WIDGET_HIDE(edit->vscroll);
                rtgui_widget_update_clip(RTGUI_WIDGET(edit));
            }
            else
            {
                _left = RTGUI_WIDGET_BORDER(edit);
                _top = rtgui_rect_height(rect) - RTGUI_WIDGET_BORDER(edit) - _width;
                _len = rtgui_rect_width(rect) - RTGUI_WIDGET_BORDER(edit) * 2;

                if (!RTGUI_WIDGET_IS_HIDE(edit->vscroll))
                    _len -= _width;
                rect.x1 += _left;
                rect.y1 += _top;
                rect.x2 = rect.x1 + _len;
                rect.y2 = rect.y1 + _width;
            }
        }
        else if (bar->orient == RTGUI_VERTICAL)
        {
            _left = rtgui_rect_width(rect) - RTGUI_WIDGET_BORDER(edit) - _width;
            _top = RTGUI_WIDGET_BORDER(edit);
            _len = rtgui_rect_height(rect) - RTGUI_WIDGET_BORDER(edit) * 2;

            if (!RTGUI_WIDGET_IS_HIDE(edit->hscroll))
                _len -= _width;
            rect.x1 += _left;
            rect.y1 += _top;
            rect.x2 = rect.x1 + _width;
            rect.y2 = rect.y1 + _len;
        }
        rtgui_widget_set_rect(bar, &rect);
    }
}
RTM_EXPORT(rtgui_edit_adjust_scroll);
#endif

struct rtgui_edit *rtgui_edit_create(struct rtgui_container *container, int left, int top, int w, int h)
{
    struct rtgui_edit *edit;

    RT_ASSERT(container != RT_NULL);

    edit = (struct rtgui_edit *)rtgui_widget_create(RTGUI_EDIT_TYPE);
    if (edit != RT_NULL)
    {
        rtgui_rect_t rect;
        int effe;
        rtgui_widget_get_rect(RTGUI_WIDGET(container), &rect);
        rtgui_widget_rect_to_device(RTGUI_WIDGET(container), &rect);
        rect.x1 += left;
        rect.y1 += top;
        rect.x2 = rect.x1 + w;
        rect.y2 = rect.y1 + h;
        rtgui_widget_set_rect(RTGUI_WIDGET(edit), &rect);
        rtgui_container_add_child(container, RTGUI_WIDGET(edit));
        /* set edit update region*/
        edit->update.start.x = 0;
        edit->update.start.y = 0;
        edit->update.end.x = w;
        edit->update.end.y = h;
        /* set character number */
        edit->item_height = edit->font_height; /* the same height */
        effe = h - (edit->margin + RTGUI_WIDGET_BORDER(edit)) * 2;
        edit->row_per_page = effe / edit->item_height;
        if (effe % edit->item_height)
            edit->row_per_page += 1;

        effe = w - (edit->margin + RTGUI_WIDGET_BORDER(edit)) * 2;
        edit->col_per_page = effe / edit->font_width;
        if (effe % edit->font_width)
            edit->col_per_page += 1;
#ifdef RTGUI_EDIT_USING_SCROLL
        if (edit->hscroll == RT_NULL && edit->flag & RTGUI_EDIT_HSCROLL)
        {
            /* create horizontal scrollbar */
            rt_uint32_t _left, _top, _width = RTGUI_DEFAULT_SB_WIDTH, _len;
            _left = RTGUI_WIDGET_BORDER(edit);
            _top = rtgui_rect_height(rect) - RTGUI_WIDGET_BORDER(edit) - _width;
            _len = rtgui_rect_width(rect) - RTGUI_WIDGET_BORDER(edit) * 2;
            if (edit->max_rows > edit->row_per_page) _len -= _width;

            edit->hscroll = rtgui_scrollbar_create(edit, _left, _top, _width, _len, RTGUI_HORIZONTAL);

            if (edit->hscroll != RT_NULL)
            {
                edit->hscroll->widget_link = (pvoid)edit;
                edit->hscroll->on_scroll = rtgui_edit_hscroll_handle;
                RTGUI_WIDGET_HIDE(edit->hscroll);
            }
        }
        if (edit->vscroll == RT_NULL && edit->flag & RTGUI_EDIT_VSCROLL)
        {
            /* create vertical scrollbar */
            rt_uint32_t _left, _top, _width = RTGUI_DEFAULT_SB_WIDTH, _len;
            _left = rtgui_rect_width(rect) - RTGUI_WIDGET_BORDER(edit) - _width;
            _top = RTGUI_WIDGET_BORDER(edit);
            _len = rtgui_rect_height(rect) - RTGUI_WIDGET_BORDER(edit) * 2;
            if (edit->max_cols > edit->col_per_page) _len -= _width;

            edit->vscroll = rtgui_scrollbar_create(edit, _left, _top, _width, _len, RTGUI_VERTICAL);

            if (edit->vscroll != RT_NULL)
            {
                edit->vscroll->widget_link = (pvoid)edit;
                edit->vscroll->on_scroll = rtgui_edit_vscroll_handle;
                RTGUI_WIDGET_HIDE(edit->vscroll);
            }
        }
#endif
    }

    return edit;
}
RTM_EXPORT(rtgui_edit_create);

void rtgui_edit_destroy(struct rtgui_edit *edit)
{
    rtgui_widget_destroy(RTGUI_WIDGET(edit));
}
RTM_EXPORT(rtgui_edit_destroy);

void rtgui_edit_set_ondelete_line(struct rtgui_edit *edit,
                                  void (*ondl)(struct rtgui_edit*, struct edit_line*))
{
    edit->on_delete_line = ondl;
}

/**
 * calc line buffer alloc length
 *
 * @param n a standard buffer value, please use edit->bzsize
 * @param m given a reference value
 *
 * @return get a proper standard values
 */
rt_inline rt_int16_t rtgui_edit_alloc_len(rt_int16_t n, rt_int16_t m)
{
    if (n > m) return n;
#ifndef RTGUI_USING_SMALL_SIZE
    return rtgui_edit_alloc_len(n * 2, m);
#else
    return rtgui_edit_alloc_len(n + 16, m);
#endif
}

/**
 * please use it to replace rt_strlen
 * especially in reading the source file.
 */
rt_inline rt_int16_t rtgui_edit_line_strlen(const char *s)
{
    const char *sc;
    /* ascii text end of 0x0A or 0x0D-0x0A*/
    for (sc = s; *sc != 0x0D && *sc != 0x0A && *sc != 0x00; ++sc);
    return sc - s;
}

static void _edit_dump_status(struct rtgui_edit *edit)
{
    rt_kprintf("edit(%p) status:\n", edit);
    rt_kprintf("flag: %08x, max_rows/col: %d, %d\n",
               edit->flag, edit->max_rows, edit->max_cols);
    rt_kprintf("rows/col_per_page: %d, %d\n",
               edit->row_per_page, edit->col_per_page);
    rt_kprintf("upleft: (%d, %d), visual: (%d, %d)\n",
               edit->upleft.x, edit->upleft.y,
               edit->visual.x, edit->visual.y);
}

static void _edit_dump_lines(struct rtgui_edit *edit)
{
    struct edit_line *line;

    rt_kprintf("edit(%p) lines:\n", edit);
    for (line = edit->head; line; line = line->next)
    {
        rt_kprintf("%2d line(%p): |%s|\n",
                   line->line_number, line, line->text);
    }
}

void rtgui_edit_dump(struct rtgui_edit *edit)
{
    _edit_dump_status(edit);
    _edit_dump_lines(edit);
}

static int _line_cursor_pos_at(struct edit_line *line, int offset)
{
    struct rtgui_char_position pos;

    /* If we are in the middle of a wide char, move the cursor to the end of
     * current wide char. */
    if (offset >= line->len)
        return line->len;

    pos = _string_char_width(line->text, line->len + 1, offset);
    if (pos.char_width == pos.remain)
        /* We are at the beginning of the char. No need to adjust. */
        return offset;

    /* Move to the boundary. */
    return offset + pos.remain;
}

static int _edit_char_width(struct rtgui_edit *edit,
                            struct edit_line *line,
                            int offset)
{
    int abs_pos;

    abs_pos = edit->upleft.x + edit->visual.x + offset;

    RT_ASSERT(abs_pos < line->len);

    return _string_char_width(line->text, line->len + 1, abs_pos).char_width;
}

/* Should be called before update/ondraw. See the comment on _edit_show_caret.
 */
static void _edit_hide_caret(struct rtgui_edit *edit)
{
    rtgui_caret_clear(&edit->caret);
}

/* _edit_show_caret should be called after _edit_hide_caret.
 *
 * Common procedure should be:
 *     _edit_hide_caret(edit)
 *     if (...)
 *     {
 *         ...
 *         rtgui_edit_ondraw(edit);
 *     }
 *     else
 *     {
 *         ...
 *         rtgui_edit_update(edit);
 *     }
 *     _edit_show_caret(edit);
 */
static void _edit_show_caret(struct rtgui_edit *edit)
{
    RT_ASSERT(!(edit->flag & RTGUI_EDIT_CARET));

    rtgui_edit_init_caret(edit, edit->visual);
    rtgui_caret_show(&edit->caret);
    rtgui_caret_start_timer(&edit->caret, RTGUI_EDIT_CARET_TIMEOUT);
}

/* Increase the line number from specific line. */
static void _line_add_ln_from(struct edit_line *line, int inc)
{
    for (; line; line = line->next)
        line->line_number += inc;
}

rt_bool_t rtgui_edit_append_line(struct rtgui_edit *edit, const char *text)
{
    rt_int16_t len;
    struct edit_line *line;

    RT_ASSERT(edit != RT_NULL);

    line = (struct edit_line *)rtgui_malloc(sizeof(struct edit_line));
    if (line == RT_NULL)
        return RT_FALSE;

    len = rtgui_edit_line_strlen(text);
    line->zsize = rtgui_edit_alloc_len(edit->bzsize, len + 1);
    line->text = (char *)rtgui_malloc(line->zsize);
    rt_memcpy(line->text, text, len);
    *(line->text + len) = '\0';
    line->len = len;

    line->next = RT_NULL;
    edit->max_rows++;
    if (edit->max_cols < len)
        edit->max_cols = len;

    if (edit->head == RT_NULL)
    {
        line->prev = RT_NULL;
        line->line_number = 0;
        edit->head = line;
    }
    else
    {
        struct edit_line *node = edit->head;
        while (node->next != RT_NULL)
            node = node->next;
        /* to tail item on to queue */
        node->next = line;
        line->prev = node;
        line->line_number = line->prev->line_number + 1;
        RT_ASSERT(line->line_number == edit->max_rows - 1);
    }
    edit->update.start.x = 0;
    edit->update.start.y = edit->max_rows - 1;
    edit->update.end.x = line->len;
    edit->update.end.y = edit->update.start.y;
    rtgui_edit_update(edit);

    return RT_TRUE;
}
RTM_EXPORT(rtgui_edit_append_line);

rt_bool_t rtgui_edit_insert_line(struct rtgui_edit *edit, struct edit_line *p, char *text)
{
    rt_int16_t len;
    struct edit_line *line, *vline;

    RT_ASSERT(edit != RT_NULL);

    if (p == RT_NULL || p->next == RT_NULL)
    {
        rtgui_edit_append_line(edit, text);
        return RT_TRUE;
    }

    /* If @p is not belong to @edit, skip this insert. */
    if (rtgui_edit_get_index_by_line(edit, p) < 0)
        return RT_FALSE;

    line = (struct edit_line *)rtgui_malloc(sizeof(struct edit_line));
    if (line == RT_NULL)
        return RT_FALSE;

    edit->max_rows++;

    line->prev = p;
    line->next = p->next;
    p->next->prev = line;
    p->next = line;
    len = rtgui_edit_line_strlen(text);
    line->zsize = rtgui_edit_alloc_len(edit->bzsize, len + 1);
    line->line_number = p->line_number + 1;
    _line_add_ln_from(line->next, 1);

    line->text = (char *)rtgui_malloc(line->zsize);
    rt_memset(line->text, 0, line->zsize);
    rt_memcpy(line->text, text, len);
    *(line->text + len) = '\0';
    line->len = len;

    _edit_hide_caret(edit);

    vline = rtgui_edit_get_line_by_index(edit, edit->upleft.y + edit->visual.y);
    RT_ASSERT(vline);
    edit->visual.x = _line_cursor_pos_at(vline,
                                         edit->upleft.x + edit->visual.x) - edit->upleft.x;
    if (edit->visual.x < edit->upleft.x)
    {
        edit->upleft.x = edit->visual.x;
        edit->visual.x = 0;
        rtgui_edit_ondraw(edit);
    }
    else
    {
        edit->update.start.x = 0;
        edit->update.end.x = edit->col_per_page;
        edit->update.start.y = rtgui_edit_get_index_by_line(edit, line);
        edit->update.end.y = edit->max_rows;
        rtgui_edit_update(edit);
    }

    _edit_show_caret(edit);

    return RT_TRUE;
}
RTM_EXPORT(rtgui_edit_insert_line);

static void _edit_del_line(struct rtgui_edit *edit, struct edit_line *line)
{
    if (edit->head == line)
        edit->head = line->next;
    if (line->prev)
        line->prev->next = line->next;
    if (line->next)
        line->next->prev = line->prev;

    if (edit->max_rows > 0)
        edit->max_rows--;
    _line_add_ln_from(line->next, -1);
    if (edit->on_delete_line)
        edit->on_delete_line(edit, line);
    if (line->text)
        rtgui_free(line->text);
    rtgui_free(line);
}

rt_bool_t rtgui_edit_delete_line(struct rtgui_edit *edit, struct edit_line *line)
{
    int redraw = 0;
    struct edit_line *comming_line;

    RT_ASSERT(edit != RT_NULL);
    if (line == RT_NULL)
        return RT_FALSE;

    if (edit->max_rows == 0)
        return RT_FALSE;

    /* Set the update start index before we free it. */
    edit->update.start.x = 0;
    edit->update.start.y = rtgui_edit_get_index_by_line(edit, line) - edit->upleft.y;
    if (edit->update.start.y < 0)
        edit->update.start.y = 0;

    _edit_del_line(edit, line);

    _edit_hide_caret(edit);
    comming_line = rtgui_edit_get_line_by_index(edit,
                                                edit->upleft.y + edit->visual.y);
    /* Loop until we found a line that fit the carect. */
    while (!comming_line && (edit->upleft.y + edit->visual.y))
    {
        if (edit->visual.y == 0)
        {
            redraw = 1;
            edit->upleft.y--;
        }
        else
            edit->visual.y--;
        comming_line = rtgui_edit_get_line_by_index(edit,
                                                    edit->upleft.y + edit->visual.y);
    }
    /* No line left. Cleanup the edit. */
    if (!comming_line)
    {
        edit->upleft.x = 0;
        edit->upleft.y = 0;
        edit->visual.x = 0;
        edit->visual.y = 0;
        rtgui_edit_ondraw(edit);
        _edit_show_caret(edit);
        return RT_TRUE;
    }

    if (edit->upleft.x > comming_line->len)
    {
        edit->upleft.x = comming_line->len;
        edit->visual.x = 0;
    }
    else if (edit->upleft.x + edit->visual.x > comming_line->len)
        edit->visual.x = comming_line->len - edit->upleft.x;
    else /* Caret is within the comming line. */
        edit->visual.x = _line_cursor_pos_at(comming_line,
                                             edit->upleft.x + edit->visual.x) - edit->upleft.x;

    if (redraw)
    {
        rtgui_edit_ondraw(edit);
    }
    else
    {
        edit->update.end.x = edit->col_per_page;
        edit->update.end.y = edit->row_per_page;
        rtgui_edit_update(edit);
    }
    _edit_show_caret(edit);

    return RT_TRUE;
}
RTM_EXPORT(rtgui_edit_delete_line);

void rtgui_edit_clear_text(struct rtgui_edit *edit)
{
    RT_ASSERT(edit != RT_NULL);

    /* Call the low level API so we don't update in the middle, thus avoiding
     * flickering. */
    _edit_hide_caret(edit);
    while (edit->head)
        _edit_del_line(edit, edit->head);
    edit->upleft.x = 0;
    edit->upleft.y = 0;
    edit->visual.x = 0;
    edit->visual.y = 0;
    rtgui_widget_update(RTGUI_WIDGET(edit));
    rtgui_edit_init_caret(edit, edit->visual);
    _edit_show_caret(edit);
}
RTM_EXPORT(rtgui_edit_clear_text);

/* set edit text */
void rtgui_edit_set_text(struct rtgui_edit *edit, const char *text)
{
    const char *begin, *ptr;
#ifdef RTGUI_EDIT_USING_SCROLL
    int hscroll_flag = 0;
    int vscroll_flag = 0;
#endif

    RT_ASSERT(edit != RT_NULL);

    rtgui_edit_clear_text(edit);

    begin = text;
    for (ptr = begin; *ptr != '\0'; ptr++)
    {
        if (*ptr == '\n')
        {
            /* rtgui_edit_append_line will deal with the \r if there is any. */
            rtgui_edit_append_line(edit, begin);
            begin = ptr + 1;
        }
    }
    /* Append the last line. */
    if (begin < ptr)
    {
        rtgui_edit_append_line(edit, begin);
    }

#ifdef RTGUI_EDIT_USING_SCROLL
    if (edit->hscroll != RT_NULL)
    {
        if (edit->max_cols > edit->col_per_page)
        {
            RTGUI_WIDGET_SHOW(edit->hscroll);
            rtgui_scrollbar_set_line_step(edit->hscroll, 1);
            rtgui_scrollbar_set_page_step(edit->hscroll, edit->col_per_page);
            rtgui_scrollbar_set_range(edit->hscroll, edit->max_cols);
            hscroll_flag = 1;
        }
        else
        {
            RTGUI_WIDGET_HIDE(edit->hscroll);
        }
    }
    if (edit->vscroll != RT_NULL)
    {
        if (edit->max_rows > edit->row_per_page)
        {
            RTGUI_WIDGET_SHOW(edit->vscroll);
            rtgui_scrollbar_set_line_step(edit->vscroll, 1);
            rtgui_scrollbar_set_page_step(edit->vscroll, edit->row_per_page);
            rtgui_scrollbar_set_range(edit->vscroll, edit->max_rows);
            vscroll_flag = 1;
        }
        else
        {
            RTGUI_WIDGET_HIDE(edit->vscroll);
        }
    }

    if (edit->hscroll != RT_NULL && !RTGUI_WIDGET_IS_HIDE(edit->hscroll))
    {
        rtgui_edit_adjust_scroll(edit->hscroll);
    }
    if (edit->vscroll != RT_NULL && !RTGUI_WIDGET_IS_HIDE(edit->vscroll))
    {
        rtgui_edit_adjust_scroll(edit->vscroll);
    }

    if (hscroll_flag || vscroll_flag)
    {
        rtgui_widget_update_clip(RTGUI_WIDGET(edit));
    }
#endif
}
RTM_EXPORT(rtgui_edit_set_text);

rt_bool_t rtgui_edit_connect_line(struct rtgui_edit *edit, struct edit_line *line, struct edit_line *connect)
{
    rt_int16_t len1, len2;

    RT_ASSERT(edit != RT_NULL);
    if (line == RT_NULL || connect == RT_NULL)
    {
        return RT_FALSE;
    }

    len1 = rtgui_edit_line_strlen(line->text);
    len2 = rtgui_edit_line_strlen(connect->text);

    line->zsize = rtgui_edit_alloc_len(edit->bzsize, len1 + len2 + 1);
    line->text = (char *)rt_realloc(line->text, line->zsize);
    rt_memcpy(line->text + len1, connect->text, len2);
    *(line->text + len1 + len2) = '\0';

    line->len = rtgui_edit_line_strlen(line->text);
    return RT_TRUE;
}
RTM_EXPORT(rtgui_edit_connect_line);

static void rtgui_edit_get_caret_rect(struct rtgui_edit *edit, rtgui_rect_t *rect, rtgui_point_t visual)
{
    RT_ASSERT(edit != RT_NULL);

    rtgui_widget_get_rect(RTGUI_WIDGET(edit), rect);

    rect->x1 += visual.x * edit->font_width + RTGUI_WIDGET_BORDER(edit) + edit->margin;
    rect->x2 = rect->x1 + 1; /* caret width: 1 */
    rect->y1 += visual.y * edit->item_height + RTGUI_WIDGET_BORDER(edit) + edit->margin;
    if ((rect->y1 + edit->font_height) < (rect->y2 - RTGUI_WIDGET_BORDER(edit) - edit->margin))
        rect->y2 = rect->y1 + edit->font_height;
    else
        rect->y2 = rect->y2 - RTGUI_WIDGET_BORDER(edit) - edit->margin;
}

static void rtgui_edit_init_caret(struct rtgui_edit *edit, rtgui_point_t visual)
{
    rtgui_rect_t rect;
    struct edit_line *line;

    RT_ASSERT(edit != RT_NULL);
    if (!RTGUI_WIDGET_IS_FOCUSED(edit))
        return;

    rtgui_edit_get_caret_rect(edit, &rect, visual);
    line = rtgui_edit_get_line_by_index(edit, edit->upleft.y + visual.y);
    if (line)
        rtgui_caret_fill(&edit->caret,
                         &rect,
                         line->text + edit->upleft.x + visual.x);
    else
        rtgui_caret_fill(&edit->caret,
                         &rect,
                         "");
}

struct edit_line *rtgui_edit_get_line_by_index(struct rtgui_edit *edit, rt_uint32_t index)
{
    int i;
    struct edit_line *line;

    RT_ASSERT(edit != RT_NULL);

    i = 0;
    line = edit->head;
    while (line)
    {
        if (i == index)
            return line;
        line = line->next;
        i++;
    }
    return RT_NULL;
}
RTM_EXPORT(rtgui_edit_get_line_by_index);

/** Return the y index of the @line.
 *
 * If line is not belong to @edit, -1 will be returned.
 */
rt_int32_t rtgui_edit_get_index_by_line(struct rtgui_edit *edit, struct edit_line *line)
{
    rt_uint32_t index;
    struct edit_line *tmp;

    RT_ASSERT(edit != RT_NULL);
    if (line == RT_NULL)
    {
        return RT_FALSE;
    }

    index = 0;
    for (tmp = edit->head; tmp && tmp != line; tmp = tmp->next)
    {
        index++;
    }
    if (!tmp)
        return -1;
    return index;
}
RTM_EXPORT(rtgui_edit_get_index_by_line);

static void rtgui_edit_onmouse(struct rtgui_edit *edit, struct rtgui_event_mouse *emouse)
{
    rtgui_rect_t rect;

    RT_ASSERT(edit != RT_NULL);
    RT_ASSERT(emouse);

    rtgui_widget_get_rect(RTGUI_WIDGET(edit), &rect);
    if ((rtgui_region_contains_point(&(RTGUI_WIDGET(edit)->clip), emouse->x, emouse->y, &rect) == RT_EOK))
    {
        rt_uint16_t x, y;

        x = (emouse->x - rect.x1) / (edit->font_width);
        y = (emouse->y - rect.y1) / (edit->item_height);
        if (!((x < edit->col_per_page) && (y < edit->row_per_page)))
            return;

        if (emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
        {
            struct edit_line *line;

            _edit_hide_caret(edit);

            line = rtgui_edit_get_line_by_index(edit, edit->upleft.y + y);
            if (line == RT_NULL)
            {
                edit->visual.x = 0;
                edit->visual.y = 0;
            }
            else
            {
                edit->visual.y = y;

                if (edit->upleft.x + x > line->len)
                {
                    /* Clicked on an empty area. */
                    if (edit->upleft.x <= line->len)
                    {
                        edit->visual.x = line->len - edit->upleft.x;
                    }
                    else
                    {
                        edit->upleft.x = line->len;
                        edit->visual.x = 0;
                        rtgui_edit_ondraw(edit);
                    }
                }
                else
                {
                    /* Clicked on the chars. Adjust the pos on need. */
                    struct rtgui_char_position pos;

                    pos = _string_char_width(line->text, line->len + 1,
                                             edit->upleft.x + x);

                    edit->visual.x = x + pos.char_width - pos.remain;
                }
            }

            _edit_show_caret(edit);
        }
        else if (emouse->button & RTGUI_MOUSE_BUTTON_UP)
        {
            /* Nothing to do yet. */
        }
#ifdef RTGUI_EDIT_USING_SCROLL
        if (edit->vscroll && !RTGUI_WIDGET_IS_HIDE(edit))
        {
            if (!RTGUI_WIDGET_IS_HIDE(edit->vscroll))
                rtgui_scrollbar_set_value(edit->vscroll, edit->upleft.y);
        }
        if (edit->hscroll && !RTGUI_WIDGET_IS_HIDE(edit))
        {
            if (!RTGUI_WIDGET_IS_HIDE(edit->hscroll))
                rtgui_scrollbar_set_value(edit->hscroll, edit->upleft.x);
        }
#endif
    }
}

rt_inline rt_uint16_t query_shift_code(rt_uint16_t key)
{
    if (RTGUIK_a <= key && key <= RTGUIK_z)
        return (key - 'a' + 'A');
    else if ('A' <= key && key <= 'Z')
        return (key - 'A' + 'a');
    else
    {
        switch (key)
        {
        case '1':
            return '!';
        case '2':
            return '@';
        case '3':
            return '#';
        case '4':
            return '$';
        case '5':
            return '%';
        case '6':
            return '^';
        case '7':
            return '&';
        case '8':
            return '*';
        case '9':
            return '(';
        case '0':
            return ')';
        case '-':
            return '_';
        case '=':
            return '+';
        case '\\':
            return '|';
        case ';':
            return ':';
        case '\'':
            return '"';
        case ',':
            return '<';
        case '.':
            return '>';
        case '/':
            return '?';
        case '`':
            return '~';
        case '[':
            return '{';
        case ']':
            return '}';
        }
    }
    return key;
}

rt_inline rt_uint16_t query_caps_code(rt_uint16_t key)
{
    if (key >= RTGUIK_a && key <= RTGUIK_z)
        return (key - ('a' - 'A'));
    return key;
}

rt_inline rt_uint16_t query_num_code(rt_uint16_t key)
{
    if (key >= RTGUIK_KP0 && key <= RTGUIK_KP9)
    {
        return key - (RTGUIK_KP0 - RTGUIK_0);
    }
    else if (key == RTGUIK_KP_PERIOD)
    {
        return '.';
    }
    else if (key == RTGUIK_KP_DIVIDE)
    {
        return '/';
    }
    else if (key == RTGUIK_KP_MULTIPLY)
    {
        return '*';
    }
    else if (key == RTGUIK_KP_MINUS)
    {
        return '-';
    }
    else if (key == RTGUIK_KP_PLUS)
    {
        return '+';
    }
    else if (key == RTGUIK_KP_ENTER)
    {
        return RTGUIK_RETURN;
    }
    return key;
}

void kbd_event_set_key(struct rtgui_event_kbd *ekbd, rt_uint16_t key)
{
    RTGUI_EVENT_KBD_INIT(ekbd);
    ekbd->mod  = RTGUI_KMOD_NONE;
    ekbd->unicode = 0;

    ekbd->key = key;
    ekbd->type = RTGUI_KEYDOWN;
}

static rt_bool_t rtgui_edit_onkey(struct rtgui_object *object, rtgui_event_t *event)
{
    enum { EDIT_NONE, EDIT_ONDRAW, EDIT_UPDATE };
    struct rtgui_edit *edit = RTGUI_EDIT(object);
    struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd *)event;
    struct edit_line *line = RT_NULL;
    rt_bool_t update_type = EDIT_NONE;

    RT_ASSERT(edit != RT_NULL);
    if (ekbd == RT_NULL)
    {
        return RT_FALSE;
    }

    if (RTGUI_KBD_IS_UP(ekbd))
    {
        return RT_TRUE;
    }

    line = rtgui_edit_get_line_by_index(edit, edit->upleft.y + edit->visual.y);
    if (line == RT_NULL)
    {
        if (edit->max_rows == 0)
        {
            rtgui_edit_append_line(edit, "");
            line = edit->head;
        }
    }

    if (ekbd->key == RTGUIK_DELETE)
    {
        /* delete latter character */
        int ofs = edit->upleft.x + edit->visual.x;
        if (ofs >= line->len)
        {
            /* will the next line marges into the current line */
            struct edit_line *next_line = line->next;

            if (next_line == RT_NULL)
                goto _edit_exit;

            update_type = EDIT_UPDATE;
            /* On deleting, the following lines may need cleanup. So we
             * should update from 0. */
            edit->update.start.x = 0;
            edit->update.start.y = edit->visual.y;
            edit->update.end.x = edit->col_per_page;
            edit->update.end.y = edit->row_per_page;

            rtgui_edit_connect_line(edit, line, next_line);
            line->next = next_line->next;
            if (next_line->next != RT_NULL)
                next_line->next->prev = line;
            edit->max_rows--;
            _line_add_ln_from(next_line->next, -1);
            if (edit->on_delete_line)
                edit->on_delete_line(edit, next_line);
            if (next_line->text)
                rtgui_free(next_line->text);
            rtgui_free(next_line);

            goto _edit_exit;
        }
        else if (ofs == line->len - 1)
        {
            line->text[ofs] = '\0';
        }
        else
        {
            char *c;
            int rm_len;
            if (line->text[ofs] & 0x80)
                rm_len = 2;
            else
                rm_len = 1;
            /* remove character */
            for (c = &line->text[ofs]; c[rm_len] != '\0'; c++)
                *c = c[rm_len];
            *c = '\0';
        }
        update_type = EDIT_UPDATE;
        edit->update.start = edit->visual;
        edit->update.end.x = line->len - edit->upleft.x;
        if (edit->update.end.x > edit->col_per_page)
            edit->update.end.x = edit->col_per_page;
        edit->update.end.y = edit->visual.y;
    }
    else if (ekbd->key == RTGUIK_BACKSPACE)
    {
        if (edit->visual.x == 0)
        {
            update_type = EDIT_UPDATE;
            if (edit->upleft.x > 0)
            {
                char *c;
                int char_len = _edit_char_width(edit, line, -1);

                update_type = EDIT_ONDRAW;
                /* remove character */
                for (c = &line->text[edit->upleft.x + edit->visual.x - char_len]; c[char_len] != '\0'; c++)
                {
                    *c = c[char_len];
                }
                *c = '\0';
                edit->upleft.x -= char_len;
            }
            else
            {
                /* incorporated into prev line */
                if (line->prev != RT_NULL)
                {
                    /* move cursor to line tail */
                    if (line->prev->len >= edit->col_per_page)
                    {
                        edit->visual.x = edit->col_per_page - 1;
                        edit->upleft.x = line->prev->len - (edit->col_per_page - 1);
                        update_type = EDIT_ONDRAW;
                    }
                    else
                        edit->visual.x = line->prev->len;
                    /*set the caret to prev line*/
                    if (edit->visual.y == 0)
                    {
                        RT_ASSERT(edit->upleft.y > 0);
                        edit->upleft.y -= 1;
                        update_type = EDIT_ONDRAW;
                    }
                    else
                    {
                        edit->visual.y -= 1;
                    }

                    rtgui_edit_connect_line(edit, line->prev, line);
                    if (line->next == RT_NULL)
                    {
                        /* last item */
                        line->prev->next = RT_NULL;
                    }
                    else
                    {
                        /* middle item */
                        line->prev->next = line->next;
                        line->next->prev = line->prev;
                    }

                    edit->max_rows--;
                    _line_add_ln_from(line->next, -1);
                    if (edit->on_delete_line)
                        edit->on_delete_line(edit, line);
                    if (line->text)
                        rtgui_free(line->text);
                    rtgui_free(line);
                    line = RT_NULL;
                }

                edit->update.start.x = 0;
                edit->update.start.y = edit->visual.y;
                edit->update.end.x = edit->col_per_page;
                edit->update.end.y = edit->row_per_page;
            }

            goto _edit_exit;
        }

        /* delete front character */
        if (edit->visual.x == line->len)
        {
            int char_len;
            char_len = _edit_char_width(edit, line, -1);
            line->text[edit->visual.x - char_len] = '\0';
            edit->visual.x -= char_len;
        }
        else if (edit->visual.x != 0)
        {
            /* remove current character */
            char *c;
            int rm_len;
            int pos = edit->upleft.x + edit->visual.x;

            rm_len = _edit_char_width(edit, line, -1);

            for (c = &line->text[pos - rm_len]; c[rm_len] != '\0'; c++)
            {
                *c = c[rm_len];
            }
            *c = '\0';
            edit->visual.x -= rm_len;
        }

        /* adjusted line buffer length */
        if (rtgui_edit_alloc_len(edit->bzsize, line->len + 2) < line->zsize)
        {
            line->zsize = rtgui_edit_alloc_len(edit->bzsize, line->len + 1);
            line->text = (char *)rt_realloc(line->text, line->zsize);
        }
        if (edit->visual.x == -1)
        {
            edit->visual.x = 0;
            edit->upleft.x --;
            update_type = EDIT_ONDRAW;
        }
        else
        {
            update_type = EDIT_UPDATE;
        }

        edit->update.start = edit->visual;
        edit->update.end.x = line->len;
        edit->update.end.y = edit->visual.y;
    }
    else if (ekbd->key == RTGUIK_UP)
    {
        /* move to prev line */
        struct edit_line *prev_line;
        if (edit->visual.y > 0)
            edit->visual.y --;
        else
        {
            /* change first row */
            if (edit->upleft.y > 0)
            {
                edit->upleft.y --;
                update_type = EDIT_ONDRAW;
            }
        }

        if (edit->upleft.x == 0 && edit->visual.x == 0)
            goto _edit_exit;

        /* The position of the recount X */
        prev_line = rtgui_edit_get_line_by_index(edit, edit->upleft.y + edit->visual.y);
        if (prev_line == RT_NULL)
            return RT_FALSE;
        /* We have to move the cursor as the prev line is shorter. */
        if (edit->upleft.x + edit->visual.x >= prev_line->len)
        {
            /* If the prev line is not in the page, show it. */
            if (edit->upleft.x > prev_line->len)
            {
                if (edit->col_per_page > prev_line->len)
                {
                    edit->upleft.x = 0;
                    edit->visual.x = prev_line->len;
                }
                else
                {
                    edit->upleft.x = prev_line->len - edit->col_per_page;
                    edit->visual.x = edit->col_per_page;
                }
                update_type = EDIT_ONDRAW;
            }
            else
            {
                edit->visual.x = prev_line->len - edit->upleft.x;
            }
        }
        else
        {
            edit->visual.x = _line_cursor_pos_at(prev_line,
                                                edit->upleft.x + edit->visual.x) - edit->upleft.x;
            if (edit->visual.x >= edit->col_per_page)
            {
                int char_len;

                RT_ASSERT(edit->col_per_page == edit->visual.x);
                char_len = _edit_char_width(edit, prev_line, -1);
                edit->visual.x -= char_len;
                edit->upleft.x += char_len;
                update_type = EDIT_ONDRAW;
            }
        }

#ifdef RTGUI_EDIT_USING_SCROLL
        /* update vscroll */
        if (edit->vscroll && !RTGUI_WIDGET_IS_HIDE(edit))
        {
            if (!RTGUI_WIDGET_IS_HIDE(edit->vscroll))
                rtgui_scrollbar_set_value(edit->vscroll, edit->upleft.y);
        }
#endif
    }
    else if (ekbd->key == RTGUIK_DOWN)
    {
        struct edit_line *tail_line, *next_line;
        tail_line = rtgui_edit_get_line_by_index(edit, edit->upleft.y + edit->visual.y);
        if (tail_line != RT_NULL)
        {
            /* it is tail line */
            if (tail_line->next == RT_NULL)
                return RT_FALSE;
        }
        /* move to next line */
        if (edit->visual.y < edit->row_per_page - 2)
        {
            edit->visual.y++;
        }
        else if (edit->visual.y + edit->upleft.y < edit->max_rows - 1)
        {
            /* change first row */
            edit->upleft.y++;
            update_type = EDIT_ONDRAW;
        }

        if (edit->upleft.x == 0 && edit->visual.x == 0)
            goto _edit_exit;

        /* adjust next line end position */
        next_line = rtgui_edit_get_line_by_index(edit, edit->upleft.y + edit->visual.y);
        if (next_line == RT_NULL)
            return RT_FALSE;
        /* We have to move the cursor as the prev line is shorter. */
        if (edit->upleft.x + edit->visual.x >= next_line->len)
        {
            /* If the next line is not in the page, show it. */
            if (edit->upleft.x > next_line->len)
            {
                if (edit->col_per_page > next_line->len)
                {
                    edit->upleft.x = 0;
                    edit->visual.x = next_line->len;
                }
                else
                {
                    edit->upleft.x = next_line->len - edit->col_per_page;
                    edit->visual.x = edit->col_per_page;
                }
                update_type = EDIT_ONDRAW;
            }
            else
            {
                edit->visual.x = next_line->len - edit->upleft.x;
            }
        }
        else
        {
            edit->visual.x = _line_cursor_pos_at(next_line,
                                                edit->upleft.x + edit->visual.x) - edit->upleft.x;
            if (edit->visual.x >= edit->col_per_page)
            {
                int char_len;

                RT_ASSERT(edit->col_per_page == edit->visual.x);
                char_len = _edit_char_width(edit, next_line, -1);
                edit->visual.x -= char_len;
                edit->upleft.x += char_len;
                update_type = EDIT_ONDRAW;
            }
        }

#ifdef RTGUI_EDIT_USING_SCROLL
        /* update vscroll */
        if (edit->vscroll && !RTGUI_WIDGET_IS_HIDE(edit))
        {
            if (!RTGUI_WIDGET_IS_HIDE(edit->vscroll))
                rtgui_scrollbar_set_value(edit->vscroll, edit->upleft.y);
        }
#endif
    }
    else if (ekbd->key == RTGUIK_LEFT)
    {
        if (edit->upleft.x == 0 && edit->visual.x == 0)
        {
            struct rtgui_event_kbd event_kbd;
            struct edit_line *first_line;

            first_line = rtgui_edit_get_line_by_index(edit, edit->upleft.y + edit->visual.y);
            if (first_line == edit->head)
            {
                /* it is head line */
                return RT_FALSE;
            }
            RT_ASSERT(first_line);
            /* move the caret to the prev line end */
            kbd_event_set_key(&event_kbd, RTGUIK_UP);
            rtgui_edit_onkey(object, (rtgui_event_t *)&event_kbd);
            kbd_event_set_key(&event_kbd, RTGUIK_END);
            rtgui_edit_onkey(object, (rtgui_event_t *)&event_kbd);
        }
        else
        {
            int char_len = _edit_char_width(edit, line, -1);

            edit->visual.x -= char_len;
            if (edit->visual.x < 0)
            {
                edit->upleft.x -= -edit->visual.x;
                if (edit->upleft.x < 0)
                    edit->upleft.x = 0;
                edit->visual.x = 0;
                update_type = EDIT_ONDRAW;
            }
        }
    }
    else if (ekbd->key == RTGUIK_RIGHT)
    {
        if (edit->upleft.x + edit->visual.x == line->len)
        {
            if (line->next != RT_NULL)
            {
                /* We reach the end of current line. Move into next line. */
                struct rtgui_event_kbd event_kbd;

                /* move to next head */
                /* TODO: provide an API should be better. */
                kbd_event_set_key(&event_kbd, RTGUIK_HOME);
                rtgui_edit_onkey(object, (rtgui_event_t *)&event_kbd);
                kbd_event_set_key(&event_kbd, RTGUIK_DOWN);
                rtgui_edit_onkey(object, (rtgui_event_t *)&event_kbd);
            }
        }
        else
        {
            int char_len = _edit_char_width(edit, line, 0);

            if (edit->visual.x + char_len < edit->col_per_page)
            {
                edit->visual.x += char_len;
            }
            else
            {
                edit->upleft.x += char_len;
                update_type = EDIT_ONDRAW;
            }
        }
    }
    else if (ekbd->key == RTGUIK_HOME)
    {
        /* move cursor to line head */
        edit->visual.x = 0;
        if (edit->upleft.x > 0)
        {
            edit->upleft.x = 0;
            update_type = EDIT_ONDRAW;
        }
    }
    else if (ekbd->key == RTGUIK_END)
    {
        /* move cursor to line tail */
        if (line->len >= edit->col_per_page)
        {
            edit->visual.x = edit->col_per_page - 1;
            edit->upleft.x = line->len - (edit->col_per_page - 1);
            update_type = EDIT_ONDRAW;
        }
        else
            edit->visual.x = line->len - edit->upleft.x;
        RT_ASSERT(edit->visual.x >= 0);
    }
    else if (ekbd->key == RTGUIK_TAB)
    {
        int space_nums;
        struct rtgui_event_kbd event_kbd;
        /* using spaces to replace TAB */
        space_nums = edit->tabsize - (edit->upleft.x + edit->visual.x) % edit->tabsize;
        while (space_nums--)
        {
            kbd_event_set_key(&event_kbd, RTGUIK_SPACE);
            rtgui_edit_onkey(object, (rtgui_event_t *)&event_kbd);
        }
    }
    else if (ekbd->key == RTGUIK_RETURN)
    {
        /* insert a new line buffer */
        rtgui_edit_insert_line(edit, line, line->text + edit->upleft.x + edit->visual.x);
        line->text[edit->upleft.x + edit->visual.x] = '\0';
        line->len = rtgui_edit_line_strlen(line->text);

        edit->update.start.x = 0;
        edit->update.start.y = edit->visual.y;
        edit->update.end.x = edit->col_per_page;
        edit->update.end.y = edit->row_per_page;
        update_type = EDIT_UPDATE;

        edit->visual.x = 0;
        if (edit->upleft.x != 0)
        {
            edit->upleft.x = 0;
            update_type = EDIT_ONDRAW;
        }
        if (edit->visual.y < edit->row_per_page - 2)
        {
            edit->visual.y++;
        }
        else
        {
            edit->upleft.y++;
            update_type = EDIT_ONDRAW;
        }
    }
    else
    {
        int char_width;
        rt_uint16_t input_char;

        if (ekbd->unicode)
        {
            char_width = 2;
            input_char = ekbd->unicode;
        }
        else
        {
            char_width = 1;
            input_char = ekbd->key;

            /* Shift/Caps key does not modify the number pad keys. */
            if (ekbd->mod & RTGUI_KMOD_NUM && (RTGUIK_KP0 <= input_char &&
                                               input_char <= RTGUIK_KP_EQUALS))
            {
                input_char = query_num_code(input_char);
            }
            else
            {
                if (ekbd->mod & RTGUI_KMOD_CAPS)
                    input_char = query_caps_code(input_char);
                if ((ekbd->mod & RTGUI_KMOD_LSHIFT) || (ekbd->mod & RTGUI_KMOD_RSHIFT))
                    input_char = query_shift_code(input_char);
            }
        }

        if (char_width == 1 && (input_char >= 127 || !isprint(input_char)))
            return RT_FALSE;

        /* it's may print character */
        update_type = EDIT_UPDATE;
        edit->update.start = edit->visual;

        if (line->len < line->zsize - char_width)
        {
            int ofs = edit->upleft.x + edit->visual.x;
            if (edit->visual.x >= edit->col_per_page - char_width)
            {
                edit->upleft.x += char_width;
                update_type = EDIT_ONDRAW;
            }

            if (ofs < line->len)
            {
                char *c;
                for (c = &line->text[line->len + char_width - 1];
                     c != &line->text[ofs];
                     c--)
                    *c = *(c - char_width);
            }
            if (char_width == 1)
            {
                line->text[ofs] = input_char;
            }
            else if (char_width == 2)
            {
                /* little endian */
                line->text[ofs]   = input_char >> 8;
                line->text[ofs + 1] = input_char & 0xFF;
            }
            else
            {
                return RT_FALSE;
            }
            if (edit->visual.x < edit->col_per_page - char_width)
                edit->visual.x += char_width;
            line->text[line->len + char_width] = '\0';
            line->len = rtgui_edit_line_strlen(line->text);
            edit->update.end.x = line->len;
            if (edit->update.end.x > edit->col_per_page)
                edit->update.end.x = edit->col_per_page;
            edit->update.end.y = edit->visual.y;
        }
        else
        {
            char *tmp;
            int zsize;

            /* adjust line buffer's zone size */
            zsize = rtgui_edit_alloc_len(edit->bzsize, line->len + char_width);
            tmp = (char *)rt_realloc(line->text, zsize);
            if (!tmp)
                return RT_TRUE;
            line->zsize = zsize;
            line->text = tmp;
            rtgui_edit_onkey(object, event); /* reentry */
        }
    }
    line->len = rtgui_edit_line_strlen(line->text);

_edit_exit:
    /* We need to hide the carect *before* drawing and reset the carect *after*
     * drawing to get the buffer right. */
    _edit_hide_caret(edit);

    /* re-draw edit widget */
    if (update_type == EDIT_ONDRAW)
        rtgui_edit_ondraw(edit);
    else if (update_type == EDIT_UPDATE)
        rtgui_edit_update(edit);

    _edit_show_caret(edit);

    return RT_TRUE;
}

static rt_bool_t rtgui_edit_onfocus(struct rtgui_object *object, rtgui_event_t *event)
{
    struct rtgui_edit *edit = RTGUI_EDIT(object);

    rtgui_caret_start_timer(&edit->caret, RTGUI_EDIT_CARET_TIMEOUT);

    return RT_TRUE;
}

static rt_bool_t rtgui_edit_onunfocus(struct rtgui_object *object, rtgui_event_t *event)
{
    struct rtgui_edit *edit = RTGUI_EDIT(object);

    _edit_hide_caret(edit);
    rtgui_caret_stop_timer(&edit->caret);

    return RT_TRUE;
}

#ifdef RTGUI_EDIT_USING_SCROLL
static rt_bool_t rtgui_edit_hscroll_handle(struct rtgui_widget *widget, rtgui_event_t *event)
{
    struct rtgui_edit *edit = RTGUI_EDIT(widget);

    /* adjust first display row when dragging */
    edit->upleft.y = edit->hscroll->value;

    rtgui_edit_ondraw(edit);

    return RT_TRUE;
}

static rt_bool_t rtgui_edit_vscroll_handle(struct rtgui_widget *widget, rtgui_event_t *event)
{
    struct rtgui_edit *edit = RTGUI_EDIT(widget);

    /* adjust first display row when dragging */
    edit->upleft.x = edit->vscroll->value;

    rtgui_edit_ondraw(edit);

    return RT_TRUE;
}
#endif

/* local area update */
static void rtgui_edit_update(struct rtgui_edit *edit)
{
    rt_int16_t i;
    rtgui_rect_t rect;
    struct rtgui_dc *dc;
    int draw_border = 0;

    RT_ASSERT(edit != RT_NULL);

    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(edit));
    if (dc == RT_NULL)
        return;

    rtgui_widget_get_rect(RTGUI_WIDGET(edit), &rect);
    rtgui_rect_inflate(&rect, -(edit->margin + RTGUI_WIDGET_BORDER(edit)));


    for (i = edit->update.start.y; i <= edit->update.end.y; i++)
    {
        char *src;
        int src_offset;
        rtgui_rect_t r;
        struct rtgui_char_position pos;
        struct edit_line *line = rtgui_edit_get_line_by_index(edit, edit->upleft.y + i);

        if (i > edit->upleft.y + edit->row_per_page)
            break;

        if (line == RT_NULL)
        {
            /* The line is deleted, clean its room. */
            r.x1 = rect.x1;
            r.x2 = rect.x2;
            r.y1 = rect.y1 + i * edit->font_height;
            if (r.y1 > rect.y2)
                break;
            r.y2 = r.y1 + edit->font_height;
            if (r.y2 > rect.y2)
                r.y2 = rect.y2;
            rtgui_dc_fill_rect(dc, &r);
            continue;
        }

        src_offset = edit->update.start.x + edit->upleft.x;

        r.x1 = rect.x1 + edit->update.start.x * edit->font_width;
        if (r.x1 > rect.x2)
            break;
        r.y1 = rect.y1 + i * edit->font_height;
        if (r.y1 > rect.y2)
            break;
        r.x2 = rect.x1 + edit->update.end.x * edit->font_width;
        if (r.x2 > rect.x2)
            r.x2 = rect.x2;
        r.y2 = r.y1 + edit->font_height;
        if (r.y2 > rect.y2)
            r.y2 = rect.y2;

        if (src_offset > line->len)
            src = line->text + line->len;
        else
            src = line->text + src_offset;

        /* Fixup the wchar. */
        pos = _string_char_width(line->text, line->len + 1,
                                 src - line->text);
        if (pos.char_width != pos.remain)
        {
            r.x1 -= edit->font_width;
            src -= pos.remain;
            draw_border = 1;
        }

        rtgui_dc_fill_rect(dc, &r);
        rtgui_dc_draw_text(dc, src, &r);
    }

    if (draw_border)
    {
        rtgui_widget_get_rect(RTGUI_WIDGET(edit), &rect);
        /* We have to draw the border at least so it could cover the partly drawed
         * fonts. */
        rtgui_dc_draw_border(dc, &rect, RTGUI_WIDGET_BORDER_STYLE(edit));
    }
    rtgui_dc_end_drawing(dc);
}

void rtgui_edit_ondraw(struct rtgui_edit *edit)
{
    rtgui_rect_t rect, r;
    struct rtgui_dc *dc;
#ifdef RTGUI_EDIT_USING_SCROLL
    int hscroll_flag = 0;
    int vscroll_flag = 0;
#endif

    RT_ASSERT(edit != RT_NULL);

    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(edit));
    if (dc == RT_NULL)
        return;

    /* get widget rect */
    rtgui_widget_get_rect(RTGUI_WIDGET(edit), &rect);
    rtgui_rect_inflate(&rect, -RTGUI_WIDGET_BORDER(edit));

    /* fill widget rect with edit background color */
    RTGUI_DC_BC(dc) = RTGUI_WIDGET_BACKGROUND(edit);
    rtgui_dc_fill_rect(dc, &rect);

    rtgui_rect_inflate(&rect, -edit->margin);

#ifdef RTGUI_EDIT_USING_SCROLL
    if (edit->vscroll && !RTGUI_WIDGET_IS_HIDE(edit->vscroll))
    {
        rect.x2 = rect.x2 - rtgui_rect_width(edit->vscroll->parent.extent);
    }
    if (edit->hscroll && !RTGUI_WIDGET_IS_HIDE(edit->hscroll))
    {
        rect.y2 = rect.y2 - rtgui_rect_height(edit->hscroll->parent.extent);
    }
#endif
    r = rect;

    /* draw text */
    if (edit->head != RT_NULL)
    {
        struct edit_line *line;
        int num = 0;

        line =  rtgui_edit_get_line_by_index(edit, edit->upleft.y);
        rect.y2 = rect.y1 + edit->item_height;
        while (line)
        {
            if (edit->upleft.x < line->len)
            {
                int orig_x1 = rect.x1;
                char *str = line->text + edit->upleft.x;
                struct rtgui_char_position pos;

                pos = _string_char_width(line->text, line->len + 1,
                                         edit->upleft.x);
                if (pos.char_width != pos.remain)
                {
                    str -= pos.remain;
                    rect.x1 = -edit->font_width + RTGUI_WIDGET_BORDER(edit);
                }
                rtgui_dc_draw_text(dc, str, &rect);
                rect.x1 = orig_x1;
            }

            line = line->next;

            rect.y1 += edit->item_height;
            if ((rect.y1 + edit->item_height) < r.y2)
                rect.y2 = rect.y1 + edit->item_height;
            else
                rect.y2 = r.y2;

            if (num++ >= edit->row_per_page)
                break;
        }
    }

#ifdef RTGUI_EDIT_USING_SCROLL
    if (edit->hscroll && !RTGUI_WIDGET_IS_HIDE(edit->hscroll))
    {
        hscroll_flag = 1;
        rtgui_scrollbar_ondraw(edit->hscroll);
    }
    if (edit->vscroll && !RTGUI_WIDGET_IS_HIDE(edit->vscroll))
    {
        vscroll_flag = 1;
        rtgui_scrollbar_ondraw(edit->vscroll);
    }

    if (hscroll_flag && vscroll_flag)
    {
        rtgui_color_t _bc;
        rtgui_widget_get_rect(RTGUI_WIDGET(edit), &rect);
        rect.x1 = rect.x2 - RTGUI_WIDGET_BORDER(edit);
        rect.y1 = rect.y2 - RTGUI_WIDGET_BORDER(edit);
        _bc = RTGUI_DC_BC(dc);
        RTGUI_DC_BC(dc) = default_background;
        rtgui_dc_fill_rect(dc, &rect);
        RTGUI_DC_BC(dc) = _bc;
    }
#endif

    /* get widget rect */
    rtgui_widget_get_rect(RTGUI_WIDGET(edit), &rect);
    /* We have to draw the border at least so it could cover the partly drawed
     * fonts. */
    rtgui_dc_draw_border(dc, &rect, RTGUI_WIDGET_BORDER_STYLE(edit));

    rtgui_dc_end_drawing(dc);
}

rt_bool_t rtgui_edit_event_handler(struct rtgui_object *object, rtgui_event_t *event)
{
#ifndef RTGUI_USING_SMALL_SIZE
    rtgui_widget_t *widget = RTGUI_WIDGET(object);
#endif
    struct rtgui_edit *edit = RTGUI_EDIT(object);

    switch (event->type)
    {
    case RTGUI_EVENT_PAINT:
        rtgui_edit_ondraw(edit);
        break;

    case RTGUI_EVENT_MOUSE_BUTTON:
        rtgui_edit_onmouse(edit, (struct rtgui_event_mouse *)event);
        return RT_TRUE;

    case RTGUI_EVENT_KBD:
#ifndef RTGUI_USING_SMALL_SIZE
        if (widget->on_key != RT_NULL)
            widget->on_key(object, event);
        else
#endif
            rtgui_edit_onkey(object, event);
        return RT_TRUE;

    default:
        return rtgui_widget_event_handler(object, event);
    }

    return RT_FALSE;
}

rtgui_point_t rtgui_edit_get_current_point(struct rtgui_edit *edit)
{
    rtgui_point_t p = {0, 0};

    RT_ASSERT(edit != RT_NULL);

    p.x = edit->upleft.x + edit->visual.x;
    p.y = edit->upleft.y + edit->visual.y;

    return p;
}

rt_uint32_t rtgui_edit_get_mem_consume(struct rtgui_edit *edit)
{
    rt_uint32_t mem_size;
    struct edit_line *line;

    mem_size = sizeof(struct rtgui_edit);
    if (edit->head != RT_NULL)
    {
        line = edit->head;
        while (line)
        {
            mem_size += line->zsize;
            mem_size += sizeof(struct edit_line);
            line = line->next;
        }
    }

    return mem_size;
}

#ifdef RTGUI_USING_DFS_FILERW
/**
 * File access component, General File Access Interface
 */
rt_bool_t rtgui_edit_readin_file(struct rtgui_edit *edit, const char *filename)
{
    struct rtgui_filerw *filerw;
    int num = 0, read_bytes, size , len = 0;
    char *text , ch;

    filerw = rtgui_filerw_create_file(filename, "rb");
    if (filerw == RT_NULL)
        return RT_FALSE;
    rtgui_edit_clear_text(edit);

    /**
     * If it was in the debug of the win32, If document encode is UTF-8 or Unicode,
     * Will read to garbled code when using the function read documents.
     * You can Change of the document contains the source code for ANSI.
     */
    size = edit->bzsize;
    text = (char *)rtgui_malloc(size);
    if (text == RT_NULL)
        return RT_FALSE;

    do {
        if ((read_bytes = rtgui_filerw_read(filerw, &ch, 1, 1)) > 0)
        {
            if (num >= size - 1)
                text = (char *)rt_realloc(text, rtgui_edit_alloc_len(size, num));
            if (ch == 0x09) //Tab
            {
                len = edit->tabsize - num % edit->tabsize;
                while (len--)
                    *(text + num++) = ' ';
            }
            else
                *(text + num++) = ch;
            if (ch == 0x0A)
            {
                rtgui_edit_append_line(edit, text);
                num = 0;
            }

        }
        else if (num > 0)
        {
            /* last line does not exist the end operator */
            *(text + num) = '\0';
            rtgui_edit_append_line(edit, text);
        }
    } while (read_bytes);

    rtgui_filerw_close(filerw);
    rtgui_free(text);
    rtgui_edit_ondraw(edit);

    return RT_TRUE;
}

rt_bool_t rtgui_edit_saveas_file(struct rtgui_edit *edit, const char *filename)
{
    struct rtgui_filerw *filerw;
    char ch_tailed = 0x0A;
    struct edit_line *line;

    filerw = rtgui_filerw_create_file(filename, "wb");
    if (filerw == RT_NULL) return RT_FALSE;

    line = edit->head;
    while (line)
    {
        rtgui_filerw_write(filerw, line->text, line->len, 1);
        if (line->next != RT_NULL)
            rtgui_filerw_write(filerw, &ch_tailed, 1, 1);
        line = line->next;
    }

    rtgui_filerw_close(filerw);

    return RT_TRUE;
}
#endif
