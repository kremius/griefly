#pragma once

#include "../OnMapObject.h"

class PressureIndicator : public IOnMapObject
{
public:
    DECLARE_SAVED(PressureIndicator, IOnMapObject);
    DECLARE_GET_TYPE_ITEM(PressureIndicator);
    PressureIndicator(quint32 id);
    virtual void Process() override;
    virtual void AfterWorldCreation() override;
private:
    void SetNumber(int number);
};
ADD_TO_TYPELIST(PressureIndicator);
