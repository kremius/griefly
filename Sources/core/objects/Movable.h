#pragma once

#include "OnMapObject.h"

inline std::istream& operator>>(std::stringstream& file, VDir& vdir)
{
    file >> vdir.x;
    file >> vdir.y;
    file >> vdir.z;
    return file;
}

inline std::ostream& operator<<(std::stringstream& file, const VDir& vdir)
{
    file << vdir.x << " ";
    file << vdir.y << " ";
    file << vdir.z << " ";
    return file;
}

inline unsigned int hash(const VDir& vdir)
{
    return   vdir.x
           + vdir.y
           + vdir.z;
}

class IMovable : public IOnMapObject
{
public:
    DECLARE_SAVED(IMovable, IOnMapObject);
    DECLARE_GET_TYPE_ITEM(IMovable);
    IMovable(size_t id);
    void processMove();//for move
    virtual bool TryMove(Dir direct);
    bool CheckMoveTime();
    virtual bool CheckPassable();
    bool MainMove();

    virtual bool Rotate(Dir dir);
    Dir GetDir() const { return dMove; }

    virtual void ProcessForce();
    virtual void ApplyForce(VDir force) override;

    virtual void Delete() override { IOnMapObject::Delete(); }

    virtual void Represent() override;
    virtual void Bump(id_ptr_on<IMovable> item) override;
    virtual void BumpByGas(Dir dir, bool inside = false) override;
public:
    VDir KV_SAVEBLE(force_);
    bool KV_SAVEBLE(anchored);
    int KV_SAVEBLE(lastMove);
    int KV_SAVEBLE(tickSpeed);
    int KV_SAVEBLE(pixSpeed);

    KV_ON_LOAD_CALL(LoadInForceManager);
    void LoadInForceManager();
private:
    Dir KV_SAVEBLE(dMove);
};
ADD_TO_TYPELIST(IMovable);

class ForceManager
{
public:
    static ForceManager& Get();

    void Add(id_ptr_on<IMovable> m);
    void Process();
    void Clear();
    unsigned int Hash();
private:
    std::vector<id_ptr_on<IMovable>> under_force_;

    std::vector<id_ptr_on<IMovable>> to_add_;
};
