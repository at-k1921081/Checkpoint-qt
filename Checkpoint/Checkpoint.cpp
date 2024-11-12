#include "Checkpoint.h"
#include "./ui_Checkpoint.h"
#include "QDialog"
#include "QMessageBox"
#include "QLabel"
#include "QFileDialog"
#include "QErrorMessage"
#include "QString"
#include "QStringListModel"
#include "QStandardItemModel"
#include "QCloseEvent"

#include "EditGoalDialog.h"
#include "SaveFileDialog.h"
#include "HelpDialog.h"
#include "AboutDialog.h"
#include "Goal.h"

static constexpr std::string_view TITLE = "Checkpoint";
static constexpr int PAGE_CURRENT_GOALS = 0;
static constexpr int PAGE_COMPLETED_GOALS = 1;
static constexpr int SEARCH_BY_NAME = 0;
static constexpr int SEARCH_BY_CATEGORY = 1;

namespace {
struct SortFunc {
    constexpr bool operator()(const Goal& g1, const Goal& g2)
    {
        return g1.title < g2.title;
    }
};
}

static void SearchByName(QListWidget* lw, QString text)
{
    for (int i = 0; i < lw->count(); ++i) {
        if (lw->item(i)->text().contains(text, Qt::CaseInsensitive)) {
            lw->item(i)->setHidden(false);
        }
        else {
            lw->item(i)->setHidden(true);
        }
    }
}

static void SearchByCategory(QListWidget* lw, std::string_view text, std::span<const Goal> goals)
{
    assert(static_cast<size_t>(lw->count()) == goals.size());

    for (int i = 0; i < lw->count(); ++i) {
        if (goals[i].category.contains(text))
            lw->item(i)->setHidden(false);
        else
            lw->item(i)->setHidden(true);
    }
}

Checkpoint::Checkpoint(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::Checkpoint>()),
    unsaved_changes{ false }
{
    ui->setupUi(this);

    connect(ui->actionNew, &QAction::triggered, this, &Checkpoint::OnFileNew);
    connect(ui->actionOpen, &QAction::triggered, this, &Checkpoint::OnFileOpen);
    connect(ui->actionSave, &QAction::triggered, this, &Checkpoint::OnFileSave);
    connect(ui->actionSave_As, &QAction::triggered, this, &Checkpoint::OnFileSaveAs);
    connect(ui->actionExit, &QAction::triggered, this, &Checkpoint::OnFileExit);
    connect(ui->actionAdd_Goal, &QAction::triggered, this, &Checkpoint::AddGoal);
    connect(ui->actionEdit_Goal, &QAction::triggered, this, &Checkpoint::EditGoal);
    connect(ui->actionRemove_Goal, &QAction::triggered, this, &Checkpoint::RemoveGoal);
    connect(ui->actionDocumentation, &QAction::triggered, this, &Checkpoint::OnHelpDocumentation);
    connect(ui->actionAbout, &QAction::triggered, this, &Checkpoint::OnHelpAbout);

    connect(ui->add_goal_button, &QPushButton::clicked, this, &Checkpoint::AddGoal);
    connect(ui->edit_goal_button, &QPushButton::clicked, this, &Checkpoint::EditGoal);
    connect(ui->remove_goal_button, &QPushButton::clicked, this, &Checkpoint::RemoveGoal);

    connect(ui->goals_list, &QListWidget::itemSelectionChanged, this, &Checkpoint::OnChangeSelectedGoal);
    connect(ui->completed_goals_list, &QListWidget::itemSelectionChanged, this, &Checkpoint::OnChangeSelectedGoal);

    connect(ui->goal_tabs, &QTabWidget::currentChanged, this, &Checkpoint::OnSwitchGoalTab);
    
    connect(ui->search_edit, &QLineEdit::textChanged, this, &Checkpoint::OnSearchboxChanged);

    ui->goal_tabs->setCurrentIndex(0);
    ui->search_by_cb->addItem("Name");
    ui->search_by_cb->addItem("Category");
    ui->search_by_cb->setCurrentIndex(0);

    ui->actionExit->setShortcut(Qt::ALT | Qt::Key_F4);
}

Checkpoint::~Checkpoint() = default;

void Checkpoint::closeEvent(QCloseEvent* event)
{
    if (OnReset() == QDialog::Accepted)
        return event->accept();
    else
        return event->ignore();
}

void Checkpoint::OnFileNew()
{
    if (OnReset() == QDialog::Rejected) return;

    goals.current.clear();
    goals.completed.clear();
    current_filepath.clear();
    unsaved_changes = false;
    ui->goals_list->reset();
    ui->label_title->setText("Goal Title");
    ui->label_description->setText("Description");
    ui->label_progress->setText("Progress");
    ui->label_goal->setText("Progress Value");

    setWindowTitle(QString::fromUtf8(TITLE) + " - Untitled");
}

void Checkpoint::OnFileOpen()
{
    if (unsaved_changes) {
        const auto result = ShowSaveFileDialog();
        if (result == SaveResult::CANCEL) return;
    }

    std::string filename = QFileDialog::getOpenFileName(this, "Open File").toStdString();
    if (filename.empty()) return;

    ui->goals_list->clear();
    ui->completed_goals_list->clear();
    goals.current.clear();
    goals.completed.clear();

    auto loaded_goals = LoadGoals(filename);
    if (loaded_goals.has_value()) {
        goals = std::move(*loaded_goals);
        current_filepath = std::move(filename);
        setWindowTitle(QString::fromUtf8(TITLE) + " - " + QString::fromStdString(current_filepath));

        std::sort(goals.current.begin(), goals.current.end(), SortFunc());
        std::sort(goals.completed.begin(), goals.completed.end(), SortFunc());

        for (const auto& g : goals.current) ui->goals_list->addItem(QString::fromStdString(g.title));
        for (const auto& g : goals.completed) ui->completed_goals_list->addItem(QString::fromStdString(g.title));

        SetupGoalInfoWidgets();
        SetUnsavedChanges(false);
    }
    else {
        ReportErrorDialog("Error", QString("Failed to load goals. Error: ").append(loaded_goals.error()), QMessageBox::Ok);
    }
}

void Checkpoint::OnFileSave()
{
    if (current_filepath.empty())
        OnFileSaveAs();
    else
        SaveGoals(current_filepath);
}

void Checkpoint::OnFileSaveAs()
{
    SaveGoalsAs();
}

void Checkpoint::OnFileExit()
{
    Quit();
}

void Checkpoint::OnHelpDocumentation()
{
    HelpDialog dialog(this);
    dialog.exec();
}

void Checkpoint::OnHelpAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void Checkpoint::OnChangeSelectedGoal()
{
    const auto page = ui->goal_tabs->currentIndex();
    const auto selected_item = page == 0 ? ui->goals_list->currentItem() : ui->completed_goals_list->currentItem();
    if (!selected_item) return;

    const auto row = page == PAGE_CURRENT_GOALS ? ui->goals_list->currentRow() : ui->completed_goals_list->currentRow();
    if (row >= (page == PAGE_CURRENT_GOALS ? goals.current.size() : goals.completed.size()) || row == -1) return;
    const Goal& selected = page == PAGE_CURRENT_GOALS ? goals.current[row] : goals.completed[row];

    ui->label_title->setText(QString::fromStdString(selected.title));
    ui->label_category->setText(QString::fromStdString(selected.category));
    ui->label_description->setText(QString::fromStdString(selected.description));
    ui->label_progress->setText(QString::number(selected.progress));
    ui->label_goal->setText(QString::number(selected.goal));
}

void Checkpoint::OnSwitchGoalTab(int tab)
{
    if (tab == PAGE_CURRENT_GOALS)
        ui->goals_list->setCurrentRow(-1);
    else
        ui->completed_goals_list->setCurrentRow(-1);
}

void Checkpoint::OnSearchboxChanged(const QString& text)
{
    const auto page = ui->goal_tabs->currentIndex();

    QListWidget* list = page == PAGE_CURRENT_GOALS ? ui->goals_list : ui->completed_goals_list;
    std::span<const Goal> goalsarr = page == PAGE_CURRENT_GOALS ? goals.current : goals.completed;
    
    switch (ui->search_by_cb->currentIndex()) {
    case SEARCH_BY_NAME: SearchByName(list, text); break;
    case SEARCH_BY_CATEGORY: SearchByCategory(list, text.toStdString(), goalsarr); break;
    }
}

void Checkpoint::Quit()
{
    close();
}

void Checkpoint::Reset()
{
    ui->goals_list->clear();
    ui->label_title->setText("Goal Title");
    ui->label_description->setText("Description");
    ui->label_progress->setText("Progress");
    ui->label_goal->setText("Progress Value");

    goals.current.clear();
    goals.completed.clear();
    current_filepath.clear();
    unsaved_changes = false;
}

QDialog::DialogCode Checkpoint::OnReset()
{
    if (unsaved_changes) {
        const auto result = ShowSaveFileDialog();

        if (result == SaveResult::CANCEL) return QDialog::Rejected;
    }
    
    Reset();

    return QDialog::Accepted;
}

void Checkpoint::AddGoal()
{
    goals.current.push_back(Goal{ "New Goal", "This is a new goal", "", 0.0, 0.0 });
    ui->goals_list->addItem("New Goal");

    std::sort(goals.current.begin(), goals.current.end(), SortFunc());
    ui->goals_list->sortItems();

    SetUnsavedChanges(true);
}

void Checkpoint::RemoveGoal()
{
    const auto page = ui->goal_tabs->currentIndex();
    const auto index = page == 0 ? ui->goals_list->currentRow() : ui->completed_goals_list->currentRow();
    if (index == -1) return;

    if (page == 0) {
        goals.current.erase(goals.current.begin() + index);
        ui->goals_list->takeItem(index);
    }
    else {
        goals.completed.erase(goals.completed.begin() + index);
        ui->completed_goals_list->takeItem(index);
    }

    SetUnsavedChanges(true);
}

void Checkpoint::EditGoal()
{
    const auto page = ui->goal_tabs->currentIndex();
    const auto selected_row = page == 0 ? ui->goals_list->currentRow() : ui->completed_goals_list->currentRow();
    if (selected_row == -1) return;
    Goal& goal = page == 0 ? goals.current[selected_row] : goals.completed[selected_row];

    EditGoalDialog dialog(this);
    dialog.SetGoalInfo(goal);
    if (dialog.exec() == QDialog::Accepted) {
        goal = dialog.GetGoal();

        SetupGoalInfoWidgets();

        if (goal.progress >= goal.goal && page == 0 && goal.goal != 0.0) {
            goals.completed.push_back(goal);
            goals.current.erase(goals.current.begin() + selected_row);
            ui->completed_goals_list->addItem(QString::fromStdString(goals.completed.back().title));
            ui->goals_list->takeItem(selected_row);
        }

        if (goal.progress < goal.goal && page == PAGE_COMPLETED_GOALS) {
            goals.current.push_back(goal);
            goals.completed.erase(goals.completed.begin() + selected_row);
            ui->completed_goals_list->takeItem(selected_row);
            ui->goals_list->addItem(QString::fromStdString(goals.current.back().title));
        }

        std::sort(goals.current.begin(), goals.current.end(), SortFunc());
        ui->goals_list->sortItems();
        std::sort(goals.completed.begin(), goals.completed.end(), SortFunc());
        ui->completed_goals_list->sortItems();

        SetUnsavedChanges(true);
    }
}

bool Checkpoint::SaveGoalsAs()
{
    QString newfile = QFileDialog::getSaveFileName(this);
    if (newfile.isEmpty()) return false;

    current_filepath = newfile.toStdString();

    SaveGoals(current_filepath);

    return true;
}

void Checkpoint::SaveGoals(std::string_view filepath)
{
    assert(!current_filepath.empty());

    if (!WriteGoals(goals, filepath))
        ReportErrorDialog("Error Saving File", QString("Failed to save file goals to file ").append(filepath), QMessageBox::StandardButton::Ok);
    else
        SetUnsavedChanges(false);
}

void Checkpoint::SetupGoalInfoWidgets()
{
    const auto page = ui->goal_tabs->currentIndex();
    auto row = page == 0 ? ui->goals_list->currentRow() : ui->completed_goals_list->currentRow();
    if (row == -1) return;

    const Goal& goal = page == 0 ? goals.current[row] : goals.completed[row];

    if (page == PAGE_CURRENT_GOALS)
        ui->goals_list->currentItem()->setText(QString::fromStdString(goal.title));
    else
        ui->completed_goals_list->currentItem()->setText(QString::fromStdString(goal.title));

    ui->label_title->setText(QString::fromStdString(goal.title));
    ui->label_category->setText(QString::fromStdString(goal.category));
    ui->label_description->setText(QString::fromStdString(goal.description));
    ui->label_progress->setText(QString::number(goal.progress));
    ui->label_goal->setText(QString::number(goal.goal));
}

void Checkpoint::SetUnsavedChanges(bool uc)
{
    unsaved_changes = uc;
    QString title = QString::fromUtf8(TITLE);
    if (!current_filepath.empty())
        title.append(" - ").append(current_filepath);
    if (unsaved_changes)
        title.append('*');

    setWindowTitle(title);
}

QMessageBox::StandardButton Checkpoint::ReportErrorDialog(const QString& title, const QString& error, QMessageBox::StandardButtons buttons)
{
    return QMessageBox::warning(this, title, error, buttons);
}

Checkpoint::SaveResult Checkpoint::ShowSaveFileDialog()
{
    // Ask the user if they want to save changes before resetting
    SaveFileDialog dialog(this);

    // User cancelled
    if (dialog.exec() == QDialog::Rejected)
        return SaveResult::CANCEL;

    const auto save = dialog.GetSave();

    // If the user chose to save, save
    if (save == SaveFileDialog::Save::SAVE) {
        if (unsaved_changes) {
            if (current_filepath.empty()) {
                // If the user cancels saving goals, don't reset yet
                if (!SaveGoalsAs())
                    return SaveResult::CANCEL;
            }
            else {
                SaveGoals(current_filepath);
            }
        }

        return SaveResult::SAVE;
    }
    // Otherwise, discard unsaved changes

    return SaveResult::DONTSAVE;
}