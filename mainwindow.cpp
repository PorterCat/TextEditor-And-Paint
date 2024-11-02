#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QInputDialog>
#include <QFontDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QImage>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QSettings>
#include <QCloseEvent>
#include <QDir>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QTimer>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isTempFileEmpty(true)
    , isContentModified(false)
    , currentTool(NoTool)
    , brushSize(5)
    , brushColor(Qt::black)
    , brushStyle(Qt::SolidLine)
    , eraserSize(10)
    , backgroundColor(Qt::white)
    , textFont(QFont("Arial", 12))
    , textColor(Qt::black)
    , selectedItem(nullptr)
    , movementTimer(new QTimer(this))
    , dx(5), dy(5)
    , movementDuration(5000)
    , selectedGroup(nullptr)
{
    ui->setupUi(this);
    loadSettings();

    collisionSound.setSource(QUrl::fromLocalFile(":/sounds/collision.wav"));
    collisionSound.setLoopCount(1);
    collisionSound.setVolume(0.5);
    scene = new QGraphicsScene(0, 0, 800, 600, this);
    ui->graphicsView->setScene(scene);
    scene->setBackgroundBrush(backgroundColor);
    graphicsView = ui->graphicsView;
    graphicsView->setCurrentTool(currentTool);
    graphicsView->setBrushSize(brushSize);
    graphicsView->setBrushColor(brushColor);
    graphicsView->setBrushStyle(brushStyle);
    graphicsView->setEraserSize(eraserSize);
    graphicsView->setBackgroundColor(backgroundColor);

    connect(graphicsView, &CustomGraphicsView::toolChanged, this, &MainWindow::onToolChanged);

    connect(ui->scaleSlider, &QSlider::valueChanged, this, &MainWindow::on_scaleSlider_valueChanged);

    connect(ui->rotateSlider, &QSlider::valueChanged, this, &MainWindow::on_rotateSlider_valueChanged);

    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::on_scene_selectionChanged);

    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::textChanged);

    connect(ui->clearCanvasButton, &QPushButton::clicked, this, &MainWindow::on_clearCanvasButton_clicked);

    connect(movementTimer, &QTimer::timeout, this, &MainWindow::updateItemPosition);

    connect(graphicsView, &CustomGraphicsView::itemDragStarted, this, &MainWindow::disableCurrentTool);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::on_tabWidget_currentChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::disableCurrentTool()
{
    if (currentTool == BrushTool || currentTool == EraserTool) {
        currentTool = NoTool;
        graphicsView->setCurrentTool(currentTool);
        qDebug() << "Current tool disabled due to item dragging.";
    }
}


void MainWindow::onToolChanged(ToolType newTool)
{
    currentTool = newTool;
    if (currentTool != BrushTool) {
        ui->brushButton->setChecked(false);
    }
    if (currentTool != EraserTool) {
        ui->eraserButton->setChecked(false);
    }
}

void MainWindow::playCollisionSound()
{
    if (collisionSound.isLoaded()) {
        collisionSound.play();
    }
}

void MainWindow::on_rotateSlider_valueChanged(int value)
{

    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    if (selectedItems.isEmpty()) return;

    foreach (QGraphicsItem *item, selectedItems) {
        QRectF boundingRect = item->boundingRect();
        QPointF center = boundingRect.center();

        item->setTransformOriginPoint(center);

        item->setRotation(static_cast<qreal>(value));
    }
}

void MainWindow::startMovingItem()
{

    if (selectedItem || selectedGroup) {
        QStringList directions = {"Вверх", "Вниз", "Влево", "Вправо"};
        bool ok;
        QString direction = QInputDialog::getItem(this, "Выбор направления", "Выберите направление:", directions, 0, false, &ok);

        if (ok && !direction.isEmpty()) {
            int duration = QInputDialog::getInt(this, "Выбор времени", "Введите время движения в миллисекундах:", 5000, 1000, 60000, 1000, &ok);
            if (ok) {
                if (direction == "Вверх") {
                    dx = 0;
                    dy = -5;
                } else if (direction == "Вниз") {
                    dx = 0;
                    dy = 5;
                } else if (direction == "Влево") {
                    dx = -5;
                    dy = 0;
                } else if (direction == "Вправо") {
                    dx = 5;
                    dy = 0;
                }

                movementDuration = duration;
                movementTimer->start(30);
                QTimer::singleShot(movementDuration, this, &MainWindow::stopMovingItem);
            }
        }
    } else {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите фигуру или группу для движения.");
    }
}

void MainWindow::stopMovingItem()
{
    movementTimer->stop();
}

void MainWindow::updateItemPosition()
{
    QGraphicsItem* item = (selectedGroup) ? static_cast<QGraphicsItem*>(selectedGroup) : selectedItem;

    if (!item) return;

    QRectF itemBounds = item->sceneBoundingRect();
    QRectF sceneBounds = scene->sceneRect();

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

    // Проверка столкновений с внешними элементами
    QList<QGraphicsItem *> collidingItems = item->collidingItems();
    foreach (QGraphicsItem* collidingItem, collidingItems) {
        // Проверяем, что collidingItem не является частью объединенной группы
        if (selectedGroup && selectedGroup->isAncestorOf(collidingItem)) {
            continue;  // Пропускаем элементы внутри объединенной группы
        }

        // Если сталкивается с элементом, не входящим в группу, меняем направление движения
        if (dynamic_cast<QGraphicsPixmapItem*>(collidingItem) ||
            collidingItem->type() == QGraphicsRectItem::Type) {
            dx = -dx;
            dy = -dy;
            collided = true;
            break;  // Прекращаем проверку на другие столкновения, так как уже обнаружено
        }
    }

    // Если было столкновение, проигрываем звук
    if (collided) {
        playCollisionSound();
    }

    // Движение элемента
    item->moveBy(dx, dy);
}


void MainWindow::on_mergeShapesButton_clicked()
{
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();

    if (selectedItems.size() < 2) {
        QMessageBox::information(this, tr("Объединение фигур"), tr("Выберите как минимум две фигуры для объединения."));
        return;
    }

    // Создаём новую группу
    QGraphicsItemGroup* group = new QGraphicsItemGroup();

    // Определяем максимальный z-value для выбранных элементов, чтобы установить группу сверху
    qreal maxZValue = 0;
    for (QGraphicsItem* item : selectedItems) {
        if (item->zValue() > maxZValue) {
            maxZValue = item->zValue();
        }
    }

    // Добавляем каждый элемент в группу и устанавливаем для группы более высокий z-value
    for (QGraphicsItem* item : selectedItems) {
        item->setZValue(maxZValue + 1);
        group->addToGroup(item);
    }

    group->setFlag(QGraphicsItem::ItemIsMovable);
    group->setFlag(QGraphicsItem::ItemIsSelectable);
    group->setFlag(QGraphicsItem::ItemIsFocusable);

    // Удаляем элементы из сцены и добавляем группу
    for (QGraphicsItem* item : selectedItems) {
        scene->removeItem(item);
    }
    scene->addItem(group);

    // Установка обводки выделения для группы
    group->setSelected(true);  // Помечаем группу как выделенную, чтобы обводка появлялась вокруг всей объединённой фигуры

    selectedGroup = group;
}

void MainWindow::on_scaleSlider_valueChanged(int value)
{
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    if (selectedItems.isEmpty()) return;

    double scaleFactor = value / 100.0;
    foreach (QGraphicsItem *item, selectedItems) {
        item->setScale(scaleFactor);
    }
}

void MainWindow::on_brushButton_clicked()
{
    currentTool = BrushTool;
    ui->graphicsView->setCurrentTool(currentTool);
    ui->brushButton->setChecked(true);
    ui->eraserButton->setChecked(false);

    bool ok;
    int size = QInputDialog::getInt(this, "Размер кисти", "Введите размер кисти:", brushSize, 1, 100, 1, &ok);
    if (ok) {
        brushSize = size;
        ui->graphicsView->setBrushSize(brushSize);
        ui->graphicsView->setBrushStyle(brushStyle);
    } else {
        return;
    }

    QColor color = QColorDialog::getColor(brushColor, this, "Выберите цвет кисти");
    if (color.isValid()) {
        brushColor = color;
        ui->graphicsView->setBrushColor(brushColor);
    }
}

void MainWindow::on_eraserButton_clicked()
{
    currentTool = EraserTool;
    ui->graphicsView->setCurrentTool(currentTool);
    ui->eraserButton->setChecked(true);
    ui->brushButton->setChecked(false);

    bool ok;
    int size = QInputDialog::getInt(this, "Размер ластика", "Введите размер ластика:", eraserSize, 1, 100, 1, &ok);
    if (ok) {
        eraserSize = size;
        ui->graphicsView->setEraserSize(eraserSize);
    }
}


void MainWindow::on_clearCanvasButton_clicked()
{
    scene->clear();
    scene->setBackgroundBrush(backgroundColor);
}

void MainWindow::on_addTextButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Добавить текст", "Введите текст:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        QFont font = QFontDialog::getFont(&ok, textFont, this);
        if (ok) {
            textFont = font;
            QGraphicsTextItem* textItem = scene->addText(text, font);
            textItem->setDefaultTextColor(textColor);
            textItem->setFlag(QGraphicsItem::ItemIsMovable);
            textItem->setFlag(QGraphicsItem::ItemIsSelectable);
        }
    }
}


void MainWindow::on_addShapeButton_clicked()
{
    QStringList shapes = {"Прямоугольник", "Эллипс", "Линия"};
    bool ok;
    QString shape = QInputDialog::getItem(this, "Выбор фигуры", "Выберите фигуру:", shapes, 0, false, &ok);
    if (ok && !shape.isEmpty()) {
        int width = QInputDialog::getInt(this, "Ширина", "Введите ширину:", 100, 1, 1000, 1, &ok);
        if (!ok) return;
        int height = QInputDialog::getInt(this, "Высота", "Введите высоту:", 100, 1, 1000, 1, &ok);
        if (!ok) return;

        // Выбор цветов
        QColor fillColor = QColorDialog::getColor(Qt::white, this, "Выберите цвет заливки");
        if (!fillColor.isValid()) return;
        QColor outlineColor = QColorDialog::getColor(Qt::black, this, "Выберите цвет обводки");
        if (!outlineColor.isValid()) return;

        QPen pen(outlineColor);
        QBrush brush(fillColor);

        QGraphicsItem *item = nullptr;

        if (shape == "Прямоугольник") {
            item = scene->addRect(0, 0, width, height, pen, brush);
        } else if (shape == "Эллипс") {
            item = scene->addEllipse(0, 0, width, height, pen, brush);
        } else if (shape == "Линия") {
            int lineWidth = QInputDialog::getInt(this, "Толщина линии", "Введите толщину линии:", pen.width(), 1, 100, 1, &ok);
            if (!ok) return;

            pen.setWidth(lineWidth);

            item = scene->addLine(0, 0, width, height, pen);
        }

        if (item) {
            item->setFlag(QGraphicsItem::ItemIsMovable);
            item->setFlag(QGraphicsItem::ItemIsSelectable);
            item->setFlag(QGraphicsItem::ItemIsFocusable);
        }
    }
}

void MainWindow::on_deleteShapeButton_clicked()
{
    QList<QGraphicsItem*> selectedItems = scene->selectedItems();
    for (QGraphicsItem* item : selectedItems) {
        scene->removeItem(item);
        delete item;
    }
}

void MainWindow::on_saveGraphicsButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "PNG Image (*.png);;JPEG Image (*.jpg);;BMP Image (*.bmp)");
    if (!fileName.isEmpty()) {
        QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        scene->render(&painter);
        image.save(fileName);
    }
}

void MainWindow::on_changeBackgroundButton_clicked()
{
    QColor color = QColorDialog::getColor(backgroundColor, this, "Выберите цвет фона");
    if (color.isValid()) {
        backgroundColor = color;
        scene->setBackgroundBrush(color);
    }
}

void MainWindow::on_scene_selectionChanged()
{
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    if (!selectedItems.isEmpty()) {
        if (selectedItems.size() == 1 && selectedItems.first()->type() == QGraphicsItemGroup::Type) {
            selectedGroup = qgraphicsitem_cast<QGraphicsItemGroup*>(selectedItems.first());
            selectedItem = nullptr;
        } else {
            selectedItem = selectedItems.first();
            selectedGroup = nullptr;
        }
    } else {
        selectedItem = nullptr;
        selectedGroup = nullptr;
    }
}

void MainWindow::textChanged()
{
    isContentModified = true;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть файл"), "", tr("HTML Files (*.html);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не могу открыть файл: %1").arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QString htmlContent = in.readAll();
        ui->textEdit->setHtml(htmlContent);
        file.close();
        ui->actionReturn->setEnabled(false);
        isContentModified = false;
    }
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить файл"), "", tr("HTML Files (*.html);;All Files (*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не могу сохранить файл: %1").arg(file.errorString()));
            return;
        }
        QTextStream out(&file);
        out << ui->textEdit->toHtml();
        file.close();
        lastFilePath = fileName;
        isContentModified = false;
    }
}

void MainWindow::on_actionClear_triggered()
{
    saveToTempFile();
    ui->textEdit->clear();
    ui->actionReturn->setEnabled(true);
    isContentModified = false;
}

void MainWindow::on_actionReturn_triggered()
{
    if (isTempFileEmpty) {
        QMessageBox::warning(this, tr("Предупреждение"), tr("Нет текста для восстановления."));
        return;
    }

    QFile file(tempFile.fileName());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не могу открыть временный файл: %1").arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    QString htmlContent = in.readAll();
    ui->textEdit->setHtml(htmlContent);
    file.close();
}

void MainWindow::on_actionChangeFont_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, ui->textEdit->currentFont(), this, "Выберите шрифт");
    if (ok) {
        ui->textEdit->setCurrentFont(font);
        isContentModified = true;
    }
}

void MainWindow::on_actionChangeColor_triggered()
{
    QColor color = QColorDialog::getColor(ui->textEdit->textColor(), this, "Выберите цвет текста");
    if (color.isValid()) {
        ui->textEdit->setTextColor(color);
        isContentModified = true;
    }
}

void MainWindow::on_actionSearch_triggered()
{
    bool ok;
    QString searchText = QInputDialog::getText(this, "Поиск", "Введите текст для поиска:", QLineEdit::Normal, "", &ok);
    if (ok && !searchText.isEmpty()) {
        if (!ui->textEdit->find(searchText)) {
            QMessageBox::information(this, "Поиск", "Текст не найден.");
        }
    }
}

void MainWindow::on_actionReplace_triggered()
{
    bool ok;
    QString searchText = QInputDialog::getText(this, "Замена", "Введите текст для поиска:", QLineEdit::Normal, "", &ok);
    if (!ok || searchText.isEmpty()) return;

    QString replaceText = QInputDialog::getText(this, "Замена", "Введите текст для замены:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString content = ui->textEdit->toPlainText();
    content.replace(searchText, replaceText);
    ui->textEdit->setPlainText(content);
    isContentModified = true;
}

void MainWindow::on_actionCreateTable_triggered()
{
    bool ok;
    int rows = QInputDialog::getInt(this, tr("Количество строк"), tr("Введите количество строк:"), 1, 1, 100, 1, &ok);
    if (!ok) return;

    int columns = QInputDialog::getInt(this, tr("Количество столбцов"), tr("Введите количество столбцов:"), 1, 1, 100, 1, &ok);
    if (!ok) return;

    QString html = "<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\">";
    for (int i = 0; i < rows; ++i) {
        html += "<tr>";
        for (int j = 0; j < columns; ++j) {
            html += "<td>&nbsp;</td>";
        }
        html += "</tr>";
    }
    html += "</table>";

    ui->textEdit->insertHtml(html);
    isContentModified = true;
}

void MainWindow::saveToTempFile()
{
    if (!tempFile.isOpen()) {
        tempFile.setFileName(QDir::temp().filePath("lab5_temp.html"));
    }

    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не могу записать во временный файл: %1").arg(tempFile.errorString()));
        return;
    }
    QTextStream out(&tempFile);
    out << ui->textEdit->toHtml();
    tempFile.close();
    isTempFileEmpty = false;
}

void MainWindow::loadSettings()
{
    QSettings settings("MyCompany", "MyApp");
    lastFilePath = settings.value("lastFilePath", "").toString();
}

void MainWindow::saveSettings()
{
    QSettings settings("MyCompany", "MyApp");
    settings.setValue("lastFilePath", lastFilePath);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isContentModified) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Сохранить изменения", "Хотите сохранить изменения?",
                                      QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Save) {
            on_actionSave_triggered();
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    saveSettings();
    event->accept();
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == ui->tabWidget->indexOf(ui->textEditorTab)) {
    } else if (index == ui->tabWidget->indexOf(ui->graphicsEditorTab)) {
    }
}

void MainWindow::on_loadImageButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Загрузить изображение"), "", tr("Images (*.png *.jpg *.bmp)"));
    if (!fileName.isEmpty()) {
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            QGraphicsPixmapItem *pixmapItem = scene->addPixmap(pixmap);
            pixmapItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось загрузить изображение."));
        }
    }
}

void MainWindow::on_moveItemButton_clicked()
{
    startMovingItem();
}
