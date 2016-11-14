#include "MainObject.h"
#include "../Map.h"
#include "../ObjectFactory.h"
#include "../Game.h"
#include "representation/Representation.h"

void IMainObject::PlaySoundIfVisible(const QString& name, quint32 tile_id)
{
    if (GetGame().GetMap().IsTileVisible(tile_id))
    {
        GetGame().AddSound(name);
    }
}


void IMainObject::PlayMusic(const QString& name, float volume)
{
    GetGame().PlayMusic(name, volume);
}

void IMainObject::Delete()
{
    GetGame().GetFactory().DeleteLater(id_);
}

bool IMainObject::Save(std::stringstream& file)
{
    file << " " << T_ITEM().toStdString() << " ";
    file << " " << id_ << " ";
    file << " " << how_often_ << " ";
    return true;
}

bool IMainObject::Load(std::stringstream& file)
{
    file >> how_often_;

    SetFreq(GetFreq());

    // It empty becouse all data load main outer function
    return true;
}

void IMainObject::SetFreq(int freq)
{
    if (game_ == nullptr)
    {
        qDebug() << "SetFreq is called in constructor";
        KvAbort();
    }

    how_often_ = freq;

    // TODO: Why is it here?
    if (!GetId())
    {
        qDebug() << "GetId() is zero";
        KvAbort();
    }

    if (how_often_ != 0)
    {
        GetGame().GetFactory().AddProcessingItem(GetId());
    }
}

const IGame& IMainObject::GetGame() const
{
    if (game_ == nullptr)
    {
        qDebug() << "IMainObject::GetGame() is called during construction of object";
        KvAbort();
    }
    return *game_;
}

IObjectFactory& IMainObject::GetFactory()
{
    return GetGame().GetFactory();
}

IGame& IMainObject::GetGame()
{
    if (game_ == nullptr)
    {
        qDebug() << "IMainObject::GetGame() is called during construction of object";
        KvAbort();
    }
    return *game_;
}

unsigned int IMainObject::GetRand()
{
    return GetGame().GetRandom().GetRand();
}

quint32 IMainObject::CreateImpl(const QString& type, quint32 owner)
{
    return GetFactory().CreateImpl(type, owner);
}

void IMainObject::SetId(quint32 id)
{
    id_ = id;
    SetFreq(GetFreq());
}
