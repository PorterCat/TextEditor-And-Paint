#ifndef IEDITABLEWIDGET_H
#define IEDITABLEWIDGET_H

#include "../enums/worktype.h"

#include <QString>
#include <QFile>
#include <QWidget>
#include <QFileInfo>
#include <QMessageBox>

class IEditableWidget
{
  public:
	virtual ~IEditableWidget() {}

	virtual void openFile(const QString& filePath) = 0;
	virtual bool saveFile(const QString& filePath) = 0;

	virtual bool isModified() const = 0;
	virtual bool isFileExist() const =0;
	virtual void resetChanges() = 0;

	virtual QString getFileName() = 0;
	virtual QString getFilePath() = 0;
	virtual WorkType getWorkType() = 0;
};

#endif // IEDITABLEWIDGET_H
