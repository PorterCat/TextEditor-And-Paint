#include "tableedit_widget.h"
#include "ui_tableedit_widget.h"
#include <QMenu>
#include <QAction>

TableEditWidget::TableEditWidget(QWidget *parent) :
      QWidget(parent),
      ui(new Ui::TableEditWidget)
{
    ui->setupUi(this);

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &TableEditWidget::on_tableWidget_customContextMenuRequested);

    contextMenu = new QMenu(this);
    QAction *addRowAction = contextMenu->addAction("Add Row");
    QAction *addColumnAction = contextMenu->addAction("Add Column");
    QAction *removeRowAction = contextMenu->addAction("Remove Row");
    QAction *removeColumnAction = contextMenu->addAction("Remove Column");

    connect(addRowAction, &QAction::triggered, this, &TableEditWidget::addRow);
    connect(addColumnAction, &QAction::triggered, this, &TableEditWidget::addColumn);
    connect(removeRowAction, &QAction::triggered, this, &TableEditWidget::removeRow);
    connect(removeColumnAction, &QAction::triggered, this, &TableEditWidget::removeColumn);
}

TableEditWidget::~TableEditWidget()
{
    delete ui;
}

void TableEditWidget::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    // Показываем контекстное меню
    contextMenu->exec(ui->tableWidget->mapToGlobal(pos));
}

void TableEditWidget::addRow()
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
}

void TableEditWidget::addColumn()
{
    int column = ui->tableWidget->columnCount();
    ui->tableWidget->insertColumn(column);
}

void TableEditWidget::removeRow()
{
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        ui->tableWidget->removeRow(row);
    }
}

void TableEditWidget::removeColumn()
{
    int column = ui->tableWidget->currentColumn();
    if (column >= 0) {
        ui->tableWidget->removeColumn(column);
    }
}
