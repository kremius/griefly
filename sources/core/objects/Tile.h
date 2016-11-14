#pragma once

#include <vector>

#include "Turf.h"
#include "OnMapBase.h"
#include "OnMapObject.h"
#include "../atmos/AtmosHolder.h"

template<class T>
std::iostream& operator<<(std::iostream& file, std::vector<IdPtr<T>>& content)
{
    file << " " << content.size() << " ";
    for (auto it = content.begin(); it != content.end(); ++it)
        file << " " << it->Id() << " ";
    return file;
}

template<class T>
std::iostream& operator>>(std::iostream& file, std::vector<IdPtr<T>>& content)
{
    quint32 size;
    file >> size;
    content.reserve(size);

    unsigned int local_id;
    for (quint32 i = 0; i < size; ++i)
    {
        file >> local_id;
        content.push_back(local_id);
    }
    return file;
}

class IMovable;

class CubeTile: public IOnMapBase
{
public:
    DECLARE_SAVED(CubeTile, IOnMapBase);
    DECLARE_GET_TYPE_ITEM(CubeTile);
    CubeTile(quint32 id);
    virtual bool AddItem(IdPtr<IOnMapBase> item) override;
    virtual bool RemoveItem(IdPtr<IOnMapBase> item) override;
    virtual IdPtr<IOnMapBase> GetNeighbour(Dir direct) const override;
    IdPtr<CubeTile> GetNeighbourImpl(Dir direct) const;
    virtual void ForEach(std::function<void(IdPtr<IOnMapBase>)> callback) override;
    virtual bool IsVisibleByPlayer() const override
    {
        // TODO
        return true;       
    };
    virtual PassableLevel GetPassable(Dir direct) const override;
    virtual bool IsTransparent() const override;

    virtual bool CanTouch(IdPtr<IOnMapBase> item) const override;
    virtual bool Contains(IdPtr<IOnMapBase> item) const override;

    virtual void Bump(IdPtr<IMovable> item) override;
    virtual void BumpByGas(Dir dir, bool inside = false) override;

    void SetPos(int posx, int posy, int posz = 0)
    {
        posx_ = posx;
        posy_ = posy;
        posz_ = posz;
    }
    int posx() const
    {
        return posx_;
    }
    int posy() const
    {
        return posy_;
    }
    int posz() const
    {
        return posz_;
    }
    virtual int GetX() const override
    {
        return posx();
    }
    virtual int GetY() const override
    {
        return posy();
    }
    virtual int GetZ() const override
    {
        return posz();
    }
    virtual IdPtr<ITurf> GetTurf()  override
    {
        return turf_;
    }
    virtual void SetTurf(IdPtr<ITurf> turf) override
    {
        turf_ = turf;
        if (turf_.IsValid())
        {
            turf_->SetOwner(GetId());
        }
        UpdatePassable();
    }
    virtual void UpdatePassable() override;
    AtmosHolder* GetAtmosHolder() { return &atmos_holder_; }

    typedef std::vector<IdPtr<IOnMapObject>> InsideType;
    InsideType& GetInsideList() { return inside_list_; }

    void UpdateAtmosPassable();
protected:
    virtual quint32 GetItemImpl(unsigned int hash) override;
private:
    bool CanTouch(IdPtr<IOnMapBase> item, Dir dir) const;
    bool CanTouch(IdPtr<IOnMapBase> item, Dir first_dir, Dir second_dir) const;

    void MoveToDir(Dir dir, int* x, int* y, int* z = nullptr) const;

    IdPtr<ITurf> KV_SAVEBLE(turf_);

    AtmosHolder KV_SAVEBLE(atmos_holder_);

    int KV_SAVEBLE(posx_);
    int KV_SAVEBLE(posy_);
    int KV_SAVEBLE(posz_);

    InsideType KV_SAVEBLE(inside_list_);

    PassableLevel KV_SAVEBLE(sum_passable_all_);
    PassableLevel KV_SAVEBLE(sum_passable_up_);
    PassableLevel KV_SAVEBLE(sum_passable_down_);
    PassableLevel KV_SAVEBLE(sum_passable_left_);
    PassableLevel KV_SAVEBLE(sum_passable_right_);

    KV_ON_LOAD_CALL(LoadInMap);
    void LoadInMap();
};
ADD_TO_TYPELIST(CubeTile);
