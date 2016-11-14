#include "Clothes.h"

#include "Slot.h"

Cloth::Cloth(quint32 id) : Item(id)
{
    name = "SimpleCloth";
}

Armor::Armor(quint32 id) : Cloth(id)
{
    SetSprite("icons/suits.dmi");
    SetState("armor");

    type = Slots::SUIT;

    name = "Armor";
}

Helmet::Helmet(quint32 id) : Cloth(id)
{
    SetSprite("icons/hats.dmi");
    SetState("helmet");

    type = Slots::HEAD;

    name = "Armor";
}

EngineUniform::EngineUniform(quint32 id) : Cloth(id)
{
    SetSprite("icons/uniforms.dmi");
    SetState("engine");

    type = Slots::UNIFORM;

    name = "Engine uniform";
}

RedUniform::RedUniform(quint32 id) : Cloth(id)
{
    SetSprite("icons/uniforms.dmi");
    SetState("red");

    type = Slots::UNIFORM;

    name = "Red uniform";
}

JanitorUniform::JanitorUniform(quint32 id) : Cloth(id)
{
    SetSprite("icons/uniforms.dmi");
    SetState("janitor");

    type = Slots::UNIFORM;

    name = "Janitor uniform";
}

ClownUniform::ClownUniform(quint32 id) : Cloth(id)
{
    SetSprite("icons/uniforms.dmi");
    SetState("clown");

    type = Slots::UNIFORM;

    name = "Clown uniform";
}

OrangeBoots::OrangeBoots(quint32 id) : Cloth(id)
{
    SetSprite("icons/shoes.dmi");
    SetState("orange");

    type = Slots::FEET;

    name = "Orange boots";
}

ClownBoots::ClownBoots(quint32 id) : Cloth(id)
{
    SetSprite("icons/shoes.dmi");
    SetState("clown");

    type = Slots::FEET;

    name = "Clown boots";
}
