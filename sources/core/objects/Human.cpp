#include "Human.h"

#include "representation/Text.h"
#include "../ObjectFactory.h"
#include "../Names.h"
#include "representation/Sound.h"
#include "Shard.h"
#include "net/MagicStrings.h"
#include "representation/Chat.h"
#include "../atmos/AtmosHolder.h"
#include "Tile.h"
#include "../Helpers.h"
#include "../SyncRandom.h"
#include "Ghost.h"
#include "Clothes.h"
#include "Floor.h"
#include "net/NetworkMessagesTypes.h"
#include "Lobby.h"
#include "../Game.h"
#include "Gun.h"
#include "Weapons.h"
#include "ElectricTools.h"
#include "SpaceTurf.h"
#include "Drinks.h"
#include "MedbayTools.h"

using namespace kv;

QVector<Position> Human::GetHeardPoints() const
{
    QVector<Position> retval;
    CalculateVisible(&retval);
    return retval;
}

void Human::Hear(const Phrase& phrase)
{
    if (IsMinded())
    {
        GetGame().GetChatFrameInfo().PostPersonal(
            QString("<b>%1</b> <i>says</i>, <span>\"%2\"</span>").arg(phrase.from).arg(phrase.text),
            GetGame().GetNetId(GetId()));
    }
}

Human::Human()
{
    tick_speed_ = 1;
    SetSprite("icons/human.dmi");
    SetState("african1_m_s");
    SetPassable(Dir::ALL, passable::BIG_ITEM);
    v_level = 9;
    attack_cooldown_ = 0;
    name = "Morgan James";
    passable_level = passable::BIG_CREATURE;

    lay_timer_ = 0;

    dead_ = false;
    lying_ = false;
    max_health_ = HUMAN_MAX_HEALTH;
    suffocation_damage_ = 0;
    burn_damage_ = 0;
    brute_damage_ = 0;

    pulled_object_ = 0;
}

void Human::AfterWorldCreation()
{
    Mob::AfterWorldCreation();

    interface_.InitSlots();
    interface_.SetOwner(GetId());

    /*interface_.uniform_.Set(Create<Item>(JanitorUniform::GetTypeStatic()));
    interface_.feet_.Set(Create<Item>(OrangeBoots::GetTypeStatic()));
    interface_.r_hand_.Set(Create<Item>(Crowbar::GetTypeStatic()));

    interface_.uniform_.Get()->SetOwner(GetId());
    interface_.feet_.Get()->SetOwner(GetId());
    interface_.r_hand_.Get()->SetOwner(GetId());*/

    UpdateOverlays();

    name = GetGame().GetNames().GetMaleName();
}

void Human::MindEnter()
{
}

void Human::MindExit()
{
}
void Human::GenerateInterfaceForFrame()
{
    Mob::GenerateInterfaceForFrame();
    interface_.Draw();
}

bool Human::TryMove(Dir direct)
{
    Vector pos;
    if (pulled_object_)
    {
        if (!CanTouch(pulled_object_))
        {
            StopPull();
        }
        else
        {
            pos.x = GetPosition().x - pulled_object_->GetPosition().x;
            pos.y = GetPosition().y - pulled_object_->GetPosition().y;
        }
    }
    if (Mob::TryMove(direct))
    {   
        if (GetOwner()->GetItem<Shard>().IsValid())
        {
            PlaySoundIfVisible("glass_step.wav");
        }

        TryClownBootsHonk();

        if (pulled_object_ && ((std::abs(pos.x) + std::abs(pos.y)) < 2))
        {
            if (!pulled_object_->TryMove(VDirToDir(pos)))
            {
                StopPull();
            }
        }
        return true;
    }
    return false;
}

void Human::ProcessMessage(const Message2 &msg)
{
    QJsonObject obj = Network2::ParseJson(msg);

    if (   msg.type == MessageType::ORDINARY
        && !lying_
        && friction::CombinedFriction(GetTurf()))
    {
        if (qAbs(force_.x) + qAbs(force_.y) + qAbs(force_.z) < 4)
        {
            if (Network2::IsKey(obj, Input::MOVE_UP))
            {
                ApplyForce(DirToVDir(Dir::UP));
                return;
            }
            else if (Network2::IsKey(obj, Input::MOVE_DOWN))
            {
                ApplyForce(DirToVDir(Dir::DOWN));
                return;
            }
            else if (Network2::IsKey(obj, Input::MOVE_LEFT))
            {
                ApplyForce(DirToVDir(Dir::LEFT));
                return;
            }
            else if (Network2::IsKey(obj, Input::MOVE_RIGHT))
            {
                ApplyForce(DirToVDir(Dir::RIGHT));
                return;
            }
        }
    }
    if (msg.type == MessageType::MESSAGE)
    {
        QString text = obj["text"].toString();
        QString prefixes[] = {"me ", "me", "* ", "*"};
        bool found = false;
        for (auto& str : prefixes)
        {
            if (text.startsWith(str))
            {
                quint32 length = str.length();
                text.replace(0, length, "");
                MakeEmote(text);
                found = true;
                break;
            }
        }
        if (!found)
        {
            Phrase phrase;
            phrase.from = name.toHtmlEscaped();
            phrase.text = text.toHtmlEscaped();
            if (!phrase.text.isEmpty())
            {
                phrase.text[0] = phrase.text[0].toUpper();
            }
            GetGame().GetChatFrameInfo().PostHear(phrase, GetPosition());
        }
    }
    else if (msg.type == MessageType::MOUSE_CLICK)
    {
        // TODO: shorter cd when shooting with weapons
        const int ATTACK_CD = 6;
        if ((MAIN_TICK - attack_cooldown_) < ATTACK_CD)
        {
            return;
        }
        attack_cooldown_ = MAIN_TICK;

        IdPtr<MaterialObject> object = Network2::ExtractObjId(obj);
        if (!object.IsValid())
        {
            return;
        }

        QString action = Network2::ExtractAction(obj);
        if (action == Click::LEFT_SHIFT)
        {
            if (IdPtr<Human> human = object)
            {
                PostVisible(
                    name + " looks at " + human->name,
                    GetPosition());
                return;
            }
            PostVisible(
                name + " looks at the " + object->name,
                GetPosition());
            return;
        }
        if (action == Click::LEFT_CONTROL)
        {
            PullAction(object);
            return;
        }
        if (action == Click::LEFT_R)
        {
            RotationAction(object);
            return;
        }
        if (action != Click::LEFT)
        {
            qDebug() << "Unknown action: " << action;
            return;
        }
        if (lying_)
        {
            return;
        }

        if (IdPtr<MapObject> object_owner = object->GetOwner())
        {
            if (CanTouch(object))
            {
                //SYSTEM_STREAM << "And we can touch it!" << std::endl;
                if(!interface_.GetActiveHand().Get())
                {
                    interface_.Pick(object);
                    if (interface_.GetActiveHand().Get())
                    {
                        if (!object_owner->RemoveObject(object))
                        {
                            KvAbort("Unable to RemoveItem from GetOwner() during"
                                    " the pick phase!");
                        }
                        object->SetOwner(GetId());
                    }
                    else
                    {
                        interface_.Pick(0);
                        object->AttackBy(0);
                    }
                }
                else
                {
                    if (GetLying() == false)
                    {
                        object->AttackBy(interface_.GetActiveHand().Get());
                    }
                }
                
            }
            else if (IdPtr<Gun> tool = interface_.GetActiveHand().Get())
            {
                if (GetLying() == false && Gun::Targetable(object))
                {
                    tool->Shoot(tool->TargetTileLoc(object));
                }
            }
            else if (  IdPtr<RemoteAtmosTool> tool
                     = interface_.GetActiveHand().Get())
            {
                IdPtr<CubeTile> tile = object->GetOwner();
                if (!tile)
                {
                    return;
                }
                PostHtmlFor(AtmosTool::GetHtmlInfo(*tile->GetAtmosHolder()), GetId());
            }
        }
    }
    else
    {
        // TODO
        interface_.HandleClick(obj["key"].toString());
    }

}

void Human::UpdateOverlays()
{
    view_.RemoveOverlays();
    interface_.AddOverlays();
}

void Human::Process()
{
    Mob::Process();
    Live();
    if (pulled_object_)
    {
        if (!CanTouch(pulled_object_))
        {
            StopPull();
        }
    }
}

void Human::SetLying(bool value)
{
    if (value == false && lay_timer_ > 0)
    {
        return;
    }

    lying_ = value;
    if (lying_)
    {
        PostVisible(name + " is lying now", GetPosition());
        view_.SetAngle(90);
        SetPassable(Dir::ALL, passable::FULL);
        v_level = 8;
    }
    else
    {
        PostVisible(name + " is standing now!", GetPosition());
        view_.SetAngle(0);
        SetPassable(Dir::ALL, passable::BIG_ITEM);
        v_level = 9;
    }
    interface_.UpdateLaying();
}

void Human::AddLyingTimer(int value)
{
    lay_timer_ += value;
}

void Human::Live()
{
    if (dead_)
    {
        return;
    }

    interface_.UpdateEnvironment();

    if (IdPtr<CubeTile> t = GetOwner())
    {
        unsigned int oxygen = t->GetAtmosHolder()->GetGase(atmos::OXYGEN);
        int temperature = t->GetAtmosHolder()->GetTemperature();
        const int BURNING_THRESHOLD = 3;
        const int MIN_BURN_DAMAGE = 1;
        if (qAbs(REGULAR_TEMPERATURE - temperature) > BURNING_THRESHOLD)
        {
            int damage = qMax(MIN_BURN_DAMAGE,qAbs(REGULAR_TEMPERATURE - temperature));
            ApplyBurnDamage(damage);
        }
        if (oxygen > 0)
        {
            t->GetAtmosHolder()->RemoveGase(atmos::OXYGEN, 1);
            t->GetAtmosHolder()->AddGase(atmos::CO2, 1);
            Regeneration();
        }
        else if (CalculateHealth() >= -1 * HUMAN_MAX_HEALTH)
        {
            ApplySuffocationDamage(100);
            
            if (GetRand() % 5 == 0 && ((MAIN_TICK % 3) == 0))
            {
                MakeEmote("gasps!");
            }
        }
    }

    interface_.UpdateHealth();

    if (lay_timer_ > 0)
    {
        --lay_timer_;
    }

    if (CalculateHealth() < 0)
    {
        if (!lying_)
        {
            SetLying(true);
        }
        if (CalculateHealth() >= -1 * HUMAN_MAX_HEALTH)
        {
            ApplySuffocationDamage(100);
            if (GetRand() % 4 == 0 && ((MAIN_TICK % 4) == 0))
            {
                MakeEmote("gasps!");
            }
        }
    }
    if (CalculateHealth() < -1 * HUMAN_MAX_HEALTH && !dead_)
    {
        OnDeath();
    }
}

void Human::Regeneration()
{
    if (GetSuffocationDamage() <= 0)
    {
        return;
    }
    ApplySuffocationDamage(-1 * 50);
}

IdPtr<MapObject> Human::GetNeighbour(Dir) const
{
    return GetId();
}

void Human::OnDeath()
{
    if (IsMinded())
    {
        auto ghost = Create<Ghost>(Ghost::GetTypeStatic(), GetOwner());
        ghost->name = name;

        MoveMindTo(ghost);
    }
    dead_ = true;
    SetFreq(0);
}

void Human::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<Drinks> drink = item)
    {
        drink->Drink(GetId(), item->GetOwner());
        return;
    }
    if (IdPtr<HealthAnalyzer> analyzer = item)
    {
        analyzer->Scan(GetId());
        return;
    }
    if (IdPtr<Medicine> medicine = item)
    {
        medicine->Heal(GetId());
        return;
    }
    bool damaged = false;
    if (item.IsValid() && (item->damage > 0))
    {
        ApplyBruteDamage(item->damage * 100);
        QString sound = QString("genhit%1.wav").arg(GetRand() % 3 + 1);
        PlaySoundIfVisible(sound);
        if (IdPtr<MaterialObject> item_owner = item->GetOwner())
        {
            PostVisible(
                QString("<font color=\"red\">%1 is attacked by %2 with %3</font>")
                    .arg(name).arg(item_owner->name).arg(item->name),
                GetPosition());
        }

        damaged = true;
    }
    else if (!item.IsValid())
    {
        ApplyBruteDamage(100);

        unsigned int punch_value = (GetRand() % 4) + 1;
        PlaySoundIfVisible(QString("punch%1.wav").arg(punch_value));

        if (GetRand() % 5 == 0)
        {
            SetLying(true);
            AddLyingTimer(100);
            PostVisible(name + " has been knocked out!", GetPosition());
        }

        damaged = true;
    }

    if (!damaged)
    {
        return;
    }
}

void Human::Represent()
{
    Representation::Entity ent;
    ent.id = GetId();
    ent.click_id = GetId();
    ent.pos_x = GetPosition().x;
    ent.pos_y = GetPosition().y;
    ent.vlevel = v_level;
    ent.view = *GetView();
    if (!lying_)
    {
        ent.dir = GetDir();
    }
    else
    {
        ent.dir = Dir::DOWN;
    }
    GetRepresentation().AddToNewFrame(ent);
}

void Human::CalculateVisible(QVector<Position>* visible_list) const
{
    if (CalculateHealth() >= 0)
    {
        GetGame().GetMap().CalculateLos(
            visible_list,
            GetPosition().x,
            GetPosition().y,
            GetPosition().z);
    }
}

void Human::Bump(IdPtr<Movable> item)
{
    if (IdPtr<Projectile> projectile = item)
    {
        ApplyBruteDamage(projectile->GetDamage() * 100);
        ApplyBurnDamage(projectile->GetBurnDamage() * 100);
        PostVisible(
            name + " got hit by a " + projectile->name + "!", GetPosition());

        // TODO (?): sound
        return;
    }
    Movable::Bump(item);
}

void Human::RotationAction(IdPtr<MapObject> item)
{
    if (IdPtr<Movable> movable = item)
    {
        if (!movable->anchored_)
        {
            if (IdPtr<Projectile> projectile = movable)
            {
                return;
            }
            movable->Rotate(static_cast<Dir>((static_cast<int>(movable->GetDir()) + 1) % 4));
        }
    }
}

void Human::PullAction(IdPtr<MapObject> item)
{
    if (IdPtr<Movable> movable = item)
    {
        if (CanTouch(movable))
        {
            if (IdPtr<Projectile> projectile = movable)
            {
                return;
            }
            pulled_object_ = movable;
            interface_.UpdatePulling(true);
        }
    }
}

void Human::StopPull()
{
    pulled_object_ = 0;
    interface_.UpdatePulling(false);
}

void Human::TryClownBootsHonk()
{
    if (lying_)
    {
        return;
    }
    if (MAIN_TICK % 3 != 0)
    {
        return;
    }
    IdPtr<ClownBoots> shoes = GetHumanInterface()->feet_.Get();
    if (!shoes.IsValid())
    {
        return;
    }
    if (IdPtr<Space> tile = GetTurf())
    {
        return;
    }
    QString sound = QString("clownstep%1.wav").arg(GetRand() % 2 + 1);
    PlaySoundIfVisible(sound);
}

void Human::MakeEmote(const QString& emote)
{
    GetGame().GetChatFrameInfo().PostVisible(
        QString("<b>%1</b> %2").arg(name.toHtmlEscaped()).arg(emote.toHtmlEscaped()),
        GetPosition());
}

int Human::CalculateHealth() const
{
    return max_health_ - (suffocation_damage_ + burn_damage_ + brute_damage_);
}

void Human::ApplyBurnDamage(int damage)
{
    burn_damage_ += damage;
    burn_damage_ = qMax(0, burn_damage_);
}

void Human::ApplySuffocationDamage(int damage)
{
    suffocation_damage_ += damage;
    suffocation_damage_ = qMax(0, suffocation_damage_);
}

void Human::ApplyBruteDamage(int damage)
{
    brute_damage_ += damage;
    brute_damage_ = qMax(0, brute_damage_);
    if (damage > 0)
    {
        const int ALWAYS_BLOOD_BORDER = 1000;
        int brute_helper = qMax(1, ALWAYS_BLOOD_BORDER - brute_damage_);
        if ((GetRand() % brute_helper) == 1)
        {
            return;
        }
        unsigned int blood_value = (GetRand() % 7) + 1;

        if (IdPtr<Floor> floor = GetTurf())
        {
            if (!floor->bloody)
            {
                floor->GetView()->AddOverlay(
                    "icons/blood.dmi",
                    QString("floor%1").arg(blood_value));
                floor->bloody = true;
            }
        }
    }
}
