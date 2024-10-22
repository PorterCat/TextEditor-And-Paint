#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
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

    void on_tabWidget_tabCloseRequested(int index);

private:
    Ui::TextEditor *ui;
};
#endif // TEXT_EDITOR_H
