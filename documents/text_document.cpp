#include "text_document.h"

void TextDocument::load(QString &inputString)
{
    text_ = std::move(inputString);
}

void TextDocument::unload(QString &inputString)
{
    inputString = std::move(text_);
}

QString &TextDocument::getContent()
{
    return text_;
}
TextDocument::TextDocument(QString &inputString) {
    load(inputString);
}
