#ifndef TEXT_DOCUMENT_H
#define TEXT_DOCUMENT_H
#include "document.h"
#include <QTextEdit>

class TextDocument : public Document {
    Q_OBJECT

public:
    explicit TextDocument(QTextEdit *textEdit, QObject *parent = nullptr) : Document(parent), textEdit(textEdit) {}

    void create() override {
        textEdit->clear();
    }

    void open(const QString &fileName) override {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            textEdit->setPlainText(in.readAll());
            file.close();
        }
    }

    void save(const QString &fileName) override {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << textEdit->toPlainText();
            file.close();
        }
    }

    void clear() override {
        tempContent = textEdit->toPlainText();
        textEdit->clear();
    }

    void undoClear() override {
        textEdit->setPlainText(tempContent);
    }

    void setContent(const QString &content) override {
        textEdit->setPlainText(content);
    }

    QString getContent() const override {
        return textEdit->toPlainText();
    }

private:
    QTextEdit *textEdit;
};

#endif // TEXT_DOCUMENT_H
