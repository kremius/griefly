#pragma once

#include "OnMapObject.h"
#include "../atmos/AtmosHolder.h"

namespace Friction
{
    const int BASE_FRICTION = 100;
}

class ITurf : public IOnMapObject
{
public:
    DECLARE_SAVED(ITurf, IOnMapObject);
    DECLARE_GET_TYPE_ITEM(ITurf);
    ITurf(quint32 id);
    virtual void Delete() override;

    int GetFriction() const { return friction_; }
    void SetFriction(int friction) { friction_ = friction; }

    AtmosState GetAtmosState() const { return atmos_state_; }
    void SetAtmosState(AtmosState as) { atmos_state_ = as; }
private:
    int KV_SAVEBLE(friction_);
    AtmosState KV_SAVEBLE(atmos_state_);
};
ADD_TO_TYPELIST(ITurf);

namespace Friction
{
    int CombinedFriction(IdPtr<ITurf> turf);
}
