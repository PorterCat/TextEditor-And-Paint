#ifndef TEXTEDIT_WIDGET_H
#define TEXTEDIT_WIDGET_H

#include <QWidget>

namespace Ui {
class TextEditWidget;
}

class TextEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TextEditWidget(QWidget *parent = nullptr);
    ~TextEditWidget();

private:
    Ui::TextEditWidget *ui;
};

#endif // TEXTEDIT_WIDGET_H
