#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QFile>
#include <QObject>
#include <QString>
#include <QTextStream>

class Document {
  Q_OBJECT

public:
  explicit Document(QObject *parent = nullptr) {}
  virtual ~Document() {}

  virtual void create() = 0;
  virtual void open(const QString &fileName) = 0;
  virtual void save(const QString &fileName) = 0;
  virtual void clear() = 0;
  virtual void undoClear() = 0;
  virtual void setContent(const QString &content) = 0;
  virtual QString getContent() const = 0;

protected:
  QString tempContent;
};

#endif // DOCUMENT_H
