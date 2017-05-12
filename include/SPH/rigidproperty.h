#ifndef RIGIDPROPERTY_H
#define RIGIDPROPERTY_H

#include "SPH/sphparticlepropeprty.h"

class RigidProperty : public SphParticleProperty
{

public:
    RigidProperty(bool _static = true,
                  bool _kinematic = false,
                  unsigned int _numParticles = 8000,
                  float _particleMass = 1.0f,
                  float _particleRadius = 0.2f,
                  float _restDensity = 998.36f,
                  float _smoothingLength = 1.2f,
                  float3 _gravity = make_float3(0.0f, -9.8f, 0.0f)):
        SphParticleProperty(_numParticles, _particleMass, _particleRadius, _restDensity, _smoothingLength, _gravity),
        m_static(_static),
        kinematic(_kinematic)
    {
    }

    ~RigidProperty(){}

    bool m_static;
    bool kinematic;
};


#endif // RIGIDPROPERTY_H
