#include "sceneeditwidget.h"
// #include "ui_sceneeditwidget.h"
#include "paintwidget.h"
#include "widgets/ui_sceneeditwidget.h"
#include <qgraphicsscene.h>

#include <QInputDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QTimer>
#include <QGraphicsView>

SceneEditWidget::SceneEditWidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::SceneEditWidget), dx(5), dy(5), movementDuration(5000),
	  selectedGroup(nullptr), selectedItem(nullptr)
{
	ui->setupUi(this);
	scene_ = new QGraphicsScene(0, 0, 565, 500, this);
	paintWidget_ = new PaintWidget(ui->widget);
	paintWidget_->setScene(scene_);
	scene_->setBackgroundBrush(QColorConstants::White);
	paintWidget_->setBrushColor(QColorConstants::Black);

	ui->widget = paintWidget_;
	ui->brushButton->setCheckable(true);
	ui->eraserButton->setCheckable(true);
	ui->selectButton->setCheckable(true);
	ui->selectButton->setChecked(true);

	collisionSound.setSource(QUrl(QString::fromUtf8(":/sounds/sounds/collision.wav3")));
	collisionSound.setLoopCount(5);
	collisionSound.setVolume(0.5f);

	movementTimer = new QTimer(this);
	connect(movementTimer, &QTimer::timeout, this, &SceneEditWidget::updateItemPosition);
}

SceneEditWidget::~SceneEditWidget() { delete ui; }

void SceneEditWidget::openFile(const QString& filePath)
{
	fileinfo_ = new QFileInfo(filePath);
	QFile file(filePath);

	if (!file.exists())
	{
		QMessageBox::warning(this, tr("File Not Found"), tr("The selected file does not exist."));
		return;
	}

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, tr("File Open Error"), tr("Could not open the file for reading."));
		return;
	}

	QTextStream in(&file);
	originalText_ = in.readAll();
	file.close();
}

bool SceneEditWidget::saveFile(const QString& filePath)
{
	if (filePath.isEmpty())
	{
		QMessageBox::information(this, tr("No File Selected"), tr("No file was selected."));
		return false;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, tr("File Save Error"), tr("Could not open the file for writing."));
		return false;
	}

	QTextStream out(&file);
	out << originalText_;
	isModified_ = false;
	fileinfo_ = new QFileInfo(filePath);
	emit sceneModified(this);
	return true;
}

void SceneEditWidget::resetChanges()
{
	isModified_ = false;
}

void SceneEditWidget::on_brushSizeSlider_valueChanged(int value) {paintWidget_->setBrushSize(value);}

void SceneEditWidget::on_brushButton_clicked()
{
	paintWidget_->setCurrentTool(ToolType::BrushTool);
	ui->brushButton->setChecked(true);
	ui->eraserButton->setChecked(false);
	ui->selectButton->setChecked(false);
}

void SceneEditWidget::on_eraserButton_clicked()
{
	paintWidget_->setCurrentTool(ToolType::EraserTool);
	ui->eraserButton->setChecked(true);
	ui->brushButton->setChecked(false);
	ui->selectButton->setChecked(false);
}

void SceneEditWidget::on_eraserSizeSlider_valueChanged(int value){paintWidget_->setEraserSize(value); }

void SceneEditWidget::on_clearCanvas_clicked()
{
	scene_->clear();
	scene_->setBackgroundBrush(QColorConstants::White);
}

void SceneEditWidget::on_colorButton_clicked()
{
	QColor brushColor;
	QColor color = QColorDialog::getColor(brushColor, this, "Select color");
	if (color.isValid())
		paintWidget_->setBrushColor(color);
}

void SceneEditWidget::on_addShapeButton_clicked()
{
	paintWidget_->setCurrentTool(ToolType::NoTool);
	QStringList shapes = {"Rectangle", "Ellipse", "Line"};
	bool ok;
	QString shape = QInputDialog::getItem(this, "Select shape", "Select shape:", shapes, 0, false, &ok);
	if (ok && !shape.isEmpty()) {
		int width = QInputDialog::getInt(this, "Width", "Enter width:", 100, 1, 1000, 1, &ok);
		if (!ok) return;
		int height = QInputDialog::getInt(this, "Height", "Enter height:", 100, 1, 1000, 1, &ok);
		if (!ok) return;

		QColor fillColor = QColorDialog::getColor(Qt::white, this, "Select fill color");
		if (!fillColor.isValid()) return;
		QColor outlineColor = QColorDialog::getColor(Qt::black, this, "Select outline color");
		if (!outlineColor.isValid()) return;

		QPen pen(outlineColor);
		QBrush brush(fillColor);

		QGraphicsItem *item = nullptr;

		if (shape == "Rectangle") {
			item = scene_->addRect(0, 0, width, height, pen, brush);
		} else if (shape == "Ellipse") {
			item = scene_->addEllipse(0, 0, width, height, pen, brush);
		} else if (shape == "Line") {
			int lineWidth = QInputDialog::getInt(this, "Thickness", "Enter thickness:", pen.width(), 1, 100, 1, &ok);
			if (!ok) return;

			pen.setWidth(lineWidth);

			item = scene_->addLine(0, 0, width, height, pen);
		}

		if (item) {
			item->setFlag(QGraphicsItem::ItemIsMovable);
			item->setFlag(QGraphicsItem::ItemIsSelectable);
			item->setFlag(QGraphicsItem::ItemIsFocusable);
		}
	}
}

void SceneEditWidget::on_addImageButton_clicked()
{
	paintWidget_->setCurrentTool(ToolType::NoTool);
	QString fileName = QFileDialog::getOpenFileName(this, tr("Upload image"), "", tr("Images (*.png *.jpg *.bmp)"));
	if (!fileName.isEmpty()) {
		QPixmap pixmap(fileName);
		if (!pixmap.isNull()) {
			QGraphicsPixmapItem *pixmapItem = scene_->addPixmap(pixmap);
			pixmapItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
		} else {
			QMessageBox::warning(this, tr("Error"), tr("Can upload image"));
		}
	}
}


void SceneEditWidget::on_addTextButton_clicked()
{
	QFont textFont;
	bool ok;
	QString text = QInputDialog::getText(this, "Add text", "Enter the text:", QLineEdit::Normal, "", &ok);
	if (ok && !text.isEmpty()) {
		QFont font = QFontDialog::getFont(&ok, textFont, this);
		if (ok) {
			QGraphicsTextItem* textItem = scene_->addText(text, font);
			textItem->setDefaultTextColor(QColorConstants::Black);
			textItem->setFlag(QGraphicsItem::ItemIsMovable);
			textItem->setFlag(QGraphicsItem::ItemIsSelectable);
		}
	}
}


void SceneEditWidget::on_scaleSlider_valueChanged(int value)
{
	QList<QGraphicsItem *> selectedItems = scene_->selectedItems();
	if (selectedItems.isEmpty()) return;

	double scaleFactor = value / 100.0;
	foreach (QGraphicsItem *item, selectedItems) {
		item->setScale(scaleFactor);
	}
}


void SceneEditWidget::on_rotateSlider_valueChanged(int value)
{
	QList<QGraphicsItem *> selectedItems = scene_->selectedItems();
	if (selectedItems.isEmpty()) return;

	foreach (QGraphicsItem *item, selectedItems) {
		QRectF boundingRect = item->boundingRect();
		QPointF center = boundingRect.center();

		item->setTransformOriginPoint(center);

		item->setRotation(static_cast<qreal>(value));
	}
}


void SceneEditWidget::on_saveImageButton_clicked()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", "", "PNG Image (*.png);;JPEG Image (*.jpg);;BMP Image (*.bmp)");
	if (!fileName.isEmpty()) {
		QImage image(scene_->sceneRect().size().toSize(), QImage::Format_ARGB32);
		image.fill(Qt::transparent);

		QPainter painter(&image);
		scene_->render(&painter);
		image.save(fileName);
	}
}

void SceneEditWidget::on_mergeShapesButton_clicked()
{
	QList<QGraphicsItem *> selectedItems = scene_->selectedItems();

	if (selectedItems.size() < 2) {
		QMessageBox::information(this, tr("Merging shapes"), tr("Select at least two objects"));
		return;
	}

	QGraphicsItemGroup* group = new QGraphicsItemGroup();

	qreal maxZValue = 0;
	for (QGraphicsItem* item : selectedItems) {
		if (item->zValue() > maxZValue) {
			maxZValue = item->zValue();
		}
	}

	for (QGraphicsItem* item : selectedItems) {
		item->setZValue(maxZValue + 1);
		group->addToGroup(item);
	}

	group->setFlag(QGraphicsItem::ItemIsMovable);
	group->setFlag(QGraphicsItem::ItemIsSelectable);
	group->setFlag(QGraphicsItem::ItemIsFocusable);

	for (QGraphicsItem* item : selectedItems) {
		scene_->removeItem(item);
	}
	scene_->addItem(group);
	group->setSelected(true);

	selectedGroup = group;
}


void SceneEditWidget::on_startMotionButton_clicked()
{
	if (selectedItem || selectedGroup) {
		QStringList directions = {"Up", "Down", "Left", "Right"};
		bool ok;
		QString direction = QInputDialog::getItem(this, "Select Direction", "Select Direction:", directions, 0, false, &ok);

		if (ok && !direction.isEmpty()) {
			int duration = QInputDialog::getInt(this, "Select Time", "Select Time (milliseconds):", 5000, 1000, 60000, 1000, &ok);
			if (ok) {
				if (direction == "Up") {
					dx = 0;
					dy = -5;
				} else if (direction == "Down") {
					dx = 0;
					dy = 5;
				} else if (direction == "Left") {
					dx = -5;
					dy = 0;
				} else if (direction == "Right") {
					dx = 5;
					dy = 0;
				}

				movementDuration = duration;
				movementTimer->start(30);
				QTimer::singleShot(movementDuration, this, &SceneEditWidget::stopMovingItem);
			}
		}
	} else {
		QMessageBox::warning(this, "Error", "Select something!");
	}
}

void SceneEditWidget::updateItemPosition()
{
	QGraphicsItem* item = (selectedGroup) ? static_cast<QGraphicsItem*>(selectedGroup) : selectedItem;

	if (!item) return;

	QRectF itemBounds = item->sceneBoundingRect();
	QRectF sceneBounds = scene_->sceneRect();

	bool collided = false;

		   // Проверка на столкновение с границами сцены
	if (itemBounds.right() >= sceneBounds.right() || itemBounds.left() <= sceneBounds.left()) {
		dx = -dx;
		collided = true;
	}
	if (itemBounds.bottom() >= sceneBounds.bottom() || itemBounds.top() <= sceneBounds.top()) {
		dy = -dy;
		collided = true;
	}

	QList<QGraphicsItem *> collidingItems = item->collidingItems();
	foreach (QGraphicsItem* collidingItem, collidingItems) {
		if (selectedGroup && selectedGroup->isAncestorOf(collidingItem)) {
			continue;
		}

		if (dynamic_cast<QGraphicsPixmapItem*>(collidingItem) ||
			collidingItem->type() == QGraphicsRectItem::Type) {
			dx = -dx;
			dy = -dy;
			collided = true;
			break;
		}
	}

	if (collided) {
		collisionSound.play();
	}
	item->moveBy(dx, dy);
}

void SceneEditWidget::stopMovingItem()
{
	movementTimer->stop();
}

void SceneEditWidget::on_selectButton_clicked()
{
	paintWidget_->setCurrentTool(ToolType::NoTool);
	ui->eraserButton->setChecked(false);
	ui->brushButton->setChecked(false);
	ui->selectButton->setChecked(true);
}


void SceneEditWidget::on_changeBackground_clicked()
{
	QColor color = QColorDialog::getColor(QColorConstants::Svg::white, this, "Background color");
	if (color.isValid()) {
		scene_->setBackgroundBrush(color);
	}
}

