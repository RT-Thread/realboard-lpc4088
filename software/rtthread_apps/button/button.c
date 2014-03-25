#include <rtthread.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/widgets/container.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/button.h>

void _on_close(struct rtgui_object *object, struct rtgui_event *event)
{
    struct rtgui_win* win;

    win = RTGUI_WIDGET(object)->toplevel;
	rtgui_win_close(win);
}

int main(int argc, char** argv)
{
	struct rtgui_app* application;
	struct rtgui_win* win;
    struct rtgui_button *button;

	application = rtgui_app_create("button");
	if (application != RT_NULL)
	{
		rtgui_rect_t rect;

		win = rtgui_mainwin_create(RT_NULL, "button",
			RTGUI_WIN_STYLE_MAINWIN | RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);

		/* create lable in app window */
        button = rtgui_button_create("close");
		rtgui_button_set_onbutton(button, _on_close);
        rtgui_rect_init(&rect, 150, 240, 80, 25);
		rtgui_widget_set_rect(RTGUI_WIDGET(button), &rect);
		rtgui_container_add_child(RTGUI_CONTAINER(win), RTGUI_WIDGET(button));

		rtgui_win_show(win, RT_TRUE);
		rtgui_app_destroy(application);
	}

    return 0;
}

