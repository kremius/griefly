#include "AtmosHolder.h"

#include "../Constheader.h"

namespace
{
    int loc_gases[atmos::GASES_NUM];
    int loc_energy;
}

namespace atmos
{
    void AtmosHolder::Connect(AtmosHolder* guest, int level_owner, int level_guest, int div)
    {
        if (level_owner > MAX_GAS_LEVEL)
        {
            level_owner = MAX_GAS_LEVEL;
        }
        if (level_owner < 0)
        {
            level_owner = 0;
        }
        if (level_guest > MAX_GAS_LEVEL)
        {
            level_guest = MAX_GAS_LEVEL;
        }
        if (level_guest < 0)
        {
            level_guest = 0;
        }
        if (div > MAX_GAS_LEVEL)
        {
            div = MAX_GAS_LEVEL;
        }
        if (div < 0)
        {
            div = 0;
        }

        for (quint32 i = 0; i < GASES_NUM; ++i)
        {
            int gase_owner = (data_ptr_->gases[i] * level_owner) / MAX_GAS_LEVEL;
            data_ptr_->gases[i] -= gase_owner;

            int gase_guest = (guest->data_ptr_->gases[i] * level_guest) / MAX_GAS_LEVEL;
            guest->data_ptr_->gases[i] -= gase_guest;

            loc_gases[i] = gase_owner + gase_guest;
        }

        int energy_owner = (data_ptr_->energy * level_owner) / MAX_GAS_LEVEL;
        data_ptr_->energy -= energy_owner;

        int energy_guest = (guest->data_ptr_->energy * level_guest) / MAX_GAS_LEVEL;
        guest->data_ptr_->energy -= energy_guest;

        loc_energy = energy_owner + energy_guest;

        for (quint32 i = 0; i < GASES_NUM; ++i)
        {
            int gase_owner = (loc_gases[i] * div) / MAX_GAS_LEVEL;
            int gase_guest = loc_gases[i] - gase_owner;

            data_ptr_->gases[i] += gase_owner;
            guest->data_ptr_->gases[i] += gase_guest;
        }

        energy_owner = (loc_energy * div) / MAX_GAS_LEVEL;
        energy_guest = loc_energy - energy_owner;

        data_ptr_->energy += energy_owner;
        guest->data_ptr_->energy += energy_guest;

        UpdateMacroParams();
        guest->UpdateMacroParams();
    }

    void AtmosHolder::UpdateMacroParams()
    {
        atmos::UpdateMacroParams(data_ptr_);
    }

    void AtmosHolder::SetVolume(int volume)
    {
        data_ptr_->volume = volume;
        UpdateMacroParams();
    }

    void AtmosHolder::AddGase(int gase, int amount)
    {
        data_ptr_->gases[gase] += amount;
        UpdateMacroParams();
    }

    void AtmosHolder::AddEnergy(int energy)
    {
        data_ptr_->energy += energy;
        UpdateMacroParams();
    }

    int AtmosHolder::GetEnergy() const
    {
        return data_ptr_->energy;
    }
    int AtmosHolder::GetPressure() const
    {
        return data_ptr_->pressure;
    }
    int AtmosHolder::GetTemperature() const
    {
        return data_ptr_->temperature;
    }
    int AtmosHolder::GetVolume() const
    {
        return data_ptr_->volume;
    }

    int AtmosHolder::GetGase(int gase) const
    {
        return data_ptr_->gases[gase];
    }

    int AtmosHolder::RemoveGase(int gase, int amount)
    {
        int retval = amount;
        if (static_cast<int>(amount) > data_ptr_->gases[gase])
        {
            retval = data_ptr_->gases[gase];
            data_ptr_->gases[gase] = 0;
            UpdateMacroParams();
            return retval;
        }
        data_ptr_->gases[gase] -= static_cast<int>(amount);
        UpdateMacroParams();
        return retval;
    }

    void AtmosHolder::Truncate()
    {
        for (quint32 i = 0; i < GASES_NUM; ++i)
        {
            data_ptr_->gases[i] = 0;
        }
        data_ptr_->energy = 0;

        UpdateMacroParams();
    }

    void AddDefaultValues(AtmosHolder *holder)
    {
        holder->AddGase(NYTROGEN, 750);
        holder->AddGase(OXYGEN, 230);
        holder->AddGase(CO2, 1);
        holder->AddEnergy(1000);
    }
}

FastSerializer& operator<<(FastSerializer& file, const atmos::AtmosHolder& atmos_holder)
{
    for (quint32 i = 0; i < atmos::GASES_NUM; ++i)
    {
        file << atmos_holder.data_ptr_->gases[i];
    }
    file << atmos_holder.data_ptr_->energy;
    file << atmos_holder.data_ptr_->pressure;
    file << atmos_holder.data_ptr_->volume;
    file << atmos_holder.data_ptr_->temperature;

    return file;
}

FastDeserializer& operator>>(FastDeserializer& file, atmos::AtmosHolder& atmos_holder)
{
    for (quint32 i = 0; i < atmos::GASES_NUM; ++i)
    {
        file >> atmos_holder.data_ptr_->gases[i];
    }
    file >> atmos_holder.data_ptr_->energy;
    file >> atmos_holder.data_ptr_->pressure;
    file >> atmos_holder.data_ptr_->volume;
    file >> atmos_holder.data_ptr_->temperature;

    return file;
}
