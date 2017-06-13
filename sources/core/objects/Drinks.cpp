#include "Drinks.h"

using namespace kv;

Drinks::Drinks()
{
    name = "BaseDrink";
    SetSprite("icons/drinks.dmi");
    full_ = false;
}

void Drinks::Drink(IdPtr<Human> consumer, IdPtr<Human> feeder)
{
    if (!full_)
    {
        full_ = true;
        if (consumer == feeder)
        {
            PostHtmlFor(QString("You swallow a gulp of %1.").arg(name), consumer);
        }
        else
        {
            PostVisible(
                QString("%1 attempts to feed the contents of %2 to %3.")
                    .arg(feeder->name).arg(name).arg(consumer->name),
                consumer->GetPosition());
        }
        PlaySoundIfVisible("drink.wav");
        return;
    }
    PostHtmlFor(QString("%1 is empty.").arg(name), consumer);
}

Vodka::Vodka()
{
    name = "Vodka";
    SetState("vodkabottle");
}

Beer::Beer()
{
    name = "Beer";
    SetState("beer");
}

Tea::Tea()
{
    name = "Tea";
    SetState("tea");
}
