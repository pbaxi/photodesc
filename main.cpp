#include "photodesc.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    photoDesc w;
    w.show();

    return a.exec();
}
