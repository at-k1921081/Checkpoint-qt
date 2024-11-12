#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <memory>
#include "QDialog"

namespace Ui {
class HelpDialog;
}

class HelpDialog : public QDialog {
public:
	HelpDialog(QWidget* parent);
	~HelpDialog();

private:
	std::unique_ptr<Ui::HelpDialog> ui;
};

#endif