#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget* parent) :
	QDialog{ parent },
	ui{ std::make_unique<Ui::AboutDialog>() }
{
	ui->setupUi(this);
}

AboutDialog::~AboutDialog() = default;