#include "text_editor.h"
#include "./ui_text_editor.h"

#include <QFileDialog>
#include <QTextEdit>

TextEditor::TextEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TextEditor)
{
    ui->setupUi(this);
    // ui->toolBar->addAction(ui->actionNew_File);
    ui->tabWidget->setTabsClosable(true);

    // QTextDocument* qTextDocument = new QTextDocument();
    // ui->tabWidget->addTab(qTextDocument);

    QTextEdit *textEdit = new QTextEdit();

    ui->tabWidget->addTab(textEdit, "Редактор текста");

    textEdit->append("123");
    textEdit->setTextColor(Qt::red);
    textEdit->append("456");
}

TextEditor::~TextEditor()
{
    delete ui;
}

void TextEditor::on_actionOpen_File_triggered()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, tr("All Files (*.txt *.csv)"));
}


void TextEditor::on_tabWidget_tabCloseRequested(int index)
{
    ui->tabWidget->removeTab(index);
}

