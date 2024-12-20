#include "worktype.h"

const QMap<QVector<QString>, WorkType> fileExtensionMap
{
	{{"txt", "html"}, WorkType::Text},
	{{"csv"}, WorkType::Table},
	{{"json"}, WorkType::InteractiveScene}
};

WorkType getWorktypeByExtension(const QString &fileExtension)
{
	WorkType worktype = WorkType::Unknown;
	for (auto it = fileExtensionMap.begin(); it != fileExtensionMap.end(); ++it)
	{
		if (it.key().contains(fileExtension)) {
			worktype = it.value();
			break;
		}
	}
	return worktype;
}
