#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "widgets/sceneeditwidget.h"
#include "widgets/tableeditwidget.h"
#include "widgets/texteditwidget.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QClipboard>
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPointer>
#include <QSettings>
#include <QTextEdit>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

bool MainWindow::maybeSave(IEditableWidget* widget)
{
	if (widget->isModified())
	{
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("Content Editor"),
								   tr("The document has been modified.\n"
									  "Do you want to save your changes?"),
								   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save)
		{
			if (!on_actionSave_triggered())  // Можно вернуть false, если сохранение не удалось
				return false;
		}
		else if (ret == QMessageBox::Cancel)
		{
			return false;  // Отменить действие, если выбрано "Cancel"
		}
	}
	return true;  // Если изменений нет или сохранение прошло успешно
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
	IEditableWidget* widget = dynamic_cast<IEditableWidget*>(ui->tabWidget->widget(index));
	if (widget && maybeSave(widget))
		ui->tabWidget->removeTab(index);
}

void MainWindow::onFileModified(IEditableWidget* widget)
{
	int index = ui->tabWidget->indexOf(dynamic_cast<QWidget*>(widget));
	if (index != -1)
		ui->tabWidget->setTabText(index, widget->isModified() ? widget->getFileName() + '*' : widget->getFileName());
}

void MainWindow::on_actionOpen_triggered()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), "",
	tr("All (*txt *html *csv *json);;Text Files (*.txt *.html);;Table Files (*csv);;Interactive scene (*json)"));

	if (filePath.isEmpty())
	{
		QMessageBox::information(this, tr("No File Selected"), tr("No file was selected."));
		return;
	}

	QWidget* widget = initilizeTab(getWorktypeByExtension(QFileInfo(filePath).suffix().toLower()));

	if(widget != nullptr)
		dynamic_cast<IEditableWidget*>(widget)->openFile(filePath);
}

void MainWindow::on_actionFind_triggered()
{
	TextEditWidget *textEdit = qobject_cast<TextEditWidget*>(ui->tabWidget->currentWidget());
	if (!textEdit)
		return;

	bool ok;
	QString searchText =
		QInputDialog::getText(this, tr("Find Text"), tr("Enter text to find:"), QLineEdit::Normal, "", &ok);
	if (ok && !searchText.isEmpty())
		textEdit->find(searchText);
}

bool MainWindow::on_actionSave_triggered()
{
	if(! isTabSelected()) return false;
	IEditableWidget* widget = parseToEditableWidget(ui->tabWidget->currentWidget());

	if(widget == nullptr) return false;

	if (!widget->isFileExist())
		if(on_actionSave_as_triggered())
			return true;
		else
			return false;
	else
	{
		widget->saveFile(widget->getFilePath());
		return true;
	}
}

bool MainWindow::on_actionSave_as_triggered()
{
	if(! isTabSelected()) return false;

	QString filePath;
	QString fileName;
	IEditableWidget* widget;

	if ((widget = parseToEditableWidget(ui->tabWidget->currentWidget())))
	{
		switch(widget->getWorkType())
		{
			case WorkType::Text:
				filePath = QFileDialog::getSaveFileName(this, tr("Save File"), widget->getFileName(), tr("Text Files (*.html)"));
				break;
			case WorkType::Table:
				filePath = QFileDialog::getSaveFileName(this, tr("Save File"), widget->getFileName(), tr("Table Files (*.csv)"));
				break;
			case WorkType::InteractiveScene:
				filePath = QFileDialog::getSaveFileName(this, tr("Save File"), widget->getFileName(), tr("Interactive Scene (*.json)"));
				break;
			default:
				return false;
		}
	}
	else
	{
		QMessageBox::warning(this, tr("Unsupported Widget"), tr("The current widget does not support saving."));
	}

	if(widget != nullptr)
		if(widget->saveFile(filePath))
			return true;
	return false;
}

QWidget* MainWindow::initilizeTab(WorkType worktype)
{
	QWidget* editWidget = nullptr;

	int index = -1;
	switch(worktype)
	{
	case WorkType::Text :
		editWidget = new TextEditWidget(ui->tabWidget);
		connect(qobject_cast<TextEditWidget*>(editWidget), &TextEditWidget::textModified, this, &MainWindow::onFileModified);
		index = ui->tabWidget->addTab(editWidget, qobject_cast<TextEditWidget*>(editWidget)->getFileName());
		break;
	case WorkType::Table :
		editWidget = new TableEditWidget(ui->tabWidget);
		connect(qobject_cast<TableEditWidget*>(editWidget), &TableEditWidget::tableModified, this, &MainWindow::onFileModified);
		index = ui->tabWidget->addTab(editWidget, qobject_cast<TableEditWidget*>(editWidget)->getFileName());
		break;
	case WorkType::InteractiveScene :
		editWidget = new SceneEditWidget(ui->tabWidget);
		connect(qobject_cast<SceneEditWidget*>(editWidget), &SceneEditWidget::sceneModified, this, &MainWindow::onFileModified);
		index = ui->tabWidget->addTab(editWidget, qobject_cast<SceneEditWidget*>(editWidget)->getFileName());
		break;
	default:
		break;
	}
	ui->tabWidget->setCurrentIndex(index);
	return editWidget;
}

IEditableWidget* MainWindow::parseToEditableWidget(QWidget* currentWidget)
{
	if (TextEditWidget* textWidget = qobject_cast<TextEditWidget*>(currentWidget))
		return textWidget;
	else if (TableEditWidget* tableWidget = qobject_cast<TableEditWidget*>(currentWidget))
		return tableWidget;
	else
		return nullptr;
}

bool MainWindow::isTabSelected()
{
	int currentIndex = ui->tabWidget->currentIndex();
	if (currentIndex == -1)
	{
		QMessageBox::information(this, tr("No Tab Selected"), tr("No tab is selected."));
		return false;
	}
	return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	for (int i = 0; i < ui->tabWidget->count(); ++i)
	{
		QWidget* currentWidget = ui->tabWidget->widget(i);

		IEditableWidget* editableWidget = dynamic_cast<IEditableWidget*>(currentWidget);
		if (editableWidget && editableWidget->isModified())
		{
			QMessageBox::StandardButton reply = QMessageBox::warning(this, tr("Unsaved Changes"),
																	 tr("You have unsaved changes. Do you want to save them?"),
																	 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
			if (reply == QMessageBox::Save) {
				on_actionSave_as_triggered();
			}
			else if (reply == QMessageBox::Cancel)
			{
				event->ignore();
				return;
			}
		}
	}
	event->accept();
}

void MainWindow::on_actionNew_File_triggered() { initilizeTab(WorkType::Text); }
void MainWindow::on_actionNew_Table_triggered(){ initilizeTab(WorkType::Table); }
void MainWindow::on_actionNew_Paint_triggered() { initilizeTab(WorkType::InteractiveScene); }

void MainWindow::on_actionClose_triggered()
{
	IEditableWidget* widget = dynamic_cast<IEditableWidget*>(ui->tabWidget->currentWidget());
	if(maybeSave(widget))
		ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabWidget->currentWidget()));
}


void MainWindow::on_actionClose_All_triggered()
{
	for (int i = 0; i < ui->tabWidget->count(); ++i)
	{
		QWidget* currentWidget = ui->tabWidget->widget(i);

		IEditableWidget* editableWidget = dynamic_cast<IEditableWidget*>(currentWidget);
		if (editableWidget && editableWidget->isModified())
		{
			QMessageBox::StandardButton reply = QMessageBox::warning(this, tr("Unsaved Changes"),
																	 tr("You have unsaved changes. Do you want to save them?"),
																	 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
			if (reply == QMessageBox::Save) {
				on_actionSave_as_triggered();
			}
			else if (reply == QMessageBox::Cancel)
			{
				return;
			}
		}
	}
}

void MainWindow::on_actionUndo_triggered()
{
	TextEditWidget *textEdit = qobject_cast<TextEditWidget*>(ui->tabWidget->currentWidget());
	if(textEdit)
		textEdit->getTextEdit()->undo();
}


void MainWindow::on_actionRedo_triggered()
{
	TextEditWidget *textEdit = qobject_cast<TextEditWidget*>(ui->tabWidget->currentWidget());
	if(textEdit)
		textEdit->getTextEdit()->redo();
}


void MainWindow::on_actionCut_triggered()
{
	TextEditWidget *textEdit = qobject_cast<TextEditWidget*>(ui->tabWidget->currentWidget());
	if(textEdit)
		QApplication::clipboard()->setText(textEdit->getTextEdit()->textCursor().selectedText());
}


void MainWindow::on_actionCopy_triggered()
{
	TextEditWidget *textEdit = qobject_cast<TextEditWidget*>(ui->tabWidget->currentWidget());
	if(textEdit)
		QApplication::clipboard()->setText(textEdit->getTextEdit()->textCursor().selectedText());
}


void MainWindow::on_actionPaste_triggered()
{
	TextEditWidget *textEdit = qobject_cast<TextEditWidget*>(ui->tabWidget->currentWidget());
	if(textEdit)
		textEdit->getTextEdit()->insertPlainText(QApplication::clipboard()->text());
}

