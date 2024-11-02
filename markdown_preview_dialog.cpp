#include "markdown_preview_dialog.h"
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

MarkdownPreviewDialog::MarkdownPreviewDialog(const QString &markdownContent, QWidget *parent = nullptr)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setMarkdown(markdownContent);

    layout->addWidget(textEdit);
    setLayout(layout);
}
