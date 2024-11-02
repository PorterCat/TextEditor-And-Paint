#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include "documents/text_document.h"
#include <QCloseEvent>
#include <QFileInfo>
#include <QMainWindow>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui
{
class TextEditor;
}
QT_END_NAMESPACE

class TextEditor : public QMainWindow
{
    Q_OBJECT

  public:
    TextEditor(QWidget *parent = nullptr);
    ~TextEditor();

  private slots:
    void on_actionOpen_File_triggered();
    void on_actionSave_triggered();
    void on_actionNew_File_triggered();
    void on_actionNew_Table_triggered();
    void on_actionSave_As_triggered();
    void on_tabWidget_tabCloseRequested(int index);
    void on_actionFind_triggered();
    void on_actionReplace_triggered();
    void on_actionClear_triggered();
    void on_actionRestore_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionSet_Font_triggered();
    void on_actionSet_Color_triggered();
    void on_actionSet_Margin_triggered();
    void on_actionCut_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionClose_triggered();
    void on_actionClose_All_triggered();
    void on_actionMarkdown_Preview_triggered();

  protected:
    void closeEvent(QCloseEvent *event) override;

  private:
    QString getTabTitle(const QString &fileName);
    bool maybeSave();
    void loadSettings();
    void saveSettings();
    TextDocument tempContent;
    Ui::TextEditor *ui;
    QMap<QString, QTextEdit *> textEdits;
    QString lastFilePath;
};
#endif // TEXT_EDITOR_H
