#ifndef TEXTEDITWIDGET_H
#define TEXTEDITWIDGET_H

#include "ieditablewidget.h"
#include <qtextedit.h>
#include <qtoolbar.h>

namespace Ui
{
	class TextEditWidget;
}

class TextEditWidget : public QWidget, public IEditableWidget
{
	Q_OBJECT

  public:
	explicit TextEditWidget(QWidget *parent = nullptr);
	~TextEditWidget();

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
	WorkType getWorkType() override {return WorkType::Text; }
	void find(QString searchText);

	QTextEdit* getTextEdit();

  signals:
	void textModified(TextEditWidget* widget);

  private slots:
	void on_textEdit_textChanged();

	void on_actionSet_Color_triggered();

	void on_actionSet_Font_triggered();

  private:
	Ui::TextEditWidget *ui;
	QString originalText_;
	QFont currentFont_;
	QFileInfo* fileinfo_ = nullptr;
	bool isModified_ = false;
};

#endif // TEXTEDITWIDGET_H
