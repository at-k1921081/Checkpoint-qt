#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <memory>
#include <QMainWindow>
#include <QMessageBox>
#include "Goal.h"

struct Goal;

QT_BEGIN_NAMESPACE
namespace Ui {
class Checkpoint;
}
QT_END_NAMESPACE

class Checkpoint : public QMainWindow
{
    Q_OBJECT

public:
    Checkpoint(QWidget *parent = nullptr);
    ~Checkpoint();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    // Window UI
    std::unique_ptr<Ui::Checkpoint> ui;

    // Slots
    void OnFileNew();
    void OnFileOpen();
    void OnFileSave();
    void OnFileSaveAs();
    void OnFileExit();
    void OnHelpDocumentation();
    void OnHelpAbout();

    void OnChangeSelectedGoal();
    void OnSwitchGoalTab(int tab);
    void OnSearchboxChanged(const QString& text);

    void Quit();
    void Reset();
    QDialog::DialogCode OnReset();

    // Helper functions, some of which are used as slots
    void AddGoal();
    void RemoveGoal();
    void EditGoal();
    bool SaveGoalsAs();
    void SaveGoals(std::string_view filepath);

    // Helper functions
    void SetupGoalInfoWidgets();
    void SetUnsavedChanges(bool uc);

    QMessageBox::StandardButton ReportErrorDialog(const QString& title, const QString& error, QMessageBox::StandardButtons buttons);
    enum class SaveResult { SAVE, DONTSAVE, CANCEL };
    SaveResult ShowSaveFileDialog();

    //std::vector<Goal> goals;
    // App data
    GoalInfo goals;
    std::string current_filepath;
    bool unsaved_changes;
};

#endif // CHECKPOINT_H
