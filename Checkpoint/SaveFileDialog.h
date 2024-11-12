#ifndef SAVEFILEDIALOG_H
#define SAVEFILEDIALOG_H

#include <memory>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class SaveFileDialog;
}
QT_END_NAMESPACE

class SaveFileDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Save { SAVE, DONT_SAVE };

    SaveFileDialog(QWidget* parent = nullptr);
    ~SaveFileDialog();

    Save GetSave() const noexcept { return save; }

private:
    std::unique_ptr<Ui::SaveFileDialog> ui;

    Save save;
};

#endif // SAVEFILEDIALOG_H
