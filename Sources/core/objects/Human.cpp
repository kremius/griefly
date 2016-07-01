#include "Human.h"

#include "representation/Text.h"
#include "../ObjectFactory.h"
#include "../Names.h"
#include "representation/Sound.h"
#include "Shard.h"
#include "net/MagicStrings.h"
#include "representation/Chat.h"
#include "../AtmosHolder.h"
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


Human::Human(size_t id) : IMob(id)
{
    tickSpeed = 1;
    pixSpeed = 2;
    SetSprite("icons/human.dmi");
    SetState("african1_m_s");
    SetPassable(D_ALL, Passable::BIG_ITEM);
    v_level = 9;
    is_strong_owner = true;
    attack_cooldown_ = 0;
    name = "Morgan James";
    interface_.InitSlots();
    interface_.SetOwner(GetId());

    lay_timer_ = 0;

    dead_ = false;
    lying_ = false;
    health_ = 100;
}

void Human::AfterWorldCreation()
{
    IMob::AfterWorldCreation();

    interface_.uniform_.Set(Create<Item>(JanitorUniform::T_ITEM_S()));
    interface_.feet_.Set(Create<Item>(OrangeBoots::T_ITEM_S()));
    interface_.r_hand_.Set(Create<Item>(LaserGun::T_ITEM_S()));

    interface_.uniform_.Get()->SetOwner(GetId());
    interface_.feet_.Get()->SetOwner(GetId());
    interface_.r_hand_.Get()->SetOwner(GetId());

    UpdateOverlays();

    name = GetGame().GetNames().GetMaleName();
}

void Human::InitGUI()
{
}

void Human::DeinitGUI()
{
}
void Human::GenerateInterfaceForFrame()
{
    IMob::GenerateInterfaceForFrame();
    interface_.Draw();
}

bool Human::TryMove(Dir direct)
{
    if (IMob::TryMove(direct))
    {   
        if (owner->GetItem<Shard>().valid())
        {
            PlaySoundIfVisible("glass_step.ogg", GetOwner().ret_id());
        }
        return true;
    }
    return false;
}

void Human::processGUImsg(const Message2 &msg)
{
    QJsonObject obj = Network2::ParseJson(msg);


 //   IMob::processGUImsg(msg);
    if (    msg.type == MessageType::ORDINARY
         && !lying_
         && Friction::CombinedFriction(GetTurf())
        )
    {
        if (std::abs(force_.x) + std::abs(force_.y) + std::abs(force_.z) < 4)
        {
            if (Network2::IsKey(obj, Input::MOVE_UP))
            {
                ApplyForce(DirToVDir[D_UP]);
                return;
            }
            else if (Network2::IsKey(obj, Input::MOVE_DOWN))
            {
                ApplyForce(DirToVDir[D_DOWN]);
                return;
            }
            else if (Network2::IsKey(obj, Input::MOVE_LEFT))
            {
                ApplyForce(DirToVDir[D_LEFT]);
                return;
            }
            else if (Network2::IsKey(obj, Input::MOVE_RIGHT))
            {
                ApplyForce(DirToVDir[D_RIGHT]);
                return;
            }
        }
    }
    if (msg.type == MessageType::MESSAGE)
    {
        std::string text = obj["text"].toString().toStdString();
        GetGame().GetChat().PostWords(name, text, owner.ret_id());
    }
    else if (msg.type == MessageType::MOUSE_CLICK)
    {
        if (lying_)
        {
            return;
        }
        const int ATTACK_CD = 10;
        if ((MAIN_TICK - attack_cooldown_) < ATTACK_CD)
        {
            return;
        }
	//TODO shorter cd when shooting with weapons
        attack_cooldown_ = MAIN_TICK;
        id_ptr_on<IOnMapObject> item = Network2::ExtractObjId(obj);
        if (item && item->GetOwner())
        {
            if (CanTouch(item))
            {
                //SYSTEM_STREAM << "And we can touch it!" << std::endl;
                if(!interface_.GetActiveHand().Get())
                {
                    interface_.Pick(item);
                    if (interface_.GetActiveHand().Get())
                    {
                        if (!item->GetOwner()->RemoveItem(item))
                        {
                            SYSTEM_STREAM << "CANNOT DELETE ITEM WTF" << std::endl;
                        }
                        item->SetOwner(GetId());
                    }
                    else
                    {
                        interface_.Pick(0);
                        item->AttackBy(0);
                    }
                }
                else
                {
                    if (GetLying() == false)
                    {
                        item->AttackBy(interface_.GetActiveHand().Get());
                    }
                }
                
            }
            else if(id_ptr_on<Gun> tool = interface_.GetActiveHand().Get())
            {
		if(GetLying() == false && tool->Targetable(item))
                {
                tool->Shoot(tool->TargetTileLoc(item));
                }
	    }
        }
    }
    else
    {
        // TODO
        interface_.HandleClick(obj["key"].toString().toStdString());
    }

}

void Human::UpdateOverlays()
{
    view_.RemoveOverlays();
    interface_.AddOverlays();
}

void Human::Process()
{
    IMob::Process();
    Live();
}

void Human::SetLying(bool value)
{
    if (value == false && lay_timer_ > 0)
        return;

    lying_ = value;
    if (lying_)
    {
        GetGame().GetChat().PostSimpleText(name + " is lying now", owner->GetId());
        view_.SetAngle(90);
        SetPassable(D_ALL, Passable::FULL);
        v_level = 8;
    }
    else
    {
        GetGame().GetChat().PostSimpleText(name + " is standing now!", owner->GetId());
        view_.SetAngle(0);
        SetPassable(D_ALL, Passable::BIG_ITEM);
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

    if (id_ptr_on<CubeTile> t = owner)
    {
        unsigned int oxygen = t->GetAtmosHolder()->GetGase(OXYGEN);
        if (oxygen > 0)
        {
            t->GetAtmosHolder()->RemoveGase(OXYGEN, 1);
            t->GetAtmosHolder()->AddGase(CO2, 1);
        }
        else if (health_ >= -100)
        {
            --health_;
            
            if (GetRand() % 5 == 0 && ((MAIN_TICK % 3) == 0))
            {
                GetGame().GetChat().PostSimpleText(name + " gasps!", owner->GetId());
            }
        }
    }

    interface_.UpdateHealth();

    if (lay_timer_ > 0)
    {
        --lay_timer_;
    }

    if (health_ < 0)
    {
        if (!lying_)
        {
            SetLying(true);
        }
        if (health_ >= -100)
        {
            --health_;
            if (GetRand() % 4 == 0 && ((MAIN_TICK % 4) == 0))
            {
                GetGame().GetChat().PostSimpleText(name + " gasps!", owner->GetId());
            }
        }
    }
    if (health_ < -100 && !dead_)
    {
        OnDeath();
    }
}

void Human::OnDeath()
{
    size_t net_id = GetGame().GetFactory().GetNetId(GetId());
    if (net_id)
    {
        auto ghost = Create<Ghost>(Ghost::T_ITEM_S());
        ghost->name = name;
        GetGame().GetFactory().SetPlayerId(net_id, ghost.ret_id());
        owner->AddItem(ghost);
        if (GetId() == GetGame().GetMob().ret_id())
        {
            GetGame().ChangeMob(ghost);
        }
    }
    dead_ = true;
    SetFreq(0);

    GetLobby().security_score_ += 1;
}

void Human::AttackBy(id_ptr_on<Item> item)
{
    bool damaged = false;
    if (item.valid() && (item->damage > 0))
    {
        health_ -= item->damage;
        unsigned int value = GetRand() % 3;
        std::string snd;
        if (value == 0)
            snd = "genhit1.ogg";
        if (value == 1)
            snd = "genhit2.ogg";
        if (value == 2)
            snd = "genhit3.ogg";
        PlaySoundIfVisible(snd, owner.ret_id());
        if (id_ptr_on<IOnMapObject> item_owner = item->GetOwner())
        {
            GetGame().GetChat().PostDamage(item_owner->name, name, item->name, owner.ret_id());
        }

        damaged = true;
    }
    else if (!item.valid())
    {
        health_ -= 1;

        unsigned int punch_value = (GetRand() % 4) + 1;
        std::stringstream conv;
        conv << "punch" << punch_value << ".ogg";
        PlaySoundIfVisible(conv.str(), owner.ret_id());

        if (GetRand() % 5 == 0)
        {
            SetLying(true);
            AddLyingTimer(100);
            GetGame().GetChat().PostSimpleText(name + " has been knocked out!", owner->GetId());
        }

        damaged = true;
    }

    if (!damaged)
    {
        return;
    }

    if ((GetRand() % 3) != 0)
    {
        return;
    }

    unsigned int blood_value = (GetRand() % 7) + 1;
    std::stringstream conv;
    conv << "floor" << blood_value;

    if (id_ptr_on<Floor> f = GetTurf())
    {
        if (!f->bloody)
        {
            f->GetView()->AddOverlay("icons/blood.dmi", conv.str());
            f->bloody = true;
        }
    }
}

void Human::Represent()
{
    Representation::Entity ent;
    ent.id = GetId();
    ent.pos_x = GetX();
    ent.pos_y = GetY();
    ent.vlevel = v_level;
    ent.view = *GetView();
    if (!lying_)
    {
        ent.dir = GetDir();
    }
    else
    {
        ent.dir = D_DOWN;
    }
    GetRepresentation().AddToNewFrame(ent);
}

void Human::CalculateVisible(std::list<point>* visible_list)
{
    if (health_ >= 0)
    {
        GetGame().GetMap().CalculateVisisble(visible_list,
            GetX(),
            GetY(),
            GetZ());
    }
}

void Human::Bump(id_ptr_on<IMovable> item)
{
    if(id_ptr_on<Projectile> p = item)
    {
        bool damaged = false;
        if (p.valid())
        {
            health_ -= p->GetDamage();
            if (id_ptr_on<Bullet> b = p)
            {
                 GetGame().GetChat().PostSimpleText(name + " got hit by a bullet!", owner->GetId());
                 unsigned int value = GetRand() % 3;
                 std::string snd;
                 if (value == 0)
                     snd = "genhit1.ogg";
                 if (value == 1)
                     snd = "genhit2.ogg";
                 if (value == 2)
                     snd = "genhit3.ogg";
                 PlaySoundIfVisible(snd, owner.ret_id());
            }
            damaged = true;
        }
        if (!damaged)
        {
            return;
        }

        if ((GetRand() % 3) != 0)
        {
            return;
        }

        unsigned int blood_value = (GetRand() % 7) + 1;
        std::stringstream conv;
        conv << "floor" << blood_value;

        if (id_ptr_on<Floor> f = GetTurf())
        {
            if (!f->bloody)
            {
                f->GetView()->AddOverlay("icons/blood.dmi", conv.str());
                f->bloody = true;
            }
        }
    }
    IMovable::Bump(item);
}

CaucasianHuman::CaucasianHuman(size_t id) : Human(id)
{
    SetState("caucasian2_m_s");
}

void CaucasianHuman::AfterWorldCreation()
{
    // It is not mistake - we don't want to call Human function
    // because it creates some new items
    IMob::AfterWorldCreation();

    interface_.uniform_.Set(Create<Item>(RedUniform::T_ITEM_S()));
    interface_.feet_.Set(Create<Item>(OrangeBoots::T_ITEM_S()));
    interface_.r_hand_.Set(Create<Item>(Revolver::T_ITEM_S())); // wrench
    interface_.head_.Set(Create<Item>(Helmet::T_ITEM_S()));
    interface_.suit_.Set(Create<Item>(Armor::T_ITEM_S()));

    interface_.uniform_.Get()->SetOwner(GetId());
    interface_.feet_.Get()->SetOwner(GetId());
    interface_.r_hand_.Get()->SetOwner(GetId());
    interface_.head_.Get()->SetOwner(GetId());
    interface_.suit_.Get()->SetOwner(GetId());

    UpdateOverlays();

    name = GetGame().GetNames().GetMaleName();
}

void CaucasianHuman::OnDeath()
{
    Human::OnDeath();

    GetLobby().security_score_ -= 1;
    GetLobby().janitors_score_ += 1;
}
