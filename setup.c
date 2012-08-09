#include <glib.h>

struct config
{
	gint width;
	gint height;
};

static struct config *config;

#ifdef NEWT
#include <newt.h>

static gboolean newt_begin(void)
{
	if(newtInit() != 0)
		return FALSE;

	newtCls();

	newtGetScreenSize(&config->width,&config->height);

	return TRUE;
}

static void newt_end(void)
{
	newtFinished();
}
#endif

extern int main(void)
{
	config = g_new0(struct config,1);

	return 0;
}
