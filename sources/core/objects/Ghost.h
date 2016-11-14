#pragma once

#include "Mob.h"

class Ghost: public IMob
{
public:
    DECLARE_SAVED(Ghost, IMob);
    DECLARE_GET_TYPE_ITEM(Ghost);
    Ghost(quint32 id);
    virtual void AfterWorldCreation() override;
    virtual void Represent() override;
    virtual void CalculateVisible(std::list<PosPoint>* visible_list) override;
    virtual void ProcessMessage(const Message2& msg) override;

    virtual void InitGUI() override;
    virtual void DeinitGUI() override;

    virtual void Process() override;
private:
    int KV_SAVEBLE(seconds_until_respawn_);
    bool IsMobGhost();
};
ADD_TO_TYPELIST(Ghost);
