#include "FastIsType.h"

#include <QJsonDocument>
#include <QFile>
#include <QVector>
#include <QBitArray>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "AutogenMetadata.h"

QVector<QBitArray>* cast_table = nullptr;

class FastTypeHelper
{
public:
    FastTypeHelper()
    {
        static bool inited = false;
        if (!inited)
        {
            cast_table = new QVector<QBitArray>;

            QFile json_file("metadata.json");
            json_file.open(QIODevice::ReadOnly);
            QByteArray json_raw = json_file.readAll();
            parsed_json_ = QJsonDocument::fromJson(json_raw);
            classes_data_ = parsed_json_.object().value("classes").toArray();

            InitTable();
            inited = true;
        }
    }
private:
    bool IsBase(int class_num, int base_num)
    {
        if (base_num == 0)
        {
            return true;
        }

        if (class_num == base_num)
        {
            return true;
        }

        if (class_num == 0)
        {
            return false;
        }

        QString base_name = GetBaseNameOfClass(class_num);
        if (base_name == "IMainObject")
        {
            return false;
        }

        return IsBase(GetIndexOfClass(base_name), base_num);
    }

    QString GetNameOfClass(int index)
    {
        return classes_data_[index - 1].toObject().value("class").toString();
    }

    QString GetBaseNameOfClass(int index)
    {
        return classes_data_[index - 1].toObject().value("base_class").toString();
    }

    int GetIndexOfClass(const QString& class_name)
    {
        for (int i = 0; i < classes_data_.size(); ++i)
        {
            if (classes_data_[i].toObject().value("class") == class_name)
            {
                return i + 1;
            }
        }
        return -1;
    }

    void InitTable()
    {
        int amount_of_classes = classes_data_.size() + 1;

        cast_table->resize(amount_of_classes);
        for (int i = 0; i < cast_table->size(); ++i)
        {
            (*cast_table)[i].fill(false, amount_of_classes);
        }

        for (int i = 0; i < amount_of_classes; ++i)
        {
            for (int j = 0; j < amount_of_classes; ++j)
            {
                (*cast_table)[i].setBit(j, IsBase(i, j));
            }
        }

    }
    QJsonArray classes_data_;
    QJsonDocument parsed_json_;
};

void InitCastTable()
{
    const static FastTypeHelper helper;
}
