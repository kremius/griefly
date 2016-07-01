#pragma once

#include <vector>

#include "Turf.h"
#include "OnMapBase.h"
#include "OnMapObject.h"
#include "Projectiles.h"
#include "../AtmosHolder.h"

template<class T>
std::iostream& operator<<(std::iostream& file, std::vector<id_ptr_on<T>>& content)
{
    file << " " << content.size() << " ";
    for (auto it = content.begin(); it != content.end(); ++it)
        file << " " << it->ret_id() << " ";
    return file;
}

template<class T>
std::iostream& operator>>(std::iostream& file, std::vector<id_ptr_on<T>>& content)
{
    size_t size;
    file >> size;
    content.reserve(size);

    unsigned int local_id;
    for (size_t i = 0; i < size; ++i)
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
    CubeTile(size_t id);
    virtual bool AddItem(id_ptr_on<IOnMapBase> item) override;
    virtual bool RemoveItem(id_ptr_on<IOnMapBase> item) override;
    virtual id_ptr_on<IOnMapBase> GetNeighbour(Dir direct) const override;
    id_ptr_on<CubeTile> GetNeighbourImpl(Dir direct) const;
    virtual void ForEach(std::function<void(id_ptr_on<IOnMapBase>)> callback) override;
    virtual bool IsVisibleByPlayer() const override
    {
        // TODO
        return true;       
    };
    virtual PassableLevel GetPassable(Dir direct) const override;
    virtual bool IsTransparent() const override;

    virtual bool CanTouch(id_ptr_on<IOnMapBase> item) const override;
    virtual bool Contains(id_ptr_on<IOnMapBase> item) const override;

    virtual void Bump(id_ptr_on<IMovable> item) override;
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
    virtual id_ptr_on<ITurf> GetTurf()  override
    {
        return turf_;
    }
    virtual void SetTurf(id_ptr_on<ITurf> turf) override
    {
        turf_ = turf;
        if (turf_.valid())
        {
            turf_->SetOwner(GetId());
        }
        UpdatePassable();
    }
    virtual void UpdatePassable() override;
    AtmosHolder* GetAtmosHolder() { return &atmos_holder_; }

    typedef std::vector<id_ptr_on<IOnMapObject>> InsideType;
    InsideType& GetInsideList() { return inside_list_; }
protected:
    virtual size_t GetItemImpl(unsigned int hash) override;
private:
    bool CanTouch(id_ptr_on<IOnMapBase> item, Dir dir) const;
    bool CanTouch(id_ptr_on<IOnMapBase> item, Dir first_dir, Dir second_dir) const;

    void MoveToDir(Dir dir, int* x, int* y, int* z = nullptr) const;

    id_ptr_on<ITurf> KV_SAVEBLE(turf_);

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
    void LoadInMap(); // TODO
};
ADD_TO_TYPELIST(CubeTile);
