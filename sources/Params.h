#pragma once

#include <QDebug>
#include <QString>

#include <map>
#include <string>
#include <sstream>

#include "core/Constheader.h"

class ParamsHolder
{
public:
    void ParseParams(int argc, char* argv[]);

    template<class T>
    T GetParam(QString name)
    {
        if (params_.find(name) == params_.end())
        {
            qDebug() << "Cannot find param: " << name;
            return T();
        }
        std::stringstream converter;
        converter << params_[name].toStdString();
        T retval = T();
        converter >> retval;
        return retval;
    }

    bool GetParamBool(QString name)
    {
        if (params_.find(name) == params_.end())
        {
            return false;
        }
        return true;
    }
private:
    std::map<QString, QString> params_; 
};

template<>
inline QString ParamsHolder::GetParam<QString>(QString name)
{
    if (params_.find(name) == params_.end())
    {
        qDebug() << "Cannot find param: " << name;
        return QString();
    }
    std::stringstream converter;
    converter << params_[name].toStdString();
    std::string retval = std::string();
    converter >> retval;
    return QString::fromStdString(retval);
}

ParamsHolder& GetParamsHolder();
