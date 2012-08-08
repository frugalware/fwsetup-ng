#include "master.h"

#define BEGINNING_TEXT \
"Welcome to the Frugalware Installer!\n" \
"\n" \
"Please click Next to continue.\n"

static enum order beginning_run(void)
{
	enum order rv = ORDER_NONE;
	YRichText *text = YUI::widgetFactory()->createRichText(master->contents,BEGINNING_TEXT,true);

	while(rv == ORDER_NONE)
	{
		YEvent *event = master->dialog->waitForEvent();

		if(event->eventType() == YEvent::WidgetEvent)
		{
			YWidgetEvent *wevent = (YWidgetEvent*) event;

			if(wevent->widget() == master->next && wevent->reason() == YEvent::Activated)
				rv = ORDER_NEXT;
		}

		master->dialog->deleteEvent(event);
	}

	return rv;
}

extern struct slave *beginning_initialize(void)
{
	struct slave *p;

	p = new struct slave;

	memset(p,0,sizeof(struct slave));

	p->next = 0;

	p->previous = 0;

	p->name = new std::string(__FILE__);

	p->run = beginning_run;

	return p;
}
