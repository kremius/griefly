#pragma once

#include "Mob.h"

class Human: public IMob
{
public:
    DECLARE_SAVED(Human, IMob);
    DECLARE_GET_TYPE_ITEM(Human);
    Human(size_t id);

    virtual void AfterWorldCreation() override;

    virtual void DeinitGUI() override;
    virtual void InitGUI() override;
    virtual void GenerateInterfaceForFrame() override;
    virtual void processGUImsg(const Message2& msg) override;
    virtual void Process() override;
    virtual void Live();

    virtual void OnDeath();

    void SetLying(bool value);
    bool GetLying() const { return lying_; }

    void AddLyingTimer(int value);
  
    virtual void Bump(id_ptr_on<IMovable> item) override;

    virtual void AttackBy(id_ptr_on<Item> item) override;

    virtual void Represent() override;

    virtual bool TryMove(Dir direct) override;
    virtual InterfaceBase* GetInterface() override { return &interface_; }

    virtual void CalculateVisible(std::list<point>* visible_list) override;

    void UpdateOverlays();

    int GetHealth() { return health_; }
protected:
    int KV_SAVEBLE(attack_cooldown_);

    HumanInterface KV_SAVEBLE(interface_);

    int KV_SAVEBLE(lay_timer_);

    bool KV_SAVEBLE(lying_);
    bool KV_SAVEBLE(dead_);

    int KV_SAVEBLE(health_);
};
ADD_TO_TYPELIST(Human);

class CaucasianHuman: public Human
{
public:
    DECLARE_SAVED(CaucasianHuman, Human);
    DECLARE_GET_TYPE_ITEM(CaucasianHuman);
    CaucasianHuman(size_t id);
    virtual void AfterWorldCreation() override;

    virtual void OnDeath() override;
};
ADD_TO_TYPELIST(CaucasianHuman);
