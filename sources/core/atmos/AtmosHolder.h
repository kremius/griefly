#pragma once

#include <array>
#include <sstream>

#include "AtmosConstants.h"
#include "AtmosGrid.h"

class AtmosHolder
{
    friend class Atmosphere;
    friend std::istream& operator>>(std::stringstream& file, AtmosHolder& atmos_holder);
    friend std::ostream& operator<<(std::stringstream& file, const AtmosHolder& atmos_holder);
    friend unsigned int hash(const AtmosHolder& atmos_holder);
public:
    AtmosHolder()
    {
        for (quint32 i = 0; i < GASES_NUM; ++i)
        {
            data_.gases[i] = 0;
        }
        data_.energy = 0;
        data_.pressure = 0;
        data_.temperature = 0;
        data_.volume = 1;
        data_ptr_ = &data_;
    }
    void Connect(AtmosHolder* guest, 
                 int level_owner = MAX_GAS_LEVEL, int level_guest = MAX_GAS_LEVEL,
                 int div = MAX_GAS_LEVEL / 2 /*100% - owner take all*/);
    void AddEnergy(int energy);
    void AddGase(int gase, int amount);
    void SetVolume(int volume);
    int GetEnergy();
    int GetPressure();
    int GetTemperature();
    int GetVolume();
    int GetGase(int gase);
    
    int RemoveGase(int gase, int amount);

    void Truncate();

    void UpdateMacroParams();
    void SetAtmosData(AtmosData* data)
    {
        data_ptr_ = data;
    }
private:
    AtmosData* data_ptr_;
    AtmosData data_;
};

std::istream& operator>>(std::stringstream& file, AtmosHolder& atmos_holder);
std::ostream& operator<<(std::stringstream& file, const AtmosHolder& atmos_holder);

inline unsigned int hash(const AtmosHolder& atmos_holder)
{
    unsigned int retval = 0;
    for (quint32 i = 0; i < GASES_NUM; ++i)
    {
        retval += atmos_holder.data_ptr_->gases[i];
    }
    retval += atmos_holder.data_ptr_->energy;
    retval += atmos_holder.data_ptr_->pressure;
    retval += atmos_holder.data_ptr_->volume;
    retval += atmos_holder.data_ptr_->temperature;
    return retval;
}

void AddDefaultValues(AtmosHolder* holder);
