#include <QDialog>
#include <QTextEdit>

class MarkdownPreviewDialog : public QDialog
{
    Q_OBJECT

  public:
    MarkdownPreviewDialog(const QString &markdownContent, QWidget *parent);

  private:
    QTextEdit *textEdit;
};
