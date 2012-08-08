#pragma once

#include <YUI.h>
#include <YDialog.h>
#include <YLayoutBox.h>
#include <YSelectionBox.h>
#include <YComboBox.h>
#include <YButtonBox.h>
#include <YPushButton.h>
#include <YEvent.h>
#include <YWidgetFactory.h>
#include <YLabel.h>
#include <YInputField.h>
#include <YCheckBox.h>
#include <YRadioButton.h>
#include <YTree.h>
#include <YTable.h>
#include <YProgressBar.h>
#include <YRichText.h>
#include <YBusyIndicator.h>
#include <YIntField.h>
#include <YMenuButton.h>
#include <YImage.h>
#include <YLogView.h>
#include <YMultiLineEdit.h>
#include <YMultiSelectionBox.h>
#include <YWidget.h>
#include <YSpacing.h>
#include <YEmpty.h>
#include <YAlignment.h>
#include <YSquash.h>
#include <YFrame.h>
#include <YCheckBoxFrame.h>
#include <YRadioButtonGroup.h>
#include <YReplacePoint.h>
#include <cstdio>
#include <cstring>
#include <search.h>

enum order
{
	ORDER_NONE,
	ORDER_PREVIOUS,
	ORDER_NEXT
};

struct master
{
	YDialog *dialog;
	YLayoutBox *verticalbox;
	YReplacePoint *contents;
	YButtonBox *buttonbox;
	YPushButton *previous;
	YPushButton *next;
};

struct slave
{
	struct slave *next;
	struct slave *previous;
	std::string *name;
	enum order (*run) (void);
};

extern struct master *master;
extern struct slave *slaves;

extern struct slave *beginning_initialize(void);
