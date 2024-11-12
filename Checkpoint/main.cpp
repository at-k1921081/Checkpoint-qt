#include "Checkpoint.h"

#include <QApplication>
#include "Checkpoint.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Checkpoint w;
    w.show();
    return a.exec();
}
