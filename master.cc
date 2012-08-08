#include "master.h"

struct master *master;

extern int main(int argc,char **argv)
{
	master = new struct master;

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

	return 0;
}
