#include "Forest_Model.h"
#include "main/forest.hpp"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Forest_Model w;
    w.show();
    return a.exec();
}
