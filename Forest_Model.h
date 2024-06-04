#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLabel>
#include "ui_Forest_Model.h"

class Forest_Model : public QWidget
{
    Q_OBJECT

public:
    Forest_Model();
    ~Forest_Model();
    void createWidgets();
    void arrangeWidgets();
    void makeConnections();

    int treeCols;
    int treeRows;
    int treeType;
    float branchlength;
    float radius;
    int n;

    QLabel* treeColsLabel;
    QSpinBox* treeColsWidget;
    QLabel* treeRowsLabel;
    QSpinBox* treeRowsWidget;
    QLabel* treeTypeLabel;
    QSpinBox* treeTypeWidget;
    QLabel* branchLabel;
    QDoubleSpinBox* branchlengthWidget;
    QLabel* radiusLabel;
    QDoubleSpinBox* radiusWidget;
    QLabel* fractalLabel;
    QSpinBox* fractalWidget;
    QPushButton* drawButton;

private:
    Ui::Forest_ModelClass ui;

public slots:
    void drawForest();
};
