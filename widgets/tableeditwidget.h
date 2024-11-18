#ifndef TABLEEDITWIDGET_H
#define TABLEEDITWIDGET_H

#include "ieditablewidget.h"

namespace Ui
{
	class TableEditWidget;
}

class TableEditWidget : public QWidget, public IEditableWidget
{
	Q_OBJECT

  public:
	explicit TableEditWidget(QWidget *parent = nullptr);
	~TableEditWidget();

	void openFile(const QString& filePath) override;
	bool saveFile(const QString& filePath) override;
	bool isModified() const override { return isModified_; }
	bool isFileExist() const override {return (fileinfo_ == nullptr) ? false : true; }
	void resetChanges() override;

	QString getFileName() override { return (fileinfo_ == nullptr) ? "Untitled" : fileinfo_->fileName(); };
	QString getFilePath() override
	{
		if(fileinfo_ == nullptr)
			throw std::runtime_error("fileinfo_ is null");
		return fileinfo_->filePath();
	}
	WorkType getWorkType() override {return WorkType::Table; }

	void showContextMenu(const QPoint &pos);

  signals:
	void tableModified(TableEditWidget* widget);

  private slots:
	void on_tableWidget_cellChanged(int row, int column);

	void on_actionAdd_Column_triggered();

	void on_actionAdd_Row_triggered();

	void on_actionRemove_Row_triggered();

	void on_actionRemove_Column_triggered();

  private:
	Ui::TableEditWidget *ui;
	QString originalText_;
	QFileInfo* fileinfo_ = nullptr;
	bool isModified_ = false;

	QString getQStringFromTable() const;
	void setTable(QString& input);
};

#endif // TABLEEDITWIDGET_H
