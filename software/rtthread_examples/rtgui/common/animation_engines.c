#include <rtgui/rtgui_system.h>
#include <rtgui/dc.h>
#include <rtgui/blit.h>
#include <rtgui/animation.h>
#include <rtgui/dc_trans.h>

void rtgui_anim_engine_move(struct rtgui_dc *background,
                            struct rtgui_dc *background_buffer,
                            struct rtgui_dc *items,
                            int item_cnt,
                            int progress,
                            void *param)
{
    int cx, cy, w, h;
    struct rtgui_anim_engine_move_ctx *ctx = param;
    struct rtgui_rect dc_rect;

    if (!background)
        return;

    if (background_buffer)
        rtgui_dc_blit((struct rtgui_dc*)background_buffer,
                      NULL, background, NULL);

	if (!items)
		return;

    rtgui_dc_get_rect(items, &dc_rect);

    cx = progress * (ctx->end.x - ctx->start.x) / RTGUI_ANIM_TICK_RANGE;
    cy = progress * (ctx->end.y - ctx->start.y) / RTGUI_ANIM_TICK_RANGE;
    w = rtgui_rect_width(dc_rect);
    h = rtgui_rect_height(dc_rect);

    dc_rect.x1 = cx + ctx->start.x;
    dc_rect.y1 = cy + ctx->start.y;
    dc_rect.x2 = dc_rect.x1 + w;
    dc_rect.y2 = dc_rect.y1 + h;

    /* To prevent overlapping, only one item can be drawn by
     * rtgui_anim_engine_move. */
    if (items)
        rtgui_dc_blit((struct rtgui_dc*)(items), NULL, background, &dc_rect);
}
RTM_EXPORT(rtgui_anim_engine_move);

void rtgui_anim_engine_fade(struct rtgui_dc *background,
                            struct rtgui_dc *background_buffer,
                            struct rtgui_dc *items,
                            int item_cnt,
                            int progress,
                            void *param)
{
    int cur_lvl;
    struct rtgui_blit_info info;
    struct rtgui_dc_buffer *buf, *buf_items;
    struct rtgui_anim_engine_fade_ctx *ctx = param;

    if (!background_buffer || !items)
        return;

    RT_ASSERT(background);

    /* NOTE: the underlaying dc only support 5bits(32 levels) alpha value. */
    cur_lvl = progress * 255 / RTGUI_ANIM_TICK_RANGE;
    if (ctx->is_fade_out)
        cur_lvl = 255 - cur_lvl;
    /* Only 5bits of alpha is effective. But always update the dc when alpha is 0 or
     * 255. */
    if ((cur_lvl >> 3) == (ctx->plvl >> 3))
    {
        if (cur_lvl == 255 || cur_lvl == 0)
        {
            if (cur_lvl == ctx->plvl)
                return;
        }
        else
        {
            return;
        }
    }
    ctx->plvl = cur_lvl;

    buf = (struct rtgui_dc_buffer*)rtgui_dc_buffer_create_from_dc(background_buffer);
    if (!buf) return;

    info.a = cur_lvl;

	buf_items	   = (struct rtgui_dc_buffer*) items;
    info.src       = buf_items->pixel;
    info.src_fmt   = buf_items->pixel_format;
    info.src_h     = buf_items->height;
    info.src_w     = buf_items->width;
    info.src_pitch = buf_items->pitch;
    info.src_skip  = info.src_pitch - info.src_w *
                     rtgui_color_get_bpp(buf_items->pixel_format);

    info.dst       = buf->pixel;
    info.dst_fmt   = buf->pixel_format;
    info.dst_h     = buf->height;
    info.dst_w     = buf->width;
    info.dst_pitch = buf->pitch;
    info.dst_skip  = info.dst_pitch - info.dst_w *
                     rtgui_color_get_bpp(buf->pixel_format);

    rtgui_blit(&info);
    rtgui_dc_blit((struct rtgui_dc*)buf, NULL, background, NULL);
    rtgui_dc_destory((struct rtgui_dc*)buf);
}
RTM_EXPORT(rtgui_anim_engine_fade);

void rtgui_anim_engine_roto(struct rtgui_dc *background,
                            struct rtgui_dc *background_buffer,
                            struct rtgui_dc *items,
                            int item_cnt,
                            int progress,
                            void *param)
{
    struct rtgui_dc_trans *trans;
    struct rtgui_anim_engine_roto_ctx *ctx = param;

    if (!background)
        return;

    if (background_buffer)
        rtgui_dc_blit((struct rtgui_dc*)background_buffer, NULL, background, NULL);

    if (!param)
        return;

    trans = rtgui_dc_trans_create((struct rtgui_dc*)items);
    if (!trans)
    {
        rt_kprintf("OOM\n");
        return;
    }

    rtgui_dc_trans_move(trans, ctx->pre_move.x, ctx->pre_move.y);
    rtgui_dc_trans_rotate(trans,
                          progress * (ctx->to_degree - ctx->from_degree)
                          / RTGUI_ANIM_TICK_RANGE + ctx->from_degree);
    rtgui_dc_trans_move(trans, ctx->post_move.x, ctx->post_move.y);
    rtgui_dc_trans_set_aa(trans, ctx->use_aa);

    rtgui_dc_trans_blit(trans, RT_NULL, background, RT_NULL);

    rtgui_dc_trans_destroy(trans);
}
RTM_EXPORT(rtgui_anim_engine_roto);

