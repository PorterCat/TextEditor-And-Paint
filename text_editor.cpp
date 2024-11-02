#include "text_editor.h"
#include "documents/text_document.h"
#include "ui_text_editor.h"

#include "markdown_preview_dialog.h"
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
#include <qobject.h>
#include <qtypes.h>

TextEditor::TextEditor(QWidget *parent) : QMainWindow(parent), ui(new Ui::TextEditor)
{
    ui->setupUi(this);
    ui->tabWidget->setTabsClosable(true);
    loadSettings();
}

TextEditor::~TextEditor()
{
    delete ui;
}

void TextEditor::on_actionNew_File_triggered()
{
    QTextEdit *textEdit = new QTextEdit();
    int index = ui->tabWidget->addTab(textEdit, "Untitled");
    ui->tabWidget->setCurrentIndex(index);
    textEdit->append("# New File");
    textEdits[""] = textEdit;
}

void TextEditor::on_actionNew_Table_triggered()
{
    QTextEdit *textEdit = new QTextEdit();
    int index = ui->tabWidget->addTab(textEdit, "Untitled");
    ui->tabWidget->setCurrentIndex(index);
    textEdit->append("# Table Name");
    textEdit->append("| Syntax      | Description |");
    textEdit->append("| ----------- | ----------- |");
    textEdit->append("| Header      | Title       |");
    textEdit->append("| Paragraph   | Text        |");
    textEdits[""] = textEdit;
}

void TextEditor::on_actionOpen_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.txt *.csv *.md)"));

    if (fileName.isEmpty())
    {
        QMessageBox::information(this, tr("No File Selected"), tr("No file was selected."));
        return;
    }

    QFile file(fileName);
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
    QString content = in.readAll();
    file.close();

    if (textEdits.contains(fileName))
    {
        QTextEdit *textEdit = textEdits[fileName];
        int index = ui->tabWidget->indexOf(textEdit);
        if (index != -1)
        {
            ui->tabWidget->setCurrentIndex(index);
            return;
        }
    }

    if (ui->tabWidget->count() > 0)
    {
        QTextEdit *currentTextEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
        if (currentTextEdit && !currentTextEdit->document()->isEmpty())
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Confirm Overwrite"),
                                          tr("The current text edit is not empty. Do you want to overwrite it?"),
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {

                currentTextEdit->setPlainText(TextDocument(content).getContent());
                ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), getTabTitle(fileName));
                return;
            }
        }
    }

    QTextEdit *textEdit = new QTextEdit();
    textEdit->setPlainText(TextDocument(content).getContent());

    int index = ui->tabWidget->addTab(textEdit, getTabTitle(fileName));
    ui->tabWidget->setCurrentIndex(index);

    textEdits[fileName] = textEdit;
    lastFilePath = fileName;
    QMessageBox::information(this, tr("File Processed"), tr("The file has been successfully processed."));
}
void TextEditor::on_actionSave_triggered()
{
    int currentIndex = ui->tabWidget->currentIndex();
    if (currentIndex == -1)
    {
        QMessageBox::information(this, tr("No Tab Selected"), tr("No tab is selected."));
        return;
    }

    QString fileName = ui->tabWidget->tabText(currentIndex);
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());

    if (!textEdit)
    {
        QMessageBox::critical(this, tr("Error"), tr("No text edit found for the selected tab."));
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("File Save Error"), tr("Could not open the file for writing."));
        return;
    }

    QTextStream out(&file);
    out << textEdit->toPlainText();
    file.close();

    textEdit->document()->setModified(false);
    lastFilePath = fileName;

    QMessageBox::information(this, tr("File Saved"), tr("The file has been successfully saved."));
}

void TextEditor::on_actionSave_As_triggered()
{
    int currentIndex = ui->tabWidget->currentIndex();
    if (currentIndex == -1)
    {
        QMessageBox::information(this, tr("No Tab Selected"), tr("No tab is selected."));
        return;
    }

    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save Markdown File"), "", tr("Markdown Files (*.md *.markdown)"));

    if (fileName.isEmpty())
    {
        QMessageBox::information(this, tr("No File Selected"), tr("No file was selected."));
        return;
    }

    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());

    if (!textEdit)
    {
        QMessageBox::critical(this, tr("Error"), tr("No text edit found for the selected tab."));
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("File Save Error"), tr("Could not open the file for writing."));
        return;
    }

    QTextStream out(&file);
    out << textEdit->toPlainText();
    file.close();

    ui->tabWidget->setTabText(currentIndex, getTabTitle(fileName));
    textEdits[fileName] = textEdit;

    textEdit->document()->setModified(false);
    lastFilePath = fileName;

    QMessageBox::information(this, tr("File Saved"), tr("The file has been successfully saved."));
}

void TextEditor::on_tabWidget_tabCloseRequested(int index)
{
    if (maybeSave())
    {
        textEdits.remove(ui->tabWidget->tabText(index));
        ui->tabWidget->removeTab(index);
    }
}

void TextEditor::on_actionClose_triggered()
{
    qsizetype index = ui->tabWidget->currentIndex();
    on_tabWidget_tabCloseRequested(index);
}

void TextEditor::on_actionClose_All_triggered()
{
    qsizetype index = ui->tabWidget->currentIndex();
    while (ui->tabWidget->currentIndex() != -1)
    {
        index = ui->tabWidget->currentIndex();
        on_tabWidget_tabCloseRequested(index);
    }
}

QString TextEditor::getTabTitle(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    QString baseName = fileInfo.fileName();

    int count = 0;
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        if (ui->tabWidget->tabText(i).startsWith(baseName))
        {
            count++;
        }
    }

    if (count > 0)
    {
        return " (" + fileInfo.path() + ")" + fileInfo.baseName();
    }

    return baseName;
}

bool TextEditor::maybeSave()
{
    int currentIndex = ui->tabWidget->currentIndex();
    if (currentIndex == -1)
        return true;

    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return true;

    if (textEdit->document()->isModified())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Markdown Editor"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
        {
            on_actionSave_triggered();
            return true;
        }
        else if (ret == QMessageBox::Cancel)
        {
            return false;
        }
    }
    return true;
}

void TextEditor::on_actionFind_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    bool ok;
    QString searchText =
        QInputDialog::getText(this, tr("Find Text"), tr("Enter text to find:"), QLineEdit::Normal, "", &ok);
    if (ok && !searchText.isEmpty())
    {
        QTextDocument *document = textEdit->document();
        QTextCursor cursor(document);
        cursor = document->find(searchText, cursor);
        if (!cursor.isNull())
        {
            textEdit->setTextCursor(cursor);
        }
        else
        {
            QMessageBox::information(this, tr("Find Text"), tr("Text not found."));
        }
    }
}

void TextEditor::on_actionReplace_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    bool ok;
    QString searchText =
        QInputDialog::getText(this, tr("Replace Text"), tr("Enter text to find:"), QLineEdit::Normal, "", &ok);
    if (ok && !searchText.isEmpty())
    {
        QString replaceText = QInputDialog::getText(this, tr("Replace Text"), tr("Enter text to replace with:"),
                                                    QLineEdit::Normal, "", &ok);
        if (ok && !replaceText.isEmpty())
        {
            QTextDocument *document = textEdit->document();
            QTextCursor cursor(document);
            cursor = document->find(searchText, cursor);
            if (!cursor.isNull())
            {
                cursor.insertText(replaceText);
                textEdit->setTextCursor(cursor);
            }
            else
            {
                QMessageBox::information(this, tr("Replace Text"), tr("Text not found."));
            }
        }
    }
}

void TextEditor::on_actionClear_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    QString temp = textEdit->toPlainText();

    tempContent.load(temp);

    textEdit->clear();
}

void TextEditor::on_actionRestore_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    textEdit->setPlainText(tempContent.getContent());
}

void TextEditor::on_actionCopy_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    QApplication::clipboard()->setText(textEdit->textCursor().selectedText());
}

void TextEditor::on_actionCut_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    QApplication::clipboard()->setText(textEdit->textCursor().selectedText());
}

void TextEditor::on_actionPaste_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    textEdit->insertPlainText(QApplication::clipboard()->text());
}

void TextEditor::on_actionSet_Font_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    bool ok;
    QFont font = QFontDialog::getFont(&ok, textEdit->font(), this);
    if (ok)
    {
        textEdit->setFont(font);
    }
}

void TextEditor::on_actionSet_Color_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    QColor color = QColorDialog::getColor(textEdit->textColor(), this);
    if (color.isValid())
    {
        textEdit->setTextColor(color);
    }
}

void TextEditor::on_actionSet_Margin_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    bool ok;
    int margin = QInputDialog::getInt(this, tr("Set Margins"), tr("Enter margin size:"),
                                      textEdit->document()->documentMargin(), 0, 100, 1, &ok);
    if (ok)
    {
        textEdit->document()->setDocumentMargin(margin);
    }
}

void TextEditor::loadSettings()
{
    QSettings settings("PorterCatNMegaDrage", "QT-lab5");
    lastFilePath = settings.value("lastFilePath", "").toString();
}

void TextEditor::saveSettings()
{
    QSettings settings("PorterCatNMegaDrage", "QT-lab5");
    settings.setValue("lastFilePath", lastFilePath);
}

void TextEditor::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        saveSettings();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void TextEditor::on_actionUndo_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        textEdit->undo();
    }
}

void TextEditor::on_actionRedo_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (textEdit)
    {
        textEdit->redo();
    }
}

void TextEditor::on_actionMarkdown_Preview_triggered()
{
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(ui->tabWidget->currentWidget());
    if (!textEdit)
        return;

    QString markdownContent = textEdit->toPlainText();
    MarkdownPreviewDialog *previewDialog = new MarkdownPreviewDialog(markdownContent, this);
    previewDialog->exec();
}
