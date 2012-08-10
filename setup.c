#include <glib.h>

struct config
{
	gint s_width;  // Screen Width
	gint s_height; // Screen Height
	gint w_width;  // Window Width
	gint w_height; // Window Height
	gint w_left;   // Window offset from left edge
	gint w_top;    // Window offset from top edge
};

static struct config *config;

#ifdef NEWT
#include <newt.h>

static gboolean newt_begin(void)
{
	if(newtInit() != 0)
		return FALSE;

	newtCls();

	newtGetScreenSize(&config->s_width,&config->s_height);

	if(config->s_width < 80 || config->s_height < 24)
	{
		newtFinished();

		return FALSE;
	}

	config->w_width = (gint) ((gdouble) (config->s_width-2) * 0.90);

	config->w_height = (gint) ((gdouble) (config->s_height-2) * 0.90);

	config->w_left = (config->s_width - config->w_width) / 2;

	config->w_top = (config->s_height - config->w_height) / 2;

	return TRUE;
}

static void newt_end(void)
{
	newtFinished();
}
#endif

extern gint main(void)
{
	config = g_new0(struct config,1);

	newt_begin();

	newtOpenWindow(config->w_left,config->w_right,config->w_width,config->w_height,"Frugalware Linux Installer");

	newtRefresh();

	sleep(2);

	newt_end();

	return 0;
}
