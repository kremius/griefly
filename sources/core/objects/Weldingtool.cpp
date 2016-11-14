#include "Weldingtool.h"

Weldingtool::Weldingtool(quint32 id) : Item(id)
{
    SetState("welder");

    name = "Weldingtool";

    on_ = false;
    fuel_ = 50;
}

void Weldingtool::Process()
{
    --fuel_;
    if (fuel_ <= 0)
    {
        TurnOff();
        fuel_ = 0;
    }
}

int Weldingtool::AddFuel(int amount)
{
    fuel_ += amount;
    if (fuel_ < 0)
        fuel_ = 0;
    if (fuel_ > 50)
        fuel_ = 50;
    return fuel_;
}

void Weldingtool::TurnOn()
{
    if (Working())
        return;
    if (fuel_ <= 0)
        return;

    on_ = true;
    SetState("welder1");
    SetFreq(10);
}

void Weldingtool::TurnOff()
{
    if (!Working())
        return;
    
    on_ = false;
    SetState("welder");
    SetFreq(0);
}

void Weldingtool::AttackBy(IdPtr<Item> item)
{
    if (Working())
        TurnOff();
    else
        TurnOn();
}
