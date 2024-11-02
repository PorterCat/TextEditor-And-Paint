#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include <QTimer>
#include <QFile>
#include <QSoundEffect>
#include "tooltype.h"
#include "customgraphicsview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слоты для текстового редактора
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionClear_triggered();
    void on_actionReturn_triggered();
    void on_actionChangeFont_triggered();
    void on_actionChangeColor_triggered();
    void on_actionSearch_triggered();
    void on_actionReplace_triggered();
    void on_actionCreateTable_triggered();
    void textChanged();

    // Слоты для графического редактора
    void on_brushButton_clicked();
    void on_eraserButton_clicked();
    void on_addShapeButton_clicked();
    void on_addTextButton_clicked();
    void on_deleteShapeButton_clicked();
    void on_changeBackgroundButton_clicked();
    void on_saveGraphicsButton_clicked();
    void on_clearCanvasButton_clicked();
    void on_moveItemButton_clicked(); // Добавьте это объявление
    void on_mergeShapesButton_clicked();

    // Слоты для ползунков масштабирования и вращения
    void on_scaleSlider_valueChanged(int value);
    void on_rotateSlider_valueChanged(int value);

    // Слот для обновления ползунков при изменении выбора элементов на сцене
    void on_scene_selectionChanged();

    // Движение фигур
    void startMovingItem();
    void updateItemPosition();
    void stopMovingItem();

    // Загрузка изображения
    void on_loadImageButton_clicked(); // Добавьте это объявление

    // Слот для отключения текущего инструмента
    void disableCurrentTool();

    // Добавленные слоты
    void onToolChanged(ToolType newTool);
    void on_tabWidget_currentChanged(int index);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    QString lastFilePath;
    QFile tempFile;
    bool isTempFileEmpty;
    bool isContentModified;

    // Переменные для графического редактора
    QGraphicsScene *scene;
    CustomGraphicsView *graphicsView;
    ToolType currentTool;
    int brushSize;
    QColor brushColor;
    Qt::PenStyle brushStyle;
    int eraserSize;
    QColor backgroundColor;

    // Свойства фигур
    QSize shapeSize;
    QColor shapeFillColor;
    QColor shapeOutlineColor;

    // Свойства текста
    QFont textFont;
    QColor textColor;

    // Для движения
    QGraphicsItem* selectedItem;
    QGraphicsItemGroup* selectedGroup;
    QTimer* movementTimer;
    int dx, dy;
    int movementDuration;

    // Звуковой эффект для столкновений
    QSoundEffect collisionSound;

    // Функция для воспроизведения звука столкновения
    void playCollisionSound();

    void saveToTempFile();
    void loadSettings();
    void saveSettings();
    void setupGraphicsView();
};

#endif // MAINWINDOW_H
