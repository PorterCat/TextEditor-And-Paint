#ifndef WORKTYPE_H
#define WORKTYPE_H

#include <QString>
#include <QMap>

enum WorkType
{
	Unknown,
	Text,
	Table,
	InteractiveScene,
};

WorkType getWorktypeByExtension(const QString &fileExtension);

#endif // WORKTYPE_H
