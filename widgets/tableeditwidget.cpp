#include "tableeditwidget.h"
#include "ui_tableeditwidget.h"
#include <qmenu.h>
#include <qtimer.h>

TableEditWidget::TableEditWidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::TableEditWidget)
{
	ui->setupUi(this);
	ui->tableWidget->setColumnCount(2);
	ui->tableWidget->setRowCount(2);

	ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &TableEditWidget::showContextMenu);
}

TableEditWidget::~TableEditWidget() { delete ui; }

void TableEditWidget::showContextMenu(const QPoint &pos)
{
	QMenu contextMenu(tr("Context Menu"), this);

	QAction *addColumnAction = new QAction(tr("Add Column"), this);
	connect(addColumnAction, &QAction::triggered, this, &TableEditWidget::on_actionAdd_Column_triggered);
	contextMenu.addAction(addColumnAction);

	QAction *addRowAction = new QAction(tr("Add Row"), this);
	connect(addRowAction, &QAction::triggered, this, &TableEditWidget::on_actionAdd_Row_triggered);
	contextMenu.addAction(addRowAction);

	QAction *removeRowAction = new QAction(tr("Remove Row"), this);
	connect(removeRowAction, &QAction::triggered, this, &TableEditWidget::on_actionRemove_Row_triggered);
	contextMenu.addAction(removeRowAction);

	QAction *removeColumnAction = new QAction(tr("Remove Column"), this);
	connect(removeColumnAction, &QAction::triggered, this, &TableEditWidget::on_actionRemove_Column_triggered);
	contextMenu.addAction(removeColumnAction);

	contextMenu.exec(ui->tableWidget->mapToGlobal(pos));
}

void TableEditWidget::openFile(const QString& filePath)
{
	fileinfo_ = new QFileInfo(filePath);
	QFile file(filePath);

	if (!file.exists())
	{
		QMessageBox::warning(this, tr("File Not Found"), tr("The selected file does not exist."));
		return;
	}

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, tr("File Open Error"), tr("Could not open the file for reading."));
		return;
	}

	QTextStream in(&file);
	originalText_ = in.readAll();
	file.close();

	setTable(originalText_);
}

void TableEditWidget::setTable(QString& input)
{
	QStringList rows = input.split('\n');
	rows.removeLast();
	int numRows = rows.size();
	int numCols = 0;

	for (const QString& row : rows) {
		int cols = row.split(',').size();
		if (cols > numCols) {
			numCols = cols;
		}
	}

	ui->tableWidget->setRowCount(numRows);
	ui->tableWidget->setColumnCount(numCols);

	for (int row = 0; row < numRows; ++row)
	{
		QStringList columns = rows[row].split(',');
		for (int col = 0; col < columns.size(); ++col)
			ui->tableWidget->setItem(row, col, new QTableWidgetItem(columns[col]));
	}
}

QString TableEditWidget::getQStringFromTable() const
{
	QString result;
	int rowCount = ui->tableWidget->rowCount();
	int colCount = ui->tableWidget->columnCount();

	for (int row = 0; row < rowCount; ++row) {
		QStringList rowContents;
		for (int col = 0; col < colCount; ++col) {
			QTableWidgetItem* item = ui->tableWidget->item(row, col);
			if (item) {
				rowContents << item->text();
			} else {
				rowContents << "";
			}
		}
		result += rowContents.join(',') + '\n';
	}
	return result;
}

bool TableEditWidget::saveFile(const QString& filePath)
{
	if (filePath.isEmpty())
	{
		QMessageBox::information(this, tr("No File Selected"), tr("No file was selected."));
		return false;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, tr("File Save Error"), tr("Could not open the file for writing."));
		return false;
	}

	QTextStream out(&file);
	originalText_ = getQStringFromTable();
	out << originalText_;
	isModified_ = false;
	fileinfo_ = new QFileInfo(filePath);
	emit tableModified(this);
	return true;
}

void TableEditWidget::resetChanges()
{
	setTable(originalText_);
	isModified_ = false;
}

void TableEditWidget::on_tableWidget_cellChanged(int row, int column)
{
	if (getQStringFromTable() == originalText_)
	{
		isModified_ = false;
	}
	else if(!isModified_)
	{
		isModified_ = true;
	}
	emit tableModified(this);
}

void TableEditWidget::on_actionAdd_Column_triggered() { ui->tableWidget->insertColumn(ui->tableWidget->currentColumn() + 1); }
void TableEditWidget::on_actionAdd_Row_triggered() { ui->tableWidget->insertRow(ui->tableWidget->currentRow() + 1);}
void TableEditWidget::on_actionRemove_Column_triggered() { ui->tableWidget->removeColumn(ui->tableWidget->currentColumn());}
void TableEditWidget::on_actionRemove_Row_triggered() { ui->tableWidget->removeRow(ui->tableWidget->currentRow());}

