#include <glib.h>

struct config
{
	gint s_width;  // Screen Width
	gint s_height; // Screen Height
	gint w_width;  // Window Width
	gint w_height; // Window Height
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

	config->w_width = config->s_width;

	config->w_height = config->s_height;

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

	newt_end();

	return 0;
}
