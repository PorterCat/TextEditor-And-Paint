#include "texteditwidget.h"
#include "ui_mainwindow.h"
#include "ui_texteditwidget.h"

#include <QColorDialog>
#include <QFontDialog>

#include "../mainwindow.h"

TextEditWidget::TextEditWidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::TextEditWidget)
{
	ui->setupUi(this);

	// QAction* findRef;

	// if(MainWindow *cast = qobject_cast<MainWindow*>(parent))
	// 	findRef = cast->ui->actionFind;

	// ui->toolBar->addAction(findRef);
}

TextEditWidget::~TextEditWidget() { delete ui; }

void TextEditWidget::on_textEdit_textChanged()
{
	if (ui->textEdit->toPlainText() == originalText_)
	{
		isModified_ = false;
	}
	else if(!isModified_)
	{
		isModified_ = true;
	}
	emit textModified(this);
}

void TextEditWidget::openFile(const QString& filePath)
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
	ui->textEdit->setHtml(originalText_);
}

bool TextEditWidget::saveFile(const QString& filePath)
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
	originalText_ = ui->textEdit->toHtml();
	out << originalText_;
	isModified_ = false;
	fileinfo_ = new QFileInfo(filePath);
	emit textModified(this);
	return true;
}

void TextEditWidget::resetChanges()
{
	ui->textEdit->setPlainText(originalText_);
	isModified_ = false;
}

void TextEditWidget::on_actionSet_Color_triggered()
{
	QColor color = QColorDialog::getColor(ui->textEdit->textColor(), this);
	if (color.isValid())
	{
		ui->textEdit->setTextColor(color);
	}
}

void TextEditWidget::find(QString searchText)
{
	QTextDocument *document = ui->textEdit->document();
	QTextCursor cursor(document);
	cursor = document->find(searchText, cursor);
	if (!cursor.isNull())
		ui->textEdit->setTextCursor(cursor);
	else
		QMessageBox::information(this, tr("Find Text"), tr("Text not found."));
}

QTextEdit* TextEditWidget::getTextEdit() { return ui->textEdit; }

void TextEditWidget::on_actionSet_Font_triggered()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this);
	if (ok)
	{
		currentFont_ = font;
		QTextCursor cursor = ui->textEdit->textCursor();
		if (!cursor.hasSelection())
		{
			QTextCharFormat format;
			format.setFont(currentFont_);
			cursor.select(QTextCursor::Document);
			cursor.mergeCharFormat(format);
			ui->textEdit->mergeCurrentCharFormat(format);
		}
		else
		{
			QTextCharFormat format;
			format.setFont(currentFont_);
			cursor.mergeCharFormat(format);
			ui->textEdit->mergeCurrentCharFormat(format);
		}
	}
}

