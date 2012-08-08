#include "master.h"

struct master *master;
struct slave *slaves;

extern int main(int argc,char **argv)
{
	struct slave *slave;

	stderr = freopen("setup.log","wb",stderr);

	if(stderr == 0)
		return 1;

	master = new struct master;

	memset(master,0,sizeof(struct master));

	master->dialog = YUI::widgetFactory()->createMainDialog();

	master->verticalbox = YUI::widgetFactory()->createVBox(master->dialog);

	master->contents = YUI::widgetFactory()->createReplacePoint(master->verticalbox);

	master->buttonbox = YUI::widgetFactory()->createButtonBox(master->verticalbox);

	master->previous = YUI::widgetFactory()->createPushButton(master->buttonbox,"Previous");

	master->previous->setRole(YCancelButton);

	master->next = YUI::widgetFactory()->createPushButton(master->buttonbox,"Next");

	master->next->setRole(YOKButton);

	master->next->setDefaultButton();

	master->dialog->open();

	master->dialog->setSize(640,480);

	slaves = beginning_initialize();

	insque(slaves,0);

	insque(ending_initialize(),slaves);

	for( slave = slaves ; slave ; )
	{
		enum order order;

		fprintf(stderr,"%s: about to clear children of contents widget\n",__FILE__);

		master->contents->deleteChildren();

		fprintf(stderr,"%s: about to run slave module '%s'\n",__FILE__,slave->name->c_str());

		order = slave->run();

		switch(order)
		{
			case ORDER_NEXT:
				slave = slave->next;
				break;

			case ORDER_PREVIOUS:
				slave = slave->previous;
				break;

			default:
				slave = 0;
				break;
		}
	}

	master->dialog->destroy();

	return 0;
}
