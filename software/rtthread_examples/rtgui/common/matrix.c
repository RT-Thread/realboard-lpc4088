#include <rtgui/rtgui.h>
#include <rtgui/matrix.h>

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

/* Port from ejoy2d: https://github.com/cloudwu/ejoy2d/blob/master/LICENSE
 * Original License:
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Ejoy.com Inc.
 *
 * Permission is hereby granted,  free of charge,  to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),  to deal in
 * the Software without restriction,  including without limitation the rights to
 * use,  copy,  modify,  merge,  publish,  distribute,  sublicense,  and/or sell copies of
 * the Software,  and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY,  WHETHER
 * IN AN ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING FROM,  OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Port to RTGUI by Grissiom */

rt_inline int _inverse_scale(const int *m , int *o)
{
    if (m[0] == 0 || m[3] == 0)
        return 1;

    o[0] = (1024 * 1024) / m[0];
    o[1] = 0;
    o[2] = 0;
    o[3] = (1024 * 1024) / m[3];
    o[4] = - (m[4] * o[0]) / 1024;
    o[5] = - (m[5] * o[3]) / 1024;

    return 0;
}

rt_inline int _inverse_rot(const int *m, int *o)
{
    if (m[1] == 0 || m[2] == 0)
        return 1;

    o[0] = 0;
    o[1] = (1024 * 1024) / m[2];
    o[2] = (1024 * 1024) / m[1];
    o[3] = 0;
    o[4] = - (m[5] * o[2]) / 1024;
    o[5] = - (m[4] * o[1]) / 1024;

    return 0;
}

int rtgui_matrix_inverse(const struct rtgui_matrix *mm, struct rtgui_matrix *mo)
{
    const int *m = mm->m;
    int *o = mo->m;
    int t;

    if (m[1] == 0 && m[2] == 0)
    {
        return _inverse_scale(m,o);
    }
    if (m[0] == 0 && m[3] == 0)
    {
        return _inverse_rot(m,o);
    }

    t = m[0] * m[3] - m[1] * m[2];
    if (t == 0)
        return 1;

    o[0] = (int32_t)((int64_t)m[3] * (1024 * 1024) / t);
    o[1] = (int32_t)(- (int64_t)m[1] * (1024 * 1024) / t);
    o[2] = (int32_t)(- (int64_t)m[2] * (1024 * 1024) / t);
    o[3] = (int32_t)((int64_t)m[0] * (1024 * 1024) / t);
    o[4] = - (m[4] * o[0] + m[5] * o[2]) / 1024;
    o[5] = - (m[4] * o[1] + m[5] * o[3]) / 1024;

    return 0;
}

/* @dd is the degree range in 0~256 */
rt_inline int icost(int dd)
{
	static const int t[256] = {
		1024,1023,1022,1021,1019,1016,1012,1008,1004,999,993,986,979,972,964,955,
		946,936,925,914,903,890,878,865,851,837,822,807,791,775,758,741,
		724,706,687,668,649,629,609,589,568,547,526,504,482,460,437,414,
		391,368,344,321,297,273,248,224,199,175,150,125,100,75,50,25,
		0,-25,-50,-75,-100,-125,-150,-175,-199,-224,-248,-273,-297,-321,-344,-368,
		-391,-414,-437,-460,-482,-504,-526,-547,-568,-589,-609,-629,-649,-668,-687,-706,
		-724,-741,-758,-775,-791,-807,-822,-837,-851,-865,-878,-890,-903,-914,-925,-936,
		-946,-955,-964,-972,-979,-986,-993,-999,-1004,-1008,-1012,-1016,-1019,-1021,-1022,-1023,
		-1024,-1023,-1022,-1021,-1019,-1016,-1012,-1008,-1004,-999,-993,-986,-979,-972,-964,-955,
		-946,-936,-925,-914,-903,-890,-878,-865,-851,-837,-822,-807,-791,-775,-758,-741,
		-724,-706,-687,-668,-649,-629,-609,-589,-568,-547,-526,-504,-482,-460,-437,-414,
		-391,-368,-344,-321,-297,-273,-248,-224,-199,-175,-150,-125,-100,-75,-50,-25,
		0,25,50,75,100,125,150,175,199,224,248,273,297,321,344,368,
		391,414,437,460,482,504,526,547,568,589,609,629,649,668,687,706,
		724,741,758,775,791,807,822,837,851,865,878,890,903,914,925,936,
		946,955,964,972,979,986,993,999,1004,1008,1012,1016,1019,1021,1022,1023,
	};
	if (dd < 0) {
		dd = 256 - (-dd % 256);
	} else {
		dd %= 256;
	}

	return t[dd];
}

rt_inline int icosd(int d)
{
	int dd = d/4;
	return icost(dd);
}

rt_inline int isind(int d)
{
	int dd = 64 - d/4;
	return icost(dd);
}

rt_inline void rot_mat(int *m, int d)
{
	int cosd = icosd(d);
	int sind = isind(d);

	int m0_cosd = m[0] * cosd;
	int m0_sind = m[0] * sind;
	int m1_cosd = m[1] * cosd;
	int m1_sind = m[1] * sind;
	int m2_cosd = m[2] * cosd;
	int m2_sind = m[2] * sind;
	int m3_cosd = m[3] * cosd;
	int m3_sind = m[3] * sind;
	int m4_cosd = m[4] * cosd;
	int m4_sind = m[4] * sind;
	int m5_cosd = m[5] * cosd;
	int m5_sind = m[5] * sind;

	m[0] = (m0_cosd - m1_sind) /1024;
	m[1] = (m0_sind + m1_cosd) /1024;
	m[2] = (m2_cosd - m3_sind) /1024;
	m[3] = (m2_sind + m3_cosd) /1024;
	m[4] = (m4_cosd - m5_sind) /1024;
	m[5] = (m4_sind + m5_cosd) /1024;
}

rt_inline void scale_mat(int *m, int sx, int sy)
{
	if (sx != 1024) {
		m[0] = m[0] * sx / 1024;
		m[2] = m[2] * sx / 1024;
		m[4] = m[4] * sx / 1024;
	}
	if (sy != 1024) {
		m[1] = m[1] * sy / 1024;
		m[3] = m[3] * sy / 1024;
		m[5] = m[5] * sy / 1024;
	}
}

void rtgui_matrix_rotate(struct rtgui_matrix *m, int rot)
{
    if (rot)
        rot_mat(m->m, rot);
}

void rtgui_matrix_scale(struct rtgui_matrix *m, int sx, int sy)
{
	scale_mat(m->m, sx, sy);
}

void rtgui_matrix_move(struct rtgui_matrix *m, int dx, int dy)
{
	m->m[4] += dx;
	m->m[5] += dy;
}

