#include <rtthread.h>
// #include <stdint.h>
#include "text_encoding.h"

/* GB18030 encoding:
 *          1st byte    2nd byte    3rd byte    4th byte
 *  1byte: 0x00~0x7F
 * 2bytes: 0x81~0xFE   0x40~0xFE
 * 4bytes: 0x81~0xFE   0x30~0x39   0x81~0xFE   0x30~0x39
 */
struct rtgui_char_position _string_char_width(char *str, size_t len, size_t offset)
{
    struct rtgui_char_position pos = {0, 0};
    unsigned char *pc;

    RT_ASSERT(offset < len);

    pc = (unsigned char*)str;

    while (pc <= (unsigned char*)str + offset)
    {
        if (pc[0] < 0x80)
        {
            pos.char_width = 1;
        }
        else if (0x81 <= pc[0] && pc[0] <= 0xFE)
        {
            if (0x40 <= pc[1] && pc[1] <= 0xFE)
            {
                /* GBK */
                pos.char_width = 2;
            }
            else if (0x30 <= pc[1] && pc[1] <= 0x39)
            {
                /* GB18030 */
                pos.char_width = 4;
            }
            else
            {
                /* FIXME: unknown encoding */
                RT_ASSERT(0);
                pos.char_width = 1;
            }
        }
        else
        {
            /* FIXME: unknown encoding */
            RT_ASSERT(0);
            pos.char_width = 1;
        }
        pc += pos.char_width;
    }
    pos.remain = pc - (unsigned char*)&str[offset];
    return pos;
}
