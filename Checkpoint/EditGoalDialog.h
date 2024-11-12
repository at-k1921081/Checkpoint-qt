#ifndef EDITGOALDIALOG_H
#define EDITGOALDIALOG_H

#include <memory>
#include "QDialog"
#include "Goal.h"

namespace Ui {
class EditGoalDialog;
}

class EditGoalDialog : public QDialog {
    Q_OBJECT

public:
    EditGoalDialog(QWidget* parent);
    ~EditGoalDialog();

    const Goal& GetGoal() const noexcept { return goal; };
    void SetGoalInfo(const Goal& goal);

private:
    std::unique_ptr<Ui::EditGoalDialog> ui;
    Goal goal;

    void SetupGoalInfo(); // Set goal info from widgets
};

#endif // EDITGOALDIALOG_H
