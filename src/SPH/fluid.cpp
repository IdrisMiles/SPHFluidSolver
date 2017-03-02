#include "SPH/fluid.h"
#include <math.h>
#include <sys/time.h>
#include <glm/gtx/transform.hpp>


Fluid::Fluid(std::shared_ptr<FluidProperty> _fluidProperty)
{
    m_fluidProperty = _fluidProperty;

    m_colour = glm::vec3(0.6f, 0.6f, 0.6f);

    m_positionMapped = false;
    m_velocityMapped = false;
    m_densityMapped = false;
    m_massMapped = false;
    m_pressureMapped = false;

    Init();
}

Fluid::Fluid(std::shared_ptr<FluidProperty> _fluidProperty, Mesh _mesh)
{
    m_fluidProperty = _fluidProperty;
    m_mesh = _mesh;


    m_colour = glm::vec3(0.6f, 0.6f, 0.6f);

    m_positionMapped = false;
    m_velocityMapped = false;
    m_densityMapped = false;
    m_massMapped = false;
    m_pressureMapped = false;

    Init();
    InitFluidAsMesh();
}

Fluid::~Fluid()
{
    m_fluidProperty = nullptr;
    CleanUpGL();
    CleanUpCUDAMemory();
}


//------------------------------------------------------------------------

void Fluid::SetupSolveSpecs(std::shared_ptr<FluidSolverProperty> _solverProps)
{
    const uint numCells = _solverProps->gridResolution * _solverProps->gridResolution * _solverProps->gridResolution;
    cudaMalloc(&d_cellOccupancyPtr, numCells * sizeof(unsigned int));
    cudaMalloc(&d_cellParticleIdxPtr, numCells * sizeof(unsigned int));
}

void Fluid::Init()
{
    cudaSetDevice(0);

    InitGL();
    InitCUDAMemory();

}

void Fluid::InitCUDAMemory()
{

    // particle properties
    cudaGraphicsGLRegisterBuffer(&m_posBO_CUDA, m_posBO.bufferId(),cudaGraphicsMapFlagsWriteDiscard);
    cudaGraphicsGLRegisterBuffer(&m_velBO_CUDA, m_velBO.bufferId(),cudaGraphicsMapFlagsWriteDiscard);
    cudaGraphicsGLRegisterBuffer(&m_denBO_CUDA, m_denBO.bufferId(),cudaGraphicsMapFlagsWriteDiscard);
    cudaGraphicsGLRegisterBuffer(&m_massBO_CUDA, m_massBO.bufferId(),cudaGraphicsMapFlagsWriteDiscard);
    cudaGraphicsGLRegisterBuffer(&m_pressBO_CUDA, m_pressBO.bufferId(),cudaGraphicsMapFlagsWriteDiscard);

    // particle forces
    cudaMalloc(&d_pressureForcePtr, m_fluidProperty->numParticles * sizeof(float3));
    cudaMalloc(&d_viscousForcePtr, m_fluidProperty->numParticles * sizeof(float3));
    cudaMalloc(&d_surfaceTensionForcePtr, m_fluidProperty->numParticles * sizeof(float3));
    cudaMalloc(&d_gravityForcePtr, m_fluidProperty->numParticles * sizeof(float3));
    cudaMalloc(&d_externalForcePtr, m_fluidProperty->numParticles * sizeof(float3));
    cudaMalloc(&d_totalForcePtr, m_fluidProperty->numParticles * sizeof(float3));

    // particle hash
    cudaMalloc(&d_particleHashIdPtr, m_fluidProperty->numParticles * sizeof(unsigned int));
}

void Fluid::InitGL()
{
    InitVAO();
}

void Fluid::InitVAO()
{

    // Setup our pos buffer object.
    m_posBO.create();
    m_posBO.bind();
    m_posBO.allocate(m_fluidProperty->numParticles * sizeof(float3));
    m_posBO.release();


    // Set up velocity buffer object
    m_velBO.create();
    m_velBO.bind();
    m_velBO.allocate(m_fluidProperty->numParticles * sizeof(float3));
    m_velBO.release();


    // Set up density buffer object
    m_denBO.create();
    m_denBO.bind();
    m_denBO.allocate(m_fluidProperty->numParticles * sizeof(float));
    m_denBO.release();


    // Set up mass buffer object
    m_massBO.create();
    m_massBO.bind();
    m_massBO.allocate(m_fluidProperty->numParticles * sizeof(float));
    m_massBO.release();


    // Set up pressure buffer object
    m_pressBO.create();
    m_pressBO.bind();
    m_pressBO.allocate(m_fluidProperty->numParticles * sizeof(float));
    m_pressBO.release();
}


void Fluid::InitFluidAsMesh()
{
    GetPositionPtr();
    cudaMemcpy(d_positionPtr, &m_mesh.verts[0], m_property->numParticles * sizeof(float3), cudaMemcpyHostToDevice);
    ReleaseCudaGLResources();
}

//------------------------------------------------------------------------
// Clean-up Functions

void Fluid::CleanUpCUDAMemory()
{
    cudaFree(d_pressureForcePtr);
    cudaFree(d_viscousForcePtr);
    cudaFree(d_surfaceTensionForcePtr);
    cudaFree(d_gravityForcePtr);
    cudaFree(d_externalForcePtr);
    cudaFree(d_totalForcePtr);
    cudaFree(d_particleHashIdPtr);
    cudaFree(d_cellOccupancyPtr);
    cudaFree(d_cellParticleIdxPtr);
}

void Fluid::CleanUpGL()
{
    cudaGraphicsUnregisterResource(m_posBO_CUDA);
    m_posBO.destroy();

    cudaGraphicsUnregisterResource(m_velBO_CUDA);
    m_velBO.destroy();

    cudaGraphicsUnregisterResource(m_denBO_CUDA);
    m_denBO.destroy();

    cudaGraphicsUnregisterResource(m_massBO_CUDA);
    m_massBO.destroy();

    cudaGraphicsUnregisterResource(m_pressBO_CUDA);
    m_pressBO.destroy();
}

//------------------------------------------------------------------------

void Fluid::MapCudaGLResources()
{
    GetPositionPtr();
    GetVelocityPtr();
    GetDensityPtr();
    GetMassPtr();
    GetPressurePtr();
}

void Fluid::ReleaseCudaGLResources()
{
    ReleasePositionPtr();
    ReleaseVelocityPtr();
    ReleaseDensityPtr();
    ReleaseMassPtr();
    ReleasePressurePtr();
}

//------------------------------------------------------------------------

float3 *Fluid::GetViscForcePtr()
{
    return d_viscousForcePtr;
}

void Fluid::ReleaseViscForcePtr()
{

}

float3 *Fluid::GetSurfTenForcePtr()
{
    return d_surfaceTensionForcePtr;
}

void Fluid::ReleaseSurfTenForcePtr()
{

}

FluidProperty* Fluid::GetProperty()
{
    return m_fluidProperty.get();
}


