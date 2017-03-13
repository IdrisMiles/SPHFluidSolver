#include "include/SPH/sph.h"

void sph::ResetProperties(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::shared_ptr<BaseSphParticle> _sphParticles)
{
    const uint numCells = _solverProps->gridResolution * _solverProps->gridResolution * _solverProps->gridResolution;
    auto fluidProps =  _sphParticles->GetProperty();

    sphGPU::ResetProperties(_sphParticles->GetPressureForcePtr(),
                            _sphParticles->GetExternalForcePtr(),
                            _sphParticles->GetTotalForcePtr(),
                            _sphParticles->GetMassPtr(),
                            _sphParticles->GetDensityPtr(),
                            _sphParticles->GetPressurePtr(),
                            _sphParticles->GetParticleHashIdPtr(),
                            _sphParticles->GetCellOccupancyPtr(),
                            _sphParticles->GetCellParticleIdxPtr(),
                            fluidProps->particleMass,
                            numCells,
                            fluidProps->numParticles);
}

void sph::ResetProperties(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::vector<std::shared_ptr<BaseSphParticle>> _sphParticles)
{
    for(auto &&sp : _sphParticles)
    {
        ResetProperties(_solverProps, sp);
    }
}

void sph::ResetProperties(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::shared_ptr<Fluid> _fluid)
{
    const uint numCells = _solverProps->gridResolution * _solverProps->gridResolution * _solverProps->gridResolution;
    auto fluidProps =  _fluid->GetProperty();

    sphGPU::ResetProperties(_fluid->GetPressureForcePtr(),
                            _fluid->GetViscForcePtr(),
                            _fluid->GetSurfTenForcePtr(),
                            _fluid->GetExternalForcePtr(),
                            _fluid->GetTotalForcePtr(),
                            _fluid->GetDensityErrPtr(),
                            _fluid->GetMassPtr(),
                            _fluid->GetDensityPtr(),
                            _fluid->GetPressurePtr(),
                            _fluid->GetParticleHashIdPtr(),
                            _fluid->GetCellOccupancyPtr(),
                            _fluid->GetCellParticleIdxPtr(),
                            fluidProps->particleMass,
                            numCells,
                            fluidProps->numParticles);
}

void sph::ResetProperties(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::vector<std::shared_ptr<Fluid>> _fluid)
{
    for(auto &&f : _fluid)
    {
        ResetProperties(_solverProps, f);
    }
}

void sph::ResetProperties(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::shared_ptr<Rigid> _rigid)
{
    const uint numCells = _solverProps->gridResolution * _solverProps->gridResolution * _solverProps->gridResolution;
    auto fluidProps =  _rigid->GetProperty();

    sphGPU::ResetProperties(_rigid->GetPressureForcePtr(),
                            _rigid->GetExternalForcePtr(),
                            _rigid->GetTotalForcePtr(),
                            _rigid->GetMassPtr(),
                            _rigid->GetDensityPtr(),
                            _rigid->GetPressurePtr(),
                            _rigid->GetVolumePtr(),
                            _rigid->GetParticleHashIdPtr(),
                            _rigid->GetCellOccupancyPtr(),
                            _rigid->GetCellParticleIdxPtr(),
                            fluidProps->particleMass,
                            numCells,
                            fluidProps->numParticles);
}

void sph::ResetProperties(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::vector<std::shared_ptr<Rigid>> _rigid)
{
    for(auto &&r : _rigid)
    {
        ResetProperties(_solverProps, r);
    }
}

void sph::ResetTotalForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::shared_ptr<BaseSphParticle> _sphParticles)
{
    const uint numCells = _solverProps->gridResolution * _solverProps->gridResolution * _solverProps->gridResolution;
    auto fluidProps =  _sphParticles->GetProperty();

    sphGPU::ResetTotalForce(_sphParticles->GetTotalForcePtr(),
                            fluidProps->numParticles);
}

void sph::InitFluidAsCube(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::shared_ptr<BaseSphParticle> _sphParticles)
{
    auto fluidProps =  _sphParticles->GetProperty();

    sphGPU::InitFluidAsCube(_sphParticles->GetPositionPtr(),
                            _sphParticles->GetVelocityPtr(),
                            _sphParticles->GetDensityPtr(),
                            fluidProps->restDensity,
                            fluidProps->numParticles,
                            ceil(cbrt(fluidProps->numParticles)),
                            2.0f*fluidProps->particleRadius);
}

//--------------------------------------------------------------------------------------

void sph::ComputeHash(std::shared_ptr<FluidSolverProperty> _solverProps,
                      std::shared_ptr<BaseSphParticle> _fluid)
{
    auto fluidProps =  _fluid->GetProperty();

    sphGPU::ParticleHash(_fluid->GetParticleHashIdPtr(),
                         _fluid->GetCellOccupancyPtr(),
                         _fluid->GetPositionPtr(),
                         fluidProps->numParticles,
                         _solverProps->gridResolution,
                         _solverProps->gridCellWidth);
}


//void sph::ComputeHash(std::vector<std::shared_ptr<BaseSphParticle>> _fluid,
//                      std::shared_ptr<FluidSolverProperty> _solverProps)
//{
//    for(auto &&f : _fluid)
//    {
//        ComputeHash(f, _solverProps);
//    }
//}

void sph::SortParticlesByHash(std::shared_ptr<BaseSphParticle> _sphParticles)
{
    sphGPU::SortParticlesByHash(_sphParticles->GetParticleHashIdPtr(),
                                _sphParticles->GetPositionPtr(),
                                _sphParticles->GetVelocityPtr(),
                                _sphParticles->GetProperty()->numParticles);
}

//void sph::SortParticlesByHash(std::vector<std::shared_ptr<BaseSphParticle>> _sphParticles)
//{
//    for(auto &&f : _sphParticles)
//    {
//        SortParticlesByHash(f);
//    }
//}

void sph::ComputeParticleScatterIds(std::shared_ptr<FluidSolverProperty> _solverProps,
                                    std::shared_ptr<BaseSphParticle> _sphParticles)
{
    const uint numCells = _solverProps->gridResolution * _solverProps->gridResolution * _solverProps->gridResolution;

    sphGPU::ComputeParticleScatterIds(_sphParticles->GetCellOccupancyPtr(),
                                      _sphParticles->GetCellParticleIdxPtr(),
                                      numCells);
}

//void sph::ComputeParticleScatterIds(std::vector<std::shared_ptr<BaseSphParticle>> _sphParticles,
//                                    std::shared_ptr<FluidSolverProperty> _solverProps)
//{
//    for(auto &&f : _sphParticles)
//    {
//        ComputeParticleScatterIds(f, _solverProps);
//    }
//}

void sph::ComputeMaxCellOccupancy(std::shared_ptr<FluidSolverProperty> _solverProps,
                                  std::shared_ptr<BaseSphParticle> _sphParticles,
                                  unsigned int &_maxCellOcc)
{
    const uint numCells = _solverProps->gridResolution * _solverProps->gridResolution * _solverProps->gridResolution;

    sphGPU::ComputeMaxCellOccupancy(_sphParticles->GetCellOccupancyPtr(),
                                    numCells,
                                    _maxCellOcc);

    _sphParticles->SetMaxCellOcc(_maxCellOcc);
}

//void sph::ComputeMaxCellOccupancy(std::vector<std::shared_ptr<BaseSphParticle>> _sphParticles,
//                                    std::shared_ptr<FluidSolverProperty> _solverProps,
//                                  unsigned int &_maxCellOcc)
//{
//    for(auto &&f : _sphParticles)
//    {
//        ComputeMaxCellOccupancy(f, _solverProps, _maxCellOcc);
//    }
//}

void sph::ComputeParticleVolume(std::shared_ptr<FluidSolverProperty> _solverProps,
                                std::shared_ptr<Rigid> _rigid)
{
    auto rigidProps =  _rigid->GetProperty();

    sphGPU::ComputeParticleVolume(_rigid->GetMaxCellOcc(),
                                  _solverProps->gridResolution,
                                  _rigid->GetVolumePtr(),
                                  _rigid->GetCellOccupancyPtr(),
                                  _rigid->GetCellParticleIdxPtr(),
                                  _rigid->GetPositionPtr(),
                                  rigidProps->numParticles,
                                  rigidProps->smoothingLength);
}

void sph::ComputeParticleVolume(std::shared_ptr<FluidSolverProperty> _solverProps,
                                std::vector<std::shared_ptr<Rigid>> _rigid)
{
    for(auto &&r : _rigid)
    {
        ComputeParticleVolume(_solverProps, r);
    }
}

//--------------------------------------------------------------------------------------

void sph::ComputeDensity(std::shared_ptr<FluidSolverProperty> _solverProps,
                         std::shared_ptr<BaseSphParticle> _fluid,
                         const bool accumulate)
{
    auto fluidProps =  _fluid->GetProperty();

    sphGPU::ComputeDensity(_fluid->GetMaxCellOcc(),
                            _solverProps->gridResolution,
                            _fluid->GetDensityPtr(),
                            _fluid->GetMassPtr(),
                            _fluid->GetCellOccupancyPtr(),
                            _fluid->GetCellParticleIdxPtr(),
                            _fluid->GetPositionPtr(),
                            fluidProps->numParticles,
                            fluidProps->smoothingLength,
                            accumulate);
}

void sph::ComputeDensity(std::shared_ptr<FluidSolverProperty> _solverProps,
                         std::shared_ptr<BaseSphParticle> _fluid,
                         std::shared_ptr<BaseSphParticle> _fluidContributer,
                         const bool accumulate)
{
    auto fluidProps =  _fluid->GetProperty();
    auto fluidContribProps =  _fluidContributer->GetProperty();

    sphGPU::ComputeDensityFluidFluid(_fluid->GetMaxCellOcc(),
                                     _solverProps->gridResolution,
                                     fluidProps->numParticles,
                                     _fluid->GetDensityPtr(),
                                     _fluid->GetPositionPtr(),
                                     _fluid->GetCellOccupancyPtr(),
                                     _fluid->GetCellParticleIdxPtr(),
                                     _fluidContributer->GetMassPtr(),
                                     _fluidContributer->GetPositionPtr(),
                                     _fluidContributer->GetCellOccupancyPtr(),
                                     _fluidContributer->GetCellParticleIdxPtr(),
                                     fluidProps->smoothingLength,
                                     accumulate);
}

void sph::ComputeDensity(std::shared_ptr<FluidSolverProperty> _solverProps,
                         std::shared_ptr<BaseSphParticle> _fluid,
                         std::shared_ptr<Rigid> _rigid,
                         const bool accumulate)
{
    auto fluidProps =  _fluid->GetProperty();
    auto rigidProps =  _rigid->GetProperty();

    sphGPU::ComputeDensityFluidRigid(_fluid->GetMaxCellOcc(),
                                        _solverProps->gridResolution,
                                        fluidProps->numParticles,
                                        fluidProps->restDensity,
                                        _fluid->GetDensityPtr(),
                                        _fluid->GetPositionPtr(),
                                        _fluid->GetCellOccupancyPtr(),
                                        _fluid->GetCellParticleIdxPtr(),
                                        _rigid->GetVolumePtr(),
                                        _rigid->GetPositionPtr(),
                                        _rigid->GetCellOccupancyPtr(),
                                        _rigid->GetCellParticleIdxPtr(),
                                        fluidProps->smoothingLength,
                                        accumulate);
}

void sph::ComputeDensity(std::shared_ptr<FluidSolverProperty> _solverProps,
                         std::shared_ptr<BaseSphParticle> _fluid,
                         std::vector<std::shared_ptr<BaseSphParticle>> &_fluidContributers,
                         const bool accumulate)
{
    for(auto &&fc : _fluidContributers)
    {
        sph::ComputeDensity(_solverProps, _fluid, fc, accumulate);
    }
}

void sph::ComputeDensity(std::shared_ptr<FluidSolverProperty> _solverProps,
                         std::shared_ptr<BaseSphParticle> _fluid,
                                    std::vector<std::shared_ptr<Rigid>> _rigids,
                                    const bool accumulate)
{
    for(auto &&r : _rigids)
    {
        sph::ComputeDensity(_solverProps, _fluid, r, accumulate);
    }
}


void sph::ComputePressure(std::shared_ptr<FluidSolverProperty> _solverProps,
                          std::shared_ptr<Fluid> _fluid)
{
    auto fluidProps =  _fluid->GetProperty();

    sphGPU::ComputePressureFluid(_fluid->GetMaxCellOcc(),
                            _solverProps->gridResolution,
                            _fluid->GetPressurePtr(),
                            _fluid->GetDensityPtr(),
                            fluidProps->restDensity,
                            fluidProps->gasStiffness,
                            _fluid->GetCellOccupancyPtr(),
                            _fluid->GetCellParticleIdxPtr(),
                            fluidProps->numParticles);
}

void sph::ComputePressureForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                               std::shared_ptr<BaseSphParticle> _fluid,
                               const bool accumulate)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::ComputePressureForceFluid(_fluid->GetMaxCellOcc(),
                                 _solverProps->gridResolution,
                                 _fluid->GetPressureForcePtr(),
                                 _fluid->GetPressurePtr(),
                                 _fluid->GetDensityPtr(),
                                 _fluid->GetMassPtr(),
                                 _fluid->GetPositionPtr(),
                                 _fluid->GetCellOccupancyPtr(),
                                 _fluid->GetCellParticleIdxPtr(),
                                 fluidProps->numParticles,
                                 fluidProps->smoothingLength, accumulate);
}

void sph::ComputePressureForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                               std::shared_ptr<BaseSphParticle> _fluid,
                               std::shared_ptr<BaseSphParticle> _fluidContributer,
                               const bool accumulate)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::ComputePressureForceFluidFluid(_fluid->GetMaxCellOcc(),
                                 _solverProps->gridResolution,
                                 _fluid->GetPressureForcePtr(),
                                 _fluid->GetPressurePtr(),
                                 _fluid->GetDensityPtr(),
                                 _fluid->GetMassPtr(),
                                 _fluid->GetPositionPtr(),
                                 _fluid->GetCellOccupancyPtr(),
                                 _fluid->GetCellParticleIdxPtr(),
                                 _fluidContributer->GetPressurePtr(),
                                 _fluidContributer->GetDensityPtr(),
                                 _fluidContributer->GetMassPtr(),
                                 _fluidContributer->GetPositionPtr(),
                                 _fluidContributer->GetCellOccupancyPtr(),
                                 _fluidContributer->GetCellParticleIdxPtr(),
                                 fluidProps->numParticles,
                                 fluidProps->smoothingLength, accumulate);
}

void sph::ComputePressureForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                               std::shared_ptr<BaseSphParticle> _fluid,
                               std::shared_ptr<Rigid> _rigid,
                               const bool accumulate)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::ComputePressureForceFluidRigid(_fluid->GetMaxCellOcc(),
                                           _solverProps->gridResolution,
                                           _fluid->GetPressureForcePtr(),
                                           _fluid->GetPressurePtr(),
                                           _fluid->GetDensityPtr(),
                                           _fluid->GetMassPtr(),
                                           _fluid->GetPositionPtr(),
                                           _fluid->GetCellOccupancyPtr(),
                                           _fluid->GetCellParticleIdxPtr(),
                                           fluidProps->restDensity,
                                           _rigid->GetVolumePtr(),
                                           _rigid->GetPositionPtr(),
                                           _rigid->GetCellOccupancyPtr(),
                                           _rigid->GetCellParticleIdxPtr(),
                                           fluidProps->numParticles,
                                           fluidProps->smoothingLength, accumulate);
}

void sph::ComputePressureForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                               std::shared_ptr<BaseSphParticle> _fluid,
                               std::vector<std::shared_ptr<BaseSphParticle>> &_fluidContributers,
                               const bool accumulate)
{
    for(auto &&fc : _fluidContributers)
    {
        sph::ComputePressureForce(_solverProps, _fluid, fc, accumulate);
    }
}

void sph::ComputePressureForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                               std::shared_ptr<BaseSphParticle> _fluid,
                               std::vector<std::shared_ptr<Rigid>> _rigids,
                               const bool accumulate)
{
    for(auto &&r : _rigids)
    {
        sph::ComputePressureForce(_solverProps, _fluid, r, accumulate);
    }
}

void sph::ComputeViscForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                           std::shared_ptr<Fluid> _fluid)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::ComputeViscForce(_fluid->GetMaxCellOcc(),
                             _solverProps->gridResolution,
                             _fluid->GetViscForcePtr(),
                             fluidProps->viscosity,
                             _fluid->GetVelocityPtr(),
                             _fluid->GetDensityPtr(),
                             _fluid->GetMassPtr(),
                             _fluid->GetPositionPtr(),
                             _fluid->GetCellOccupancyPtr(),
                             _fluid->GetCellParticleIdxPtr(),
                             fluidProps->numParticles,
                             fluidProps->smoothingLength);
}

void sph::ComputeSurfaceTensionForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                                     std::shared_ptr<Fluid> _fluid)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::ComputeSurfaceTensionForce(_fluid->GetMaxCellOcc(),
                                       _solverProps->gridResolution,
                                       _fluid->GetSurfTenForcePtr(),
                                       fluidProps->surfaceTension,
                                       fluidProps->surfaceThreshold,
                                       _fluid->GetDensityPtr(),
                                       _fluid->GetMassPtr(),
                                       _fluid->GetPositionPtr(),
                                       _fluid->GetCellOccupancyPtr(),
                                       _fluid->GetCellParticleIdxPtr(),
                                       fluidProps->numParticles,
                                       fluidProps->smoothingLength);
}

void sph::ComputeForces(std::shared_ptr<FluidSolverProperty> _solverProps,
                        std::shared_ptr<Fluid> _fluid,
                        const bool pressure,
                        const bool viscosity,
                        const bool surfTen,
                        const bool accumulate)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::ComputeForce(_fluid->GetMaxCellOcc(),
                         _solverProps->gridResolution,
                         _fluid->GetPressureForcePtr(),
                         _fluid->GetViscForcePtr(),
                         _fluid->GetSurfTenForcePtr(),
                         fluidProps->viscosity,
                         fluidProps->surfaceTension,
                         fluidProps->surfaceThreshold,
                         _fluid->GetPressurePtr(),
                         _fluid->GetDensityPtr(),
                         _fluid->GetMassPtr(),
                         _fluid->GetPositionPtr(),
                         _fluid->GetVelocityPtr(),
                         _fluid->GetCellOccupancyPtr(),
                         _fluid->GetCellParticleIdxPtr(),
                         fluidProps->numParticles,
                         fluidProps->smoothingLength, accumulate);
}

void sph::ComputeTotalForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                            std::shared_ptr<Fluid> _fluid,
                            const bool accumulatePressure,
                            const bool accumulateViscous,
                            const bool accumulateSurfTen,
                            const bool accumulateExternal,
                            const bool accumulateGravity)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::ComputeTotalForce(_fluid->GetMaxCellOcc(),
                              _solverProps->gridResolution,
                              accumulatePressure,
                              accumulateViscous,
                              accumulateSurfTen,
                              accumulateExternal,
                              accumulateGravity,
                              _fluid->GetTotalForcePtr(),
                              _fluid->GetExternalForcePtr(),
                              _fluid->GetPressureForcePtr(),
                              _fluid->GetViscForcePtr(),
                              _fluid->GetSurfTenForcePtr(),
                              fluidProps->gravity,
                              _fluid->GetMassPtr(),
                              _fluid->GetPositionPtr(),
                              _fluid->GetVelocityPtr(),
                              _fluid->GetCellOccupancyPtr(),
                              _fluid->GetCellParticleIdxPtr(),
                              fluidProps->numParticles,
                              fluidProps->smoothingLength);

}

void sph::Integrate(std::shared_ptr<FluidSolverProperty> _solverProps,
                    std::shared_ptr<Fluid> _fluid)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::Integrate(_fluid->GetMaxCellOcc(),
                      _solverProps->gridResolution,
                      _fluid->GetTotalForcePtr(),
                      _fluid->GetPositionPtr(),
                      _fluid->GetVelocityPtr(),
                      _solverProps->deltaTime,
                      fluidProps->numParticles);
}

void sph::HandleBoundaries(std::shared_ptr<FluidSolverProperty> _solverProps,
                           std::shared_ptr<Fluid> _fluid)
{
    auto fluidProps = _fluid->GetProperty();

    sphGPU::HandleBoundaries(_fluid->GetMaxCellOcc(),
                             _solverProps->gridResolution,
                             _fluid->GetPositionPtr(),
                             _fluid->GetVelocityPtr(),
                             (float)0.5f*_solverProps->gridCellWidth * _solverProps->gridResolution,
                             fluidProps->numParticles);
}






//---------------------------------------------------------------------------------------
// PCISPH functions

void sph::pci::PredictIntegrate(std::shared_ptr<FluidSolverProperty> _solverProps,
                                std::shared_ptr<Fluid> _fluid)
{

}

void sph::pci::PredictDensity(std::shared_ptr<FluidSolverProperty> _solverProps,
                              std::shared_ptr<Fluid> _fluid)
{

}

void sph::pci::predictDensityVariation(std::shared_ptr<FluidSolverProperty> _solverProps,
                                       std::shared_ptr<Fluid> _fluid)
{

}

void sph::pci::ComputeMaxDensityVariation(std::shared_ptr<FluidSolverProperty> _solverProps,
                                          std::shared_ptr<Fluid> _fluid,
                                          float &_maxDenVar)
{

}

void sph::pci::UpdatePressure(std::shared_ptr<FluidSolverProperty> _solverProps,
                              std::shared_ptr<Fluid> _fluid)
{

}

void sph::pci::ComputePressureForce(std::shared_ptr<FluidSolverProperty> _solverProps,
                                    std::shared_ptr<Fluid> _fluid)
{

}
