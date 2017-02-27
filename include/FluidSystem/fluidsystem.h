#ifndef FLUIDSYSTEM_H
#define FLUIDSYSTEM_H

#include "SPH/fluid.h"
#include "SPH/rigid.h"
#include "SPH/fluidproperty.h"
#include "FluidSystem/fluidsolverproperty.h"
#include "SPH/sph.h"

class Poly6Kernel;
class SpikyKernel;

/// @class FluidSystem, this class implements a custom fluid solver
class FluidSystem
{
public:
    FluidSystem(std::shared_ptr<FluidSolverProperty> _fluidSolverProperty = nullptr);
    FluidSystem(const FluidSystem &_FluidSystem);
    ~FluidSystem();

    void SetContainer(std::shared_ptr<Rigid> _container);
    void AddFluid(std::shared_ptr<Fluid> _fluid);
    void AddRigid(std::shared_ptr<Rigid> _rigid);
    void AddAlgae(std::shared_ptr<Fluid> _algae);
    void AddFluidSolverProperty(std::shared_ptr<FluidSolverProperty> _fluidSolverProperty);

    virtual void InitialiseSim();
    virtual void ResetSim();
    virtual void StepSimulation();



private:
    void ResetRigid(std::shared_ptr<Rigid> _rigid);
    void ResetFluid(std::shared_ptr<Fluid> _fluid);
    void GenerateDefaultContainer();

    std::shared_ptr<Fluid> m_algae;
    std::shared_ptr<Fluid> m_fluid;
    std::shared_ptr<Rigid> m_container;
    std::vector<std::shared_ptr<Rigid>> m_staticRigids;
    std::vector<std::shared_ptr<Rigid>> m_activeRigids;
    std::shared_ptr<FluidSolverProperty> m_fluidSolverProperty;
    Poly6Kernel *m_poly6Kernel;
    SpikyKernel *m_spikyKernel;

};

#endif // FLUIDSYSTEM_H