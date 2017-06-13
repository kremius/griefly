#include "GasTank.h"

#include "Tile.h"
#include "representation/Chat.h"
#include "../Game.h"

#include "ElectricTools.h"

using namespace kv;

namespace
{
    const int O2_TANK_AMOUNT = 10000;
    const int O2_TANK_ENERGY = 5000;

    const int PLASMA_TANK_AMOUNT = 10000;
}

GasTank::GasTank()
{
    name = "Oxygen tank";

    v_level = 5;

    SetPassable(Dir::ALL, passable::AIR);

    SetSprite("icons/atmos.dmi");
    SetState("blue");

    open_ = false;
    atmos_holder_.AddGase(atmos::OXYGEN, O2_TANK_AMOUNT);
    atmos_holder_.AddEnergy(O2_TANK_ENERGY);
    // atmos_holder_.SetVolume();
}

void GasTank::AfterWorldCreation()
{
    Movable::AfterWorldCreation();
    SetFreq(1);
}

void GasTank::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<AtmosTool> tool = item)
    {
        PostHtmlFor(AtmosTool::GetHtmlInfo(atmos_holder_), tool->GetOwner());
        return;
    }

    if (item)
    {
        return;
    }

    if (open_)
    {
        Close();
    }
    else
    {
        Open();
    }

}

void GasTank::Open()
{
    PostVisible(name + " is open", GetPosition());

    open_ = true;
}

void GasTank::Close()
{
    PostVisible(name + " is closed", GetPosition());

    open_ = false;
}

void GasTank::Process()
{
    if (!open_)
    {
        return;
    }
    if (IdPtr<CubeTile> tile = GetOwner())
    {
        atmos_holder_.Connect(tile->GetAtmosHolder());
    }
}

MagicGasTank::MagicGasTank()
{
    name = "Magic tank";
}

void MagicGasTank::AfterWorldCreation()
{
    GasTank::AfterWorldCreation();
    SetFreq(10);
}

void MagicGasTank::Process()
{
    GasTank::Process();

    GetAtmosHolder()->RemoveGase(atmos::CO2, GetAtmosHolder()->GetGase(atmos::CO2));
    GetAtmosHolder()->RemoveGase(atmos::NYTROGEN, GetAtmosHolder()->GetGase(atmos::NYTROGEN));

    int new_oxygen = qMax(0, O2_TANK_AMOUNT - GetAtmosHolder()->GetGase(atmos::OXYGEN));
    GetAtmosHolder()->AddGase(atmos::OXYGEN, new_oxygen);

    int new_energy = qMax(0, O2_TANK_ENERGY - GetAtmosHolder()->GetEnergy());
    GetAtmosHolder()->AddEnergy(new_energy);
}

PlasmaGasTank::PlasmaGasTank()
{
    name = "Plasma tank";
    SetState("orange");

    GetAtmosHolder()->RemoveGase(atmos::OXYGEN, O2_TANK_AMOUNT);
    GetAtmosHolder()->AddGase(atmos::PLASMA, PLASMA_TANK_AMOUNT);
}
