#include "include/SPH/algae.h"

#include <math.h>
#include <glm/gtx/transform.hpp>


Algae::Algae(std::shared_ptr<AlgaeProperty> _property, std::string _name):
    BaseSphParticle(_property, _name),
    m_property(_property)
{
    m_positionMapped = false;
    m_velocityMapped = false;
    m_densityMapped = false;
    m_pressureMapped = false;
    m_illumMapped = false;

    m_setupSolveSpecsInit = false;

    Init();
}

//--------------------------------------------------------------------------------------------------------------------

Algae::Algae(std::shared_ptr<AlgaeProperty> _property, Mesh _mesh, std::string _name):
    BaseSphParticle(_property, _name),
    m_property(_property)
{
    m_mesh = _mesh;

    m_positionMapped = false;
    m_velocityMapped = false;
    m_densityMapped = false;
    m_pressureMapped = false;
    m_illumMapped = false;

    Init();
    InitAlgaeAsMesh();
}

//--------------------------------------------------------------------------------------------------------------------

Algae::~Algae()
{
    m_property = nullptr;
    CleanUp();
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::SetupSolveSpecs(const FluidSolverProperty &_solverProps)
{
    if(m_setupSolveSpecsInit)
    {
        checkCudaErrorsMsg(cudaFree(d_cellOccupancyPtr),"");
        checkCudaErrorsMsg(cudaFree(d_cellParticleIdxPtr),"");

        m_setupSolveSpecsInit = false;
    }

    const uint numCells = _solverProps.gridResolution * _solverProps.gridResolution * _solverProps.gridResolution;
    checkCudaErrorsMsg(cudaMalloc(&d_cellOccupancyPtr, numCells * sizeof(unsigned int)),"");
    checkCudaErrorsMsg(cudaMalloc(&d_cellParticleIdxPtr, numCells * sizeof(unsigned int)),"");


    getLastCudaError("SetUpSolveSpecs Algae");

    m_setupSolveSpecsInit = true;
}

//--------------------------------------------------------------------------------------------------------------------

AlgaeProperty *Algae::GetProperty()
{
    return m_property.get();
}

//---------------------------------------------------------------------------------------------------------------

void Algae::SetProperty(AlgaeProperty _property)
{
    m_property->gravity = _property.gravity;
    m_property->particleMass = _property.particleMass;
    m_property->particleRadius = _property.particleRadius;
    m_property->restDensity = _property.restDensity;
    m_property->numParticles = _property.numParticles;

    m_property->bioluminescenceThreshold = _property.bioluminescenceThreshold;
    m_property->reactionRate = _property.reactionRate;
    m_property->deactionRate = _property.deactionRate;

    UpdateCUDAMemory();
}
//---------------------------------------------------------------------------------------------------------------

AlgaeGpuData Algae::GetAlgaeGpuData()
{
    AlgaeGpuData particle;
    particle.pos = GetPositionPtr();
    particle.vel = GetVelocityPtr();
    particle.den = GetDensityPtr();
    particle.pressure = GetPressurePtr();

    particle.pressureForce = GetPressureForcePtr();
    particle.gravityForce = GetGravityForcePtr();
    particle.externalForce = GetExternalForcePtr();
    particle.totalForce = GetTotalForcePtr();

    particle.id = GetParticleIdPtr();
    particle.hash = GetParticleHashIdPtr();
    particle.cellOcc = GetCellOccupancyPtr();
    particle.cellPartIdx = GetCellParticleIdxPtr();

    particle.maxCellOcc = GetMaxCellOcc();

    particle.gravity = m_property->gravity;
    particle.mass = m_property->particleMass;
    particle.restDen = m_property->restDensity;
    particle.radius = m_property->particleRadius;
    particle.smoothingLength = m_property->smoothingLength;
    particle.numParticles = m_property->numParticles;

    particle.prevPressure = GetPrevPressurePtr();
    particle.illum = GetIlluminationPtr();

    particle.bioluminescenceThreshold = m_property->bioluminescenceThreshold;
    particle.reactionRate = m_property->reactionRate;
    particle.deactionRate = m_property->deactionRate;

    return particle;
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::MapCudaGLResources()
{
    GetPositionPtr();
    GetVelocityPtr();
    GetDensityPtr();
    GetPressurePtr();
    GetIlluminationPtr();
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::ReleaseCudaGLResources()
{
    ReleasePositionPtr();
    ReleaseVelocityPtr();
    ReleaseDensityPtr();
    ReleasePressurePtr();
    ReleaseIlluminationPtr();
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::Init()
{
    cudaSetDevice(0);

    InitGL();
    InitCUDAMemory();

    m_init = true;

}

//--------------------------------------------------------------------------------------------------------------------

void Algae::InitCUDAMemory()
{

    // particle properties
    cudaGraphicsGLRegisterBuffer(&m_posBO_CUDA, m_posBO.bufferId(),cudaGraphicsMapFlagsNone);
    cudaGraphicsGLRegisterBuffer(&m_velBO_CUDA, m_velBO.bufferId(),cudaGraphicsMapFlagsNone);
    cudaGraphicsGLRegisterBuffer(&m_denBO_CUDA, m_denBO.bufferId(),cudaGraphicsMapFlagsWriteDiscard);
    cudaGraphicsGLRegisterBuffer(&m_pressBO_CUDA, m_pressBO.bufferId(),cudaGraphicsMapFlagsWriteDiscard);
    cudaGraphicsGLRegisterBuffer(&m_illumBO_CUDA, m_illumBO.bufferId(),cudaGraphicsMapFlagsNone);//WriteDiscard);

    // particle forces
    cudaMalloc(&d_pressureForcePtr, m_property->numParticles * sizeof(float3));
    cudaMalloc(&d_gravityForcePtr, m_property->numParticles * sizeof(float3));
    cudaMalloc(&d_externalForcePtr, m_property->numParticles * sizeof(float3));
    cudaMalloc(&d_totalForcePtr, m_property->numParticles * sizeof(float3));

    cudaMalloc(&d_particleHashIdPtr, m_property->numParticles * sizeof(unsigned int));
    cudaMalloc(&d_particleIdPtr, m_property->numParticles * sizeof(unsigned int));

    cudaMalloc(&d_prevPressurePtr, m_property->numParticles * sizeof(float));
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::InitGL()
{
    InitVAO();
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::InitVAO()
{

    // Setup our pos buffer object.
    m_posBO.create();
    m_posBO.bind();
    m_posBO.allocate(m_property->numParticles * sizeof(float3));
    m_posBO.release();


    // Set up velocity buffer object
    m_velBO.create();
    m_velBO.bind();
    m_velBO.allocate(m_property->numParticles * sizeof(float3));
    m_velBO.release();


    // Set up density buffer object
    m_denBO.create();
    m_denBO.bind();
    m_denBO.allocate(m_property->numParticles * sizeof(float));
    m_denBO.release();


    // Set up pressure buffer object
    m_pressBO.create();
    m_pressBO.bind();
    m_pressBO.allocate(m_property->numParticles * sizeof(float));
    m_pressBO.release();


    // Set up illum buffer object
    m_illumBO.create();
    m_illumBO.bind();
    m_illumBO.allocate(m_property->numParticles * sizeof(float));
    m_illumBO.release();
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::InitAlgaeAsMesh()
{
    GetPositionPtr();
    cudaMemcpy(d_positionPtr, &m_mesh.verts[0], m_property->numParticles * sizeof(float3), cudaMemcpyHostToDevice);
    ReleaseCudaGLResources();
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::CleanUpCUDAMemory()
{
    cudaFree(d_prevPressurePtr);
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::CleanUpGL()
{
    cudaGraphicsUnregisterResource(m_illumBO_CUDA);
    m_illumBO.destroy();
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::UpdateCUDAMemory()
{
    // delete memory
    checkCudaErrorsMsg(cudaFree(d_pressureForcePtr),"");
    checkCudaErrorsMsg(cudaFree(d_gravityForcePtr),"");
    checkCudaErrorsMsg(cudaFree(d_externalForcePtr),"");
    checkCudaErrorsMsg(cudaFree(d_totalForcePtr),"");

    checkCudaErrorsMsg(cudaFree(d_particleHashIdPtr),"");
    checkCudaErrorsMsg(cudaFree(d_particleIdPtr),"");

    checkCudaErrorsMsg(cudaFree(d_prevPressurePtr),"");


    // re allocate memory
    checkCudaErrorsMsg(cudaMalloc(&d_pressureForcePtr, m_property->numParticles * sizeof(float3)),"");
    checkCudaErrorsMsg(cudaMalloc(&d_gravityForcePtr, m_property->numParticles * sizeof(float3)),"");
    checkCudaErrorsMsg(cudaMalloc(&d_externalForcePtr, m_property->numParticles * sizeof(float3)),"");
    checkCudaErrorsMsg(cudaMalloc(&d_totalForcePtr, m_property->numParticles * sizeof(float3)),"");

    checkCudaErrorsMsg(cudaMalloc(&d_particleHashIdPtr, m_property->numParticles * sizeof(unsigned int)),"");
    checkCudaErrorsMsg(cudaMalloc(&d_particleIdPtr, m_property->numParticles * sizeof(unsigned int)), "");

    checkCudaErrorsMsg(cudaMalloc(&d_prevPressurePtr, m_property->numParticles * sizeof(float)), "");


    // Setup our pos buffer object.
    m_posBO.bind();
    m_posBO.allocate(m_property->numParticles * sizeof(float3));
    m_posBO.release();

    // Set up velocity buffer object
    m_velBO.bind();
    m_velBO.allocate(m_property->numParticles * sizeof(float3));
    m_velBO.release();

    // Set up density buffer object
    m_denBO.bind();
    m_denBO.allocate(m_property->numParticles * sizeof(float));
    m_denBO.release();


    // Set up pressure buffer object
    m_pressBO.bind();
    m_pressBO.allocate(m_property->numParticles * sizeof(float));
    m_pressBO.release();

    // Set up bioluminous buffer object
    m_illumBO.bind();
    m_illumBO.allocate(m_property->numParticles * sizeof(float));
    m_illumBO.release();
}

//--------------------------------------------------------------------------------------------------------------------


float *Algae::GetPrevPressurePtr()
{
    return d_prevPressurePtr;
}

//--------------------------------------------------------------------------------------------------------------------


float *Algae::GetIlluminationPtr()
{
    if(!m_illumMapped)
    {
        size_t numBytesIllum;
        cudaGraphicsMapResources(1, &m_illumBO_CUDA, 0);
        cudaGraphicsResourceGetMappedPointer((void **)&d_illumPtr, &numBytesIllum, m_illumBO_CUDA);

        m_illumMapped = true;
    }

    return d_illumPtr;
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::ReleaseIlluminationPtr()
{
    if(m_illumMapped)
    {
        cudaGraphicsUnmapResources(1, &m_illumBO_CUDA, 0);
        m_illumMapped = false;
    }
}

//--------------------------------------------------------------------------------------------------------------------

QOpenGLBuffer &Algae::GetIllumBO()
{
    return m_illumBO;
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::GetBioluminescentIntensities(std::vector<float> &_bio)
{
    if(!m_init || this->m_property == nullptr)
    {
        return;
    }

    _bio.resize(this->m_property->numParticles);
    checkCudaErrors(cudaMemcpy(&_bio[0], GetIlluminationPtr(), this->m_property->numParticles * sizeof(float), cudaMemcpyDeviceToHost));
    ReleaseIlluminationPtr();
}

//--------------------------------------------------------------------------------------------------------------------

void Algae::SetBioluminescentIntensities(const std::vector<float> &_bio)
{
    assert(_bio.size() == m_property->numParticles);

    cudaMemcpy(GetIlluminationPtr(), &_bio[0], m_property->numParticles * sizeof(float), cudaMemcpyHostToDevice);
    ReleaseIlluminationPtr();
}

//--------------------------------------------------------------------------------------------------------------------
