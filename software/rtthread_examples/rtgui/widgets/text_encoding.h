#ifndef __TEXT_ENCODING_H__
#define __TEXT_ENCODING_H__

#include <stddef.h>

struct rtgui_char_position
{
    /* Keep the size of this struct within 4 bytes so it can be passed by
     * value. */
    /* How long this char is. */
    rt_uint16_t char_width;
    /* How many bytes remaining from current pointer. At least, it will be 1. */
    rt_uint16_t remain;
};

/*
 * @len the length of @str.
 * @offset the char offset on the string to check with.
 */
struct rtgui_char_position _string_char_width(char *str, size_t len, size_t offset);

#endif /* end of include guard: __TEXT_ENCODING_H__ */

