#ifndef FLUIDSYSTEM_H
#define FLUIDSYSTEM_H


#include "FluidSystem/fluidsolverproperty.h"
#include "SPH/sph.h"
#include "SPH/fluid.h"
#include "SPH/rigid.h"
#include "SPH/fluidproperty.h"


/// @author Idris Miles
/// @version 1.0


class Poly6Kernel;
class SpikyKernel;

/// @class FluidSystem, this class implements a custom fluid solver
class FluidSystem
{
public:
    FluidSystem(FluidSolverProperty _fluidSolverProperty = FluidSolverProperty());
    FluidSystem(const FluidSystem &_FluidSystem);
    ~FluidSystem();

    void SetFluidSolverProperty(FluidSolverProperty _fluidSolverProperty);
    void SetContainer(std::shared_ptr<Rigid> _container);
    void AddFluid(std::shared_ptr<Fluid> _fluid);
    void AddRigid(std::shared_ptr<Rigid> _rigid);
    void AddAlgae(std::shared_ptr<Algae> _algae);

    FluidSolverProperty GetProperty() const;
    std::shared_ptr<Fluid> GetFluid();
    std::shared_ptr<Algae> GetAlgae();
    std::vector<std::shared_ptr<Rigid>> GetActiveRigids();
    std::vector<std::shared_ptr<Rigid>> GetStaticRigids();

    virtual void InitialiseSim();
    virtual void ResetSim();
    virtual void StepSim();



private:
    void InitRigid(std::shared_ptr<Rigid> _rigid);
    void InitFluid(std::shared_ptr<Fluid> _fluid);
    void InitAlgae(std::shared_ptr<Algae> _algae);

    void ResetRigid(std::shared_ptr<Rigid> _rigid);
    void ResetFluid(std::shared_ptr<Fluid> _fluid);
    void ResetAlgae(std::shared_ptr<Algae> _algae);

    void GenerateDefaultContainer();

    std::shared_ptr<Algae> m_algae;
    std::shared_ptr<Fluid> m_fluid;
    std::shared_ptr<Rigid> m_container;
    std::vector<std::shared_ptr<Rigid>> m_staticRigids;
    std::vector<std::shared_ptr<Rigid>> m_activeRigids;
    FluidSolverProperty m_fluidSolverProperty;
    Poly6Kernel *m_poly6Kernel;
    SpikyKernel *m_spikyKernel;

    int m_frame;
    bool m_cache;

};

#endif // FLUIDSYSTEM_H
