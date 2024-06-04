#include "Forest_Model.h"
#include "main/forest.hpp"

Forest_Model::Forest_Model()
{
    //ui.setupUi(this);
    createWidgets();
    arrangeWidgets();
    makeConnections();
}

Forest_Model::~Forest_Model()
{}

void Forest_Model::createWidgets() {
    drawButton = new QPushButton("draw");

    treeColsLabel = new QLabel();
    treeColsLabel->setText("Tree Columns: ");
    treeColsWidget = new QSpinBox();
    treeColsWidget->setMinimum(1);
    treeColsWidget->setMaximum(20);

    treeRowsLabel = new QLabel();
    treeRowsLabel->setText("Tree Rows: ");
    treeRowsWidget = new QSpinBox();
    treeRowsWidget->setMinimum(1);
    treeRowsWidget->setMaximum(20);

    treeTypeLabel = new QLabel();
    treeTypeLabel->setText("Tree Type: ");
    treeTypeWidget = new QSpinBox();
    treeTypeWidget->setMinimum(1);
    treeTypeWidget->setMaximum(3);

    branchLabel = new QLabel();
    branchLabel->setText("Initial Branch Length: ");
    branchlengthWidget = new QDoubleSpinBox();
    branchlengthWidget->setMinimum(0.5);
    branchlengthWidget->setMaximum(20);
    branchlengthWidget->setDecimals(1);
    branchlengthWidget->setSingleStep(0.5);

    radiusLabel = new QLabel();
    radiusLabel->setText("Initial Radius length: ");
    radiusWidget = new QDoubleSpinBox();
    radiusWidget->setMinimum(0.1);
    radiusWidget->setMaximum(5);
    radiusWidget->setDecimals(1);
    radiusWidget->setSingleStep(0.1);

    fractalLabel = new QLabel();
    fractalLabel->setText("Fractals: ");
    fractalWidget = new QSpinBox();
    fractalWidget->setMinimum(1);
    fractalWidget->setMaximum(5);
}

void Forest_Model::arrangeWidgets() {
    QVBoxLayout* layout = new QVBoxLayout();

    QHBoxLayout* colLayout = new QHBoxLayout();
    colLayout->addWidget(treeColsLabel);
    colLayout->addWidget(treeColsWidget);

    QHBoxLayout* rowLayout = new QHBoxLayout();
    rowLayout->addWidget(treeRowsLabel);
    rowLayout->addWidget(treeRowsWidget);

    QHBoxLayout* typeLayout = new QHBoxLayout();
    typeLayout->addWidget(treeTypeLabel);
    typeLayout->addWidget(treeTypeWidget);

    QHBoxLayout* branchLayout = new QHBoxLayout();
    branchLayout->addWidget(branchLabel);
    branchLayout->addWidget(branchlengthWidget);

    QHBoxLayout* radiusLayout = new QHBoxLayout();
    radiusLayout->addWidget(radiusLabel);
    radiusLayout->addWidget(radiusWidget);

    QHBoxLayout* fractalLayout = new QHBoxLayout();
    fractalLayout->addWidget(fractalLabel);
    fractalLayout->addWidget(fractalWidget);

    layout->addLayout(colLayout);
    layout->addLayout(rowLayout);
    layout->addLayout(typeLayout);
    layout->addLayout(branchLayout);
    layout->addLayout(radiusLayout);
    layout->addLayout(fractalLayout);
    layout->addWidget(drawButton);
    setLayout(layout);
}

void Forest_Model::drawForest() {
    treeCols = treeColsWidget->value();
    treeRows = treeRowsWidget->value();
    treeType = treeTypeWidget->value();
    branchlength = branchlengthWidget->value();
    radius = radiusWidget->value();
    n = fractalWidget->value();
    createForest(treeCols, treeRows, treeType, branchlength, radius, n);
}

void Forest_Model::makeConnections() {
    connect(drawButton, SIGNAL(clicked()), this, SLOT(drawForest()));
}
