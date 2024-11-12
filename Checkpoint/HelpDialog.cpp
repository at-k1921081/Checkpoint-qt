#include "HelpDialog.h"
#include "ui_HelpDialog.h"

HelpDialog::HelpDialog(QWidget* parent) :
	QDialog{ parent },
	ui{ std::make_unique<Ui::HelpDialog>() }
{
	ui->setupUi(this);
}

HelpDialog::~HelpDialog() = default;