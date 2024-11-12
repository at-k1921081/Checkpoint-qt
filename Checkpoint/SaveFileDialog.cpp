#include "SaveFileDialog.h"
#include "ui_SaveFileDialog.h"

SaveFileDialog::SaveFileDialog(QWidget* parent) :
    QDialog{ parent },
    ui{ std::make_unique<Ui::SaveFileDialog>() },
    save{ Save::DONT_SAVE }
{
    ui->setupUi(this);

    connect(ui->button_save, &QPushButton::clicked, this, [this] { save = Save::SAVE; accept(); });
    connect(ui->button_dontsave, &QPushButton::clicked, this, [this] { save = Save::DONT_SAVE; accept(); });
    connect(ui->button_cancel, &QPushButton::clicked, this, [this] { reject(); });
}

SaveFileDialog::~SaveFileDialog() = default;
