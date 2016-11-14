#pragma once

#include <QString>

#include <string>
#include <vector>

class SyncRandom;
class Names
{
public:
    Names(SyncRandom* random);
    QString GetMaleName();
private:
    void LoadNames();
    std::vector<QString> male_names_;
    std::vector<QString> last_name_;
    SyncRandom* random_;
};
