#include "paintwidget.h"

PaintWidget::PaintWidget(QWidget *parent)
	: QGraphicsView(parent),
	  currentTool_(ToolType::NoTool),
	  isDrawing_(false),
	  brushSize_(5),
	  eraserSize_(10),
	  brushColor_(Qt::black),
	  brushStyle_(Qt::SolidLine),
	  backgroundColor_(Qt::white),
	  isDragging_(false)
{
}

PaintWidget::~PaintWidget(){}

void PaintWidget::resizeEvent(QResizeEvent *event) {
	QGraphicsView::resizeEvent(event);
	if (scene()) {
		fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
	}
}

void PaintWidget::setCurrentTool(ToolType tool)
{
	currentTool_ = tool;
	emit toolChanged(currentTool_);
	qDebug() << "Tool changed to:" << currentTool_;
}

void PaintWidget::setBackgroundColor(const QColor &color)
{
	backgroundColor_ = color;
	if (scene())
		scene()->setBackgroundBrush(backgroundColor_);
}

void PaintWidget::mousePressEvent(QMouseEvent *event)
{
	QPointF scenePos = mapToScene(event->pos());

	QGraphicsItem* item = scene()->itemAt(scenePos, QTransform());

	if (event->button() == Qt::LeftButton) {
		if (currentTool_ == BrushTool || currentTool_ == EraserTool) {
			isDrawing_ = true;
			lastPoint_ = scenePos;
			qDebug() << "Mouse pressed at:" << lastPoint_ << "with tool:" << currentTool_;
		}

		if (item && (item->flags() & QGraphicsItem::ItemIsMovable)) {
			isDragging_ = true;
			emit itemDragStarted();
			qDebug() << "Item drag started.";
		}
	}
	QGraphicsView::mousePressEvent(event);
}

void PaintWidget::mouseMoveEvent(QMouseEvent *event)
{
	QPointF currentPoint = mapToScene(event->pos());

	if ((event->buttons() & Qt::LeftButton) && isDrawing_) {
		if (currentTool_ == BrushTool)
		{
			QPen pen(brushColor_, brushSize_, brushStyle_);
			scene()->addLine(QLineF(lastPoint_, currentPoint), pen);
			lastPoint_ = currentPoint;
		}
		else if (currentTool_ == EraserTool)
		{
			QList<QGraphicsItem *> items = scene()->items(QRectF(currentPoint - QPointF(eraserSize_, eraserSize_),
																 QSizeF(eraserSize_ * 2, eraserSize_ * 2)));
			foreach (QGraphicsItem *item, items)
			{
				scene()->removeItem(item);
				delete item;
			}
		}
	}

	QGraphicsView::mouseMoveEvent(event);
}

void PaintWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (isDrawing_) {
			isDrawing_ = false;
			qDebug() << "Drawing stopped.";
		}
		if (isDragging_)
		{
			isDragging_ = false;
			qDebug() << "Item drag stopped.";
		}
	}
	QGraphicsView::mouseReleaseEvent(event);
}
