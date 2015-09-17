/*
* File      : textbox.c
* This file is part of RT-Thread RTOS
* COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
*
* The license and distribution terms for this file may be
* found in the file LICENSE in this distribution or at
* http://www.rt-thread.org/license/LICENSE
*
* Change Logs:
* Date           Author       Notes
* 2009-10-16     Bernard      first version
* 2011-01-224    Bernard      fix backspace issue.
* 2012-06-09     asml         refactor
* 2012-06-17     Grissiom     misc cleanup and merge
*/
#include <string.h>
#include <rtgui/widgets/textbox.h>
#include <rtgui/widgets/combobox.h>
#include <rtgui/rtgui_system.h>

#include <ctype.h>

#include "text_encoding.h"

#define TB_ABSPOS(tb)   ((tb)->first_pos + (tb)->position)

static rt_bool_t rtgui_textbox_onkey(struct rtgui_object *widget, rtgui_event_t *event);
static rt_bool_t rtgui_textbox_onunfocus(struct rtgui_object *widget, rtgui_event_t *event);

static void _rtgui_textbox_constructor(rtgui_textbox_t *box)
{
	rtgui_rect_t rect;

	RTGUI_WIDGET_FLAG(RTGUI_WIDGET(box)) |= RTGUI_WIDGET_FLAG_FOCUSABLE;

	rtgui_object_set_event_handler(RTGUI_OBJECT(box), rtgui_textbox_event_handler);
	rtgui_widget_set_onfocus(RTGUI_WIDGET(box), rtgui_textbox_onfocus);
	rtgui_widget_set_onunfocus(RTGUI_WIDGET(box), rtgui_textbox_onunfocus);
#ifndef RTGUI_USING_SMALL_SIZE
	rtgui_widget_set_onkey(RTGUI_WIDGET(box), rtgui_textbox_onkey);
#endif

	RTGUI_WIDGET_FOREGROUND(box) = black;
	RTGUI_WIDGET_BACKGROUND(box) = white;
	/* set default text align */
	RTGUI_WIDGET_TEXTALIGN(box) = RTGUI_ALIGN_CENTER_VERTICAL;
    rtgui_caret_init(&box->caret, RTGUI_WIDGET(box));

	box->line = box->line_begin = box->position = 0;
	box->flag = RTGUI_TEXTBOX_SINGLE;
	/* allocate default line buffer */
	box->text = RT_NULL;
	rtgui_textbox_set_mask_char(box, '*');

	rtgui_font_get_metrics(RTGUI_WIDGET_FONT(box), "H", &rect);
    rtgui_widget_set_minheight(RTGUI_WIDGET(box),
            rtgui_rect_height(rect) + RTGUI_TEXTBOX_BORDER_WIDTH * 2);
    /* at least, we want to display one char. */
    rtgui_widget_set_minwidth(RTGUI_WIDGET(box),
            rtgui_rect_width(rect) + RTGUI_TEXTBOX_BORDER_WIDTH * 2 \
            + RTGUI_WIDGET_DEFAULT_MARGIN /* there is a margin in the beginning
                                             of the text. */
            );
    box->font_width = rtgui_rect_width(rect);
	box->on_enter = RT_NULL;
	box->dis_length = 0;
	box->first_pos = 0;
}

static void _rtgui_textbox_deconstructor(rtgui_textbox_t *textbox)
{
	if (textbox->text != RT_NULL)
	{
		rtgui_free(textbox->text);
		textbox->text = RT_NULL;
	}
    rtgui_caret_cleanup(&textbox->caret);
}

DEFINE_CLASS_TYPE(textbox, "textbox",
				  RTGUI_PARENT_TYPE(widget),
				  _rtgui_textbox_constructor,
				  _rtgui_textbox_deconstructor,
				  sizeof(struct rtgui_textbox));

static void rtgui_textbox_get_caret_rect(rtgui_textbox_t *box,
                                         rtgui_rect_t *rect,
                                         rt_uint16_t position)
{
	int font_h;
	rtgui_rect_t item_rect;

	RT_ASSERT(box != RT_NULL);

	rtgui_font_get_metrics(RTGUI_WIDGET_FONT(box), "H", &item_rect);
	font_h = rtgui_rect_height(item_rect);
    box->font_width = rtgui_rect_width(item_rect);

	rtgui_widget_get_rect(RTGUI_WIDGET(box), rect);

    rect->x1 = box->font_width * position + RTGUI_WIDGET_DEFAULT_MARGIN;
	rect->x2 = rect->x1 + 2;
	rect->y1 += (rtgui_rect_height(*rect) - font_h) / 2;
	rect->y2 = rect->y1 + font_h;
}

static void rtgui_textbox_init_caret(rtgui_textbox_t *box, rt_uint16_t position)
{
	rtgui_rect_t rect;

	RT_ASSERT(box != RT_NULL);

	rtgui_textbox_get_caret_rect(box, &rect, position - box->first_pos);

    rtgui_caret_fill(&box->caret, &rect, box->text + position);
}

static void rtgui_textbox_onmouse(rtgui_textbox_t *box, struct rtgui_event_mouse *event)
{
	rt_size_t length;

	RT_ASSERT(box != RT_NULL);
	RT_ASSERT(event != RT_NULL);

	length = rt_strlen(box->text);

	if (event->button & RTGUI_MOUSE_BUTTON_LEFT && event->button & RTGUI_MOUSE_BUTTON_DOWN)
	{
		rt_int32_t x;
		/* single line text */
		/* set caret position */
		x = event->x - RTGUI_WIDGET(box)->extent.x1;
		if (x < 0)
		{
			box->position = 0;
		}
		else if (x > (length - box->first_pos) * box->font_width)
		{
            /* Hit beyond the end of the line. */
			box->position = length - box->first_pos;
		}
		else
		{
            struct rtgui_char_position cpos;
            int offset = x / box->font_width;

            cpos = _string_char_width(box->text, length, box->first_pos + offset);
			box->position = offset - (cpos.char_width - cpos.remain);
		}

		if (rtgui_caret_is_shown(&box->caret))
		{
			rtgui_caret_clear(&box->caret);
		}

		rtgui_textbox_init_caret(box, TB_ABSPOS(box));
		rtgui_caret_show(&box->caret);
	}
}

static rt_bool_t rtgui_textbox_onkey(struct rtgui_object *widget, rtgui_event_t *event)
{
	rtgui_textbox_t *box = RTGUI_TEXTBOX(widget);
	struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd *)event;
	rt_size_t length;

	RT_ASSERT(box != RT_NULL);
	RT_ASSERT(ekbd != RT_NULL);

	/* handle the key at down time and nothing to do with up */
	if (RTGUI_KBD_IS_UP(ekbd))
		return RT_TRUE;

	if (box->dis_length == 0)
	{
		rtgui_rect_t rect;

		rtgui_widget_get_rect(RTGUI_WIDGET(box), &rect);

		if (box->font_width == 0)
			return RT_FALSE;

		box->dis_length = ((rtgui_rect_width(rect) - 5) / box->font_width) & ~0x1;
	}

	length = rt_strlen(box->text);
	if (ekbd->key == RTGUIK_DELETE)
	{
        int chw;
        char *c;

        if (TB_ABSPOS(box) == length)
        {
            goto _exit;
        }
        chw = _string_char_width(box->text, length, TB_ABSPOS(box)).char_width;

        /* remove character */
        for (c = &box->text[TB_ABSPOS(box)]; c[chw] != '\0'; c++)
            *c = c[chw];
        *c = '\0';
	}
	else if (ekbd->key == RTGUIK_BACKSPACE)
	{
		/* delete front character */
		if (box->position == 0)
		{
			if(box->first_pos == 0)
                goto _exit;

            if(box->first_pos > box->dis_length)
            {
                int head_fix;
                int chw = _string_char_width(box->text, length, TB_ABSPOS(box) - 1).char_width;

                rt_memmove(box->text + TB_ABSPOS(box) - chw,
                           box->text + TB_ABSPOS(box),
                           length - TB_ABSPOS(box) + 1);

                head_fix = 0;
                /* FIXME: */
                if (box->text[box->first_pos - box->dis_length - chw] & 0x80)
                {
                    int i;

                    for (i = box->first_pos - box->dis_length - chw;
                         i < box->first_pos;
                         i++)
                    {
                        if (box->text[i] & 0x80)
                            head_fix++;
                        else
                            break;
                    }
                    /* if the head is in middle of wide char, move one less
                     * byte */
                    head_fix = head_fix % 2;
                }

                box->first_pos = box->first_pos - box->dis_length
                                 + head_fix - chw;
                box->position  = box->dis_length - head_fix;
            }
            else
            {
                int chw;
                if (!(box->text[TB_ABSPOS(box) - 1] & 0x80))
                {
                    /* also copy the \0 */
                    rt_memmove(box->text + TB_ABSPOS(box) - 1,
                               box->text + TB_ABSPOS(box),
                                  length - TB_ABSPOS(box) + 1);
                    chw = 1;
                }
                else
                {
                    rt_memmove(box->text + TB_ABSPOS(box) - 2,
                               box->text + TB_ABSPOS(box),
                                  length - TB_ABSPOS(box) + 1);
                    chw = 2;
                }
                box->position = box->first_pos - chw;
                box->first_pos = 0;
            }
		}
		else
		{
            char *c;
            int chw;

            chw = _string_char_width(box->text, length, TB_ABSPOS(box) - 1).char_width;

            /* remove character */
            for (c = &box->text[TB_ABSPOS(box) - chw]; c[chw] != '\0'; c++)
                *c = c[chw];
            *c = '\0';

            box->position -= chw;
		}
	}
	else if (ekbd->key == RTGUIK_LEFT)
	{
        int chw;

        if (box->first_pos == 0 && box->position == 0)
            goto _exit;

        if (box->text[TB_ABSPOS(box) - 1] & 0x80)
            chw = 2;
        else
            chw = 1;

		/* move to prev */
		if (box->position >= chw)
		{
            box->position -= chw;
		}
		else
		{
            if (box->first_pos >= chw)
                box->first_pos -= chw;
            else
                box->first_pos = 0;
		}
	}
	else if (ekbd->key == RTGUIK_RIGHT)
	{
        int chw;

        if ((TB_ABSPOS(box) + 2) <= length &&
            box->text[TB_ABSPOS(box)] & 0x80)
            chw = 2;
        else
            chw = 1;

		/* move to next */
		if (TB_ABSPOS(box) < length)
		{
			if(box->position + chw <= box->dis_length)
				box->position += chw;
			else
            {
                /* always move one wide char when the first char is wide */
                if (box->text[box->first_pos] & 0x80)
                {
                    box->first_pos += 2;
                    if (chw == 2)
                    {
                        int i;
                        int head_fix = 0;
                        for (i = box->first_pos;
                             i < box->first_pos + box->dis_length;
                             i++)
                        {
                            if (box->text[i] & 0x80)
                                head_fix++;
                        }
                        head_fix %= 2;
                        if (head_fix)
                        {
                            box->first_pos += 2;
                            box->position = box->dis_length - 1;
                        }
                    }
                    else if (chw == 1)
                        /* we have moved the box by 2 bytes but the last one is
                         * a narrow char */
                        box->position -= 1;
                    else
                        RT_ASSERT(0);
                }
                else
                    box->first_pos += chw;
            }
		}
	}
	else if (ekbd->key == RTGUIK_HOME)
	{
		/* move cursor to start */
		box->position = 0;
		box->first_pos = 0;
	}
	else if (ekbd->key == RTGUIK_END)
	{
		/* move cursor to end */
		if(length > box->dis_length)
		{
			box->position = box->dis_length;
			box->first_pos = length - box->dis_length;
		}
		else
		{
			box->position = length;
			box->first_pos = 0;
		}
	}
	else if (ekbd->key == RTGUIK_RETURN)
	{
		if (box->on_enter != RT_NULL)
		{
			box->on_enter(box, event);
		}
	}
	else if (ekbd->key == RTGUIK_NUMLOCK)
	{
		/* change numlock state */
		/*
		extern void update_number_lock(void);
		update_number_lock();
		*/
	}
	else
    {
        rt_uint16_t chr;
        int chw;

        if (!(ekbd->unicode || isprint(ekbd->key)))
            goto _exit;

        if (ekbd->unicode)
        {
            chr = ekbd->unicode;
            chw = 2;
        }
        else
        {
            chr = ekbd->key;
            chw = 1;
        }

        if (box->flag & RTGUI_TEXTBOX_DIGIT)
        {
            /* only input digit */
            if (!isdigit(chr))
            {
                /* exception: '.' and '-' */
                if (chr != '.' && chr != '-')return RT_FALSE;
                if (chr == '.' && strchr((char*)box->text, '.'))return RT_FALSE;

                if (chr == '-')
                {
                    if (length + chw > box->line_length) return RT_FALSE;

                    if (strchr((char*)box->text, '-'))
                    {
                        char *c;
                        for (c = &box->text[0]; c != &box->text[length]; c++)
                            *c = *(c + 1);
                        box->text[length] = '\0';
                        box->position --;
                        goto _exit;
                    }
                    else
                    {
                        char *c;
                        for (c = &box->text[length]; c != &box->text[0]; c--)
                            *c = *(c - 1);
                        box->text[0] = '-';
                        box->text[length + 1] = '\0';
                        box->position ++;
                        goto _exit;
                    }
                }
            }
        }

        if (length + chw > box->line_length)
            return RT_FALSE;

        if (TB_ABSPOS(box) <= length - 1)
        {
            char *c;

            for (c = &box->text[length + chw - 1];
                 c != &box->text[TB_ABSPOS(box)];
                 c -= 1)
                *c = *(c - chw);
        }

        if (chw == 1)
        {
            box->text[TB_ABSPOS(box)] = chr;
        }
        else if (chw == 2)
        {
            box->text[TB_ABSPOS(box)] = chr >> 8;
            box->text[TB_ABSPOS(box)+1] = chr & 0xFF;
        }
        else
        {
            RT_ASSERT(0);
        }
        box->text[length + chw] = '\0';

        if (box->position < box->dis_length)
        {
            box->position += chw;
        }
        else
        {
            if (box->text[box->first_pos] & 0x80)
            {
                box->first_pos += 2;
                if (chw == 1)
                    box->position--;
            }
            else if (chw == 2 &&
                     box->text[box->first_pos+1] & 0x80)
            {
                box->first_pos += 3;
                box->position  -= 1;
            }
            else
                box->first_pos += chw;
        }
    }

_exit:
	if (rtgui_caret_is_shown(&box->caret))
	{
		rtgui_caret_clear(&box->caret);/* refresh it */
	}

	/* re-draw text box */
	rtgui_textbox_ondraw(box);

	rtgui_textbox_init_caret(box, TB_ABSPOS(box));
	rtgui_caret_show(&box->caret);

	return RT_TRUE;
}

rt_bool_t rtgui_textbox_onfocus(struct rtgui_object *widget, rtgui_event_t *event)
{
    rtgui_textbox_t *box = RTGUI_TEXTBOX(widget);

    rtgui_caret_start_timer(&box->caret, RT_TICK_PER_SECOND);
    rtgui_textbox_init_caret(box, TB_ABSPOS(box));

    return RT_TRUE;
}

static rt_bool_t rtgui_textbox_onunfocus(struct rtgui_object *widget, rtgui_event_t *event)
{
	rtgui_textbox_t *box = RTGUI_TEXTBOX(widget);

    rtgui_caret_stop_timer(&box->caret);
	/* set caret to hide */
	rtgui_caret_clear(&box->caret);

	if (box->on_enter != RT_NULL)
		box->on_enter(box, event);

	return RT_TRUE;
}

rtgui_textbox_t *rtgui_textbox_create(const char *text, enum rtgui_textbox_flag flag)
{
	rtgui_textbox_t *box;

	box = (struct rtgui_textbox *)rtgui_widget_create(RTGUI_TEXTBOX_TYPE);
    if (!box)
        return RT_NULL;

    /* allocate default line buffer */
    rtgui_textbox_set_value(box, text);
    box->flag = flag;

	return box;
}
RTM_EXPORT(rtgui_textbox_create);

void rtgui_textbox_destroy(rtgui_textbox_t *box)
{
	rtgui_widget_destroy(RTGUI_WIDGET(box));
}
RTM_EXPORT(rtgui_textbox_destroy);

void rtgui_textbox_ondraw(rtgui_textbox_t *box)
{
	/* draw button */
	rtgui_rect_t rect;
	struct rtgui_dc *dc;
	rtgui_color_t fc;

	RT_ASSERT(box != RT_NULL);

	/* begin drawing */
	dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(box));
	if (dc == RT_NULL)
		return;

	/* get widget rect */
	rtgui_widget_get_rect(RTGUI_WIDGET(box), &rect);
	fc = RTGUI_WIDGET_FOREGROUND(box);

	rtgui_rect_inflate(&rect, -RTGUI_TEXTBOX_BORDER_WIDTH);

	/* fill widget rect with white color */
	RTGUI_WIDGET_BACKGROUND(box) = white;
	rtgui_dc_fill_rect(dc, &rect);

	rtgui_rect_inflate(&rect, RTGUI_TEXTBOX_BORDER_WIDTH);
	/* draw border */
	RTGUI_WIDGET_FOREGROUND(box) = RTGUI_RGB(123, 158, 189);
	rtgui_dc_draw_rect(dc, &rect);

	/* draw text */
	RTGUI_WIDGET_FOREGROUND(box) = fc;
	if (box->text != RT_NULL)
	{
		rect.x1 += RTGUI_WIDGET_DEFAULT_MARGIN;
		/* draw single text */
		if (box->flag & RTGUI_TEXTBOX_MASK)
		{
			/* draw mask char */
			rt_size_t len = rt_strlen(box->text);
			if (len > 0)
			{
				char *text_mask = rtgui_malloc(len + 1);

                if (!text_mask)
                    goto _out;
				rt_memset(text_mask, box->mask_char, len + 1);
				text_mask[len] = 0;
				rtgui_dc_draw_text(dc, text_mask+box->first_pos, &rect);
				rtgui_free(text_mask);
			}
		}
		else
		{
			rtgui_dc_draw_text(dc, box->text + box->first_pos, &rect);
		}
	}

_out:
	rtgui_dc_end_drawing(dc);
}

/* set textbox text */
void rtgui_textbox_set_value(rtgui_textbox_t *box, const char *text)
{
    int cshow;

    cshow = rtgui_caret_is_shown(&box->caret);
    /* If the caret is shown before, reset it. */
    if (cshow)
    {
        rtgui_caret_clear(&box->caret);
    }

    if (box->text != RT_NULL)
    {
        /* yet exist something */
        /* free the old text */
        rtgui_free(box->text);
        box->text = RT_NULL;
    }

    /* no something */
    box->line_length = ((rt_strlen(text) + 1) / RTGUI_TEXTBOX_LINE_MAX + 1) * RTGUI_TEXTBOX_LINE_MAX;

    /* allocate line buffer */
    box->text = rtgui_malloc(box->line_length+1);
    rt_memset(box->text, 0, box->line_length+1);

    /* copy text */
    rt_memcpy(box->text, text, rt_strlen(text) + 1);

    /* set current position */
    box->position = rt_strlen(text);

    if (cshow)
    {
        /* Reset the caret to get the pixel buffer right. */
        rtgui_textbox_init_caret(box, TB_ABSPOS(box));
        /* Than show it. */
        rtgui_caret_show(&box->caret);
    }
}
RTM_EXPORT(rtgui_textbox_set_value);

const char *rtgui_textbox_get_value(rtgui_textbox_t *box)
{
	return (const char *)box->text;
}
RTM_EXPORT(rtgui_textbox_get_value);

void rtgui_textbox_set_mask_char(rtgui_textbox_t *box, const char ch)
{
	box->mask_char = ch;
}

char rtgui_textbox_get_mask_char(rtgui_textbox_t *box)
{
	return box->mask_char;
}

rt_err_t rtgui_textbox_set_line_length(rtgui_textbox_t *box, rt_size_t length)
{
    char *new_line;

    RT_ASSERT(box != RT_NULL);

    /* invalid length */
    if (length <= 0)
        return -RT_ERROR;

    new_line = (char*)rtgui_realloc(box->text, length+1);
    if (new_line == RT_NULL)
        return -RT_ENOMEM;

    if (length < box->line_length)
        new_line[length] = '\0';

    box->line_length = length;
    box->text = new_line;

    return RT_EOK;
}

/* get textbox text area */
void rtgui_textbox_get_edit_rect(rtgui_textbox_t *box, rtgui_rect_t *rect)
{
	rtgui_widget_get_rect(RTGUI_WIDGET(box), rect);
	rtgui_rect_inflate(rect, -1);
}

rt_bool_t rtgui_textbox_event_handler(struct rtgui_object *object, rtgui_event_t *event)
{
	rtgui_widget_t *widget = RTGUI_WIDGET(object);
	rtgui_textbox_t *box = RTGUI_TEXTBOX(object);

	switch (event->type)
	{
	case RTGUI_EVENT_PAINT:
		rtgui_textbox_ondraw(box);
		break;

	case RTGUI_EVENT_MOUSE_BUTTON:
		rtgui_textbox_onmouse(box, (struct rtgui_event_mouse *)event);
		return RT_TRUE;

	case RTGUI_EVENT_KBD:
#ifndef RTGUI_USING_SMALL_SIZE
		if (widget->on_key != RT_NULL)
			widget->on_key(RTGUI_OBJECT(widget), event);
		else
#endif
			rtgui_textbox_onkey(RTGUI_OBJECT(box), (struct rtgui_event *)event);
		return RT_TRUE;

	default:
		return rtgui_widget_event_handler(RTGUI_OBJECT(widget), event);
	}

	return RT_FALSE;
}
RTM_EXPORT(rtgui_textbox_event_handler);

