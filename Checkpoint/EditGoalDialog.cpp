#include "EditGoalDialog.h"
#include "ui_EditGoalDialog.h"
#include "QDoubleValidator"

EditGoalDialog::EditGoalDialog(QWidget* parent) :
    QDialog{ parent },
    ui{ std::make_unique<Ui::EditGoalDialog>() }
{
    ui->setupUi(this);

    ui->edit_progress->setValidator(new QDoubleValidator(this));
    ui->edit_goal->setValidator(new QDoubleValidator(this));

    connect(ui->button_edit, &QPushButton::clicked, this, [this] { SetupGoalInfo(); accept(); });
}

EditGoalDialog::~EditGoalDialog() = default;

void EditGoalDialog::SetupGoalInfo()
{
    goal.title = ui->edit_title->text().toStdString();
    goal.description = ui->edit_description->toPlainText().toStdString();
    goal.category = ui->edit_category->text().toStdString();
    goal.progress = ui->edit_progress->text().toDouble();
    goal.goal = ui->edit_goal->text().toDouble();
}

void EditGoalDialog::SetGoalInfo(const Goal& goal)
{
    this->goal = goal;

    ui->edit_title->setText(QString::fromStdString(goal.title));
    ui->edit_description->setText(QString::fromStdString(goal.description));
    ui->edit_category->setText(QString::fromStdString(goal.category));
    ui->edit_progress->setText(QString::number(goal.progress));
    ui->edit_goal->setText(QString::number(goal.goal));
}
