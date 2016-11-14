#include "PressureIndicator.h"

#include "../Tile.h"

PressureIndicator::PressureIndicator(quint32 id) : IOnMapObject(id)
{
    v_level = 11;
    name = "Pressure indicator";
    SetSprite("icons/numbers.dmi");
    SetState("empty");
}

void PressureIndicator::Process()
{
    if (IdPtr<CubeTile> tile = GetRoot())
    {
        AtmosHolder* holder = tile->GetAtmosHolder();
        SetNumber(holder->GetPressure() / 1000);
    }
}

void PressureIndicator::AfterWorldCreation()
{
    IOnMapObject::AfterWorldCreation();
    SetFreq(1);
}

void PressureIndicator::SetNumber(int number)
{
    if (number >= 100)
    {
        number = 99;
    }
    if (number < 0)
    {
        number = 0;
    }

    static const QString NUMBERS[10] =
        { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    static const QString SPRITE = "icons/numbers.dmi";

    int first_digit = number / 10;
    int second_digit = number % 10;

    view_.RemoveOverlays();
    view_.AddOverlay(SPRITE, NUMBERS[first_digit]).SetShift(-7, 0);
    view_.AddOverlay(SPRITE, NUMBERS[second_digit]).SetShift(7, 0);
}
