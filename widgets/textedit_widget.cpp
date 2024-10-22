#include "textedit_widget.h"
#include "ui_textedit_widget.h"
#include "widgets/ui_textedit_widget.h"

TextEditWidget::TextEditWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TextEditWidget)
{
    ui->setupUi(this);
}

TextEditWidget::~TextEditWidget()
{
    delete ui;
}
