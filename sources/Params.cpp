#include "Params.h"

void ParamsHolder::ParseParams(int argc, char* argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        QString str = QString(argv[i]);

        int num = str.indexOf('=');
        QString key;
        QString value;
        if (num != -1)
        {
            key = str.left(num);
            value = str.mid(num + 1);
        }
        else
        {
            key = str;
            value = "";
        }
        params_[key] = value;
    }
}

ParamsHolder& GetParamsHolder()
{
    static ParamsHolder* params_holder = new ParamsHolder;
    return *params_holder;
}
