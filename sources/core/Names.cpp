#include "Names.h"

#include <vector>
#include <fstream>

#include "Game.h"

void Names::LoadNames()
{
    {
        male_names_.clear();
        QFile male_file("names/first_male.txt");
        if (!male_file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "Unable to open" << male_file.fileName();
            KvAbort();
        }
        while (male_file.bytesAvailable())
        {
            QString name = male_file.readLine();
            male_names_.push_back(name);
        }
    }
    {
        last_name_.clear();
        QFile last_file("names/last.txt");
        if (!last_file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "Unable to open" << last_file.fileName();
            KvAbort();
        }
        while (last_file.bytesAvailable())
        {
            QString name = last_file.readLine();
            last_name_.push_back(name);
        }
    }
}

Names::Names(SyncRandom* random)
    : random_(random)
{
    LoadNames();
}

QString Names::GetMaleName()
{
    unsigned int f = random_->GetRand() % male_names_.size();
    unsigned int l = random_->GetRand() % last_name_.size();
    return male_names_[f] + " " + last_name_[l];
}
