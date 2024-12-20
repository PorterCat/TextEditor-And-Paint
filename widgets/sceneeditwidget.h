#ifndef SCENEEDITWIDGET_H
#define SCENEEDITWIDGET_H

#include "ieditablewidget.h"
#include "paintwidget.h"
#include <QWidget>
#include <qgraphicsscene.h>
#include <QSoundEffect>

namespace Ui
{
	class SceneEditWidget;
}

class SceneEditWidget : public QWidget, public IEditableWidget
{
	Q_OBJECT

  public:
	explicit SceneEditWidget(QWidget *parent = nullptr);
	~SceneEditWidget();

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
	WorkType getWorkType() override {return WorkType::InteractiveScene; }

  signals:
	void sceneModified(SceneEditWidget* widget);

  private slots:
	void on_brushSizeSlider_valueChanged(int value);

	void on_brushButton_clicked();

	void on_eraserButton_clicked();

	void on_eraserSizeSlider_valueChanged(int value);

	void on_clearCanvas_clicked();

	void on_colorButton_clicked();

	void on_addShapeButton_clicked();

	void on_addImageButton_clicked();

	void on_addTextButton_clicked();

	void on_scaleSlider_valueChanged(int value);

	void on_rotateSlider_valueChanged(int value);

	void on_saveImageButton_clicked();

	void on_mergeShapesButton_clicked();

	void on_startMotionButton_clicked();

	void on_selectButton_clicked();

	void on_changeBackground_clicked();

  private:
	Ui::SceneEditWidget *ui;

	QString originalText_;
	QFileInfo* fileinfo_ = nullptr;
	bool isModified_ = false;

	QGraphicsScene* scene_;
	PaintWidget* paintWidget_;

	// Для движения
	QGraphicsItem* selectedItem;
	QGraphicsItemGroup* selectedGroup;
	QTimer* movementTimer;
	int dx, dy;
	int movementDuration;

	// Звуковой эффект для столкновений
	QSoundEffect collisionSound;

	void stopMovingItem();
	void updateItemPosition();
};

#endif // SCENEEDITWIDGET_H
