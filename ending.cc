#include "master.h"

#define ENDING_TEXT \
"Frugalware has been successfully installed!\n" \
"\n" \
"Please click Next to exit the installer.\n"

static enum order ending_run(void)
{
	enum order rv = ORDER_NONE;
	YRichText *text = YUI::widgetFactory()->createRichText(master->contents,ENDING_TEXT,true);

	while(rv == ORDER_NONE)
	{
		YEvent *event = master->dialog->waitForEvent();

		if(event->eventType() == YEvent::WidgetEvent)
		{
			YWidgetEvent *wevent = (YWidgetEvent*) event;

			if(wevent->widget() == master->previous && wevent->reason() == YEvent::Activated)
				rv = ORDER_PREVIOUS;
			else if(wevent->widget() == master->next && wevent->reason() == YEvent::Activated)
				rv = ORDER_NEXT;
		}

		master->dialog->deleteEvent(event);
	}

	return rv;
}

extern struct slave *ending_initialize(void)
{
	struct slave *p;

	p = new struct slave;

	memset(p,0,sizeof(struct slave));

	p->next = 0;

	p->previous = 0;

	p->name = new std::string(__FILE__);

	p->run = ending_run;

	return p;
}
