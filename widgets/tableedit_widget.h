#ifndef TABLEEDITWIDGET_H
#define TABLEEDITWIDGET_H

#include <QWidget>
#include <QMenu>

QT_BEGIN_NAMESPACE
namespace Ui { class TableEditWidget; }
QT_END_NAMESPACE

class TableEditWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit TableEditWidget(QWidget *parent = nullptr);
    ~TableEditWidget();

  private slots:
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void addRow();
    void addColumn();
    void removeRow();
    void removeColumn();

  private:
    Ui::TableEditWidget *ui;
    QMenu *contextMenu;
};

#endif // TABLEEDITWIDGET_H
