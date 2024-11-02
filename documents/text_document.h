#ifndef TEXT_DOCUMENT_H
#define TEXT_DOCUMENT_H
#include <QTextEdit>
#include <qobject.h>

class TextDocument
{

  public:
    explicit TextDocument() = default;

    explicit TextDocument(QString &inputString);

    void load(QString &inputString);
    void unload(QString &inputString);
    QString &getContent();

  private:
    QString text_;
};

#endif // TEXT_DOCUMENT_H
