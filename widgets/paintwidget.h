#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QGraphicsView>
#include <QWidget>
#include <QMouseEvent>
#include "../enums/tooltype.h"
#include <QGraphicsItem>


class PaintWidget : public QGraphicsView
{
	Q_OBJECT

  public:
	explicit PaintWidget(QWidget *parent = nullptr);
	~PaintWidget();

	void setCurrentTool(ToolType tool);
	ToolType getCurrentTool() const {return currentTool_;}

	void setBrushSize(int size) { brushSize_ = size; }
	void setBrushColor(const QColor &color) { brushColor_ = color; }
	void setBrushStyle(Qt::PenStyle style) { brushStyle_ = style; }
	void setEraserSize(int size) { eraserSize_ = size; }
	void setBackgroundColor(const QColor &color);

  signals:
	void toolChanged(ToolType newTool);
	void itemDragStarted();

  protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;

  private:
	ToolType currentTool_;
	QPointF lastPoint_;
	bool isDrawing_;
	int brushSize_;
	int eraserSize_;
	QColor brushColor_;
	QColor backgroundColor_;
	Qt::PenStyle brushStyle_;

	bool isDragging_;
};

#endif // PAINTWIDGET_H
