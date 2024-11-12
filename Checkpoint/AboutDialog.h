#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "QDialog"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog {
public:
	AboutDialog(QWidget* parent);
	~AboutDialog();

private:
	std::unique_ptr<Ui::AboutDialog> ui;
};

#endif