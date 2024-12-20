#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "widgets/ieditablewidget.h"
#include <QMainWindow>
#include "enums/worktype.h"

#include <QFileDialog>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui
{
	class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	bool isTabSelected();

	static IEditableWidget* parseToEditableWidget(QWidget* widget);

  private slots:
	void on_tabWidget_tabCloseRequested(int index);

	void on_actionOpen_triggered();

	void on_actionFind_triggered();

	void on_actionNew_File_triggered();

	bool on_actionSave_triggered();

	bool on_actionSave_as_triggered();

	void onFileModified(IEditableWidget* widget);

	void on_actionNew_Table_triggered();

	void closeEvent(QCloseEvent *event) override;

	bool maybeSave(IEditableWidget* widget);
	void on_actionNew_Paint_triggered();

	void on_actionClose_triggered();

	void on_actionClose_All_triggered();

	void on_actionUndo_triggered();

	void on_actionRedo_triggered();

	void on_actionCut_triggered();

	void on_actionCopy_triggered();

	void on_actionPaste_triggered();

  private:
	Ui::MainWindow *ui;

	QWidget* initilizeTab(WorkType worktype);
};
#endif // MAINWINDOW_H
