#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <qdom.h>
#include <qdragobject.h>
#include <qtoolbutton.h>

#include "action.h"

class ToolBar;

class ToolButton : public QToolButton
{
	Q_OBJECT

	private:
		QString ActionName;
		bool InOnState;
		QIconSet OffIcon;
		QString OffText;
		QIconSet OnIcon;
		QString OnText;

		bool IsEnabled;
		Action::ActionType Type;

	private slots:
		void buttonClicked();
		void deleteButtonClicked();
		void showTextLabelClicked();
		void toolbarPlaceChanged(QDockWindow::Place p);

	protected:
		void mouseMoveEvent(QMouseEvent* e);
		void contextMenuEvent(QContextMenuEvent* e);

	public:
		ToolButton(QWidget* parent, const QString& action_name, Action::ActionType actionType);
		~ToolButton();

		void setEnabled(bool enabled);

		/**
			button works just like toggled but using two shapes
			(pictures and texts)
		**/
		void setOnShape(const QIconSet& icon, const QString& text);
		bool isOn() const;
		void setOn(bool on);
		QString actionName();
		ToolBar* toolbar();
		void writeToConfig(QDomElement parent_element);
};

class ToolButtonDrag : public QTextDrag
{
	private:
		ToolButton* Button;

	public:
		ToolButtonDrag(ToolButton* button, QWidget* dragSource = 0, const char* name = 0);
};

#endif
