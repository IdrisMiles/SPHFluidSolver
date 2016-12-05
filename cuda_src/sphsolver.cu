
#include "../include/sphsolver.h"
#include "mathUtils.cu"
#include "smoothingKernels.cu"
#include <stdio.h>
#include <math.h>
#include <float.h>

#define NULL_HASH UINT_MAX

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------

__device__ float3 operator+(const float3 lhs, const float3 rhs)
{
    return make_float3(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
}

__device__ float3 operator-(const float3 lhs, const float3 rhs)
{
    return make_float3(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
}

__device__ float3 operator*(const float3 lhs, const float3 rhs)
{
    return make_float3(lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z);
}

__device__ float3 operator*(const float3 lhs, const float rhs)
{
    return make_float3(lhs.x*rhs, lhs.y*rhs, lhs.z*rhs);
}

__device__ float3 operator*(const float lhs, const float3 rhs)
{
    return make_float3(lhs*rhs.x, lhs*rhs.y, lhs*rhs.z);
}

__device__ float3 operator/(const float3 lhs, const float3 rhs)
{
    return make_float3(lhs.x/rhs.x, lhs.y/rhs.y, lhs.z/rhs.z);
}

__device__ float3 operator/(const float3 lhs, const float rhs)
{
    return make_float3(lhs.x/rhs, lhs.y/rhs, lhs.z/rhs);
}

__device__ float dot(const float3 lhs, const float3 rhs)
{
    return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

__device__ float dist(const float3 lhs, const float3 rhs)
{
    float3 vec = lhs - rhs;
    return sqrtf(dot(vec,vec));
}

__device__ float magnitude(const float3 vec)
{
    return sqrtf(dot(vec,vec));
}


__device__ float3 normalized(const float3 vec)
{
    float mag = magnitude(vec);
    if(fabs(mag) < FLT_EPSILON)
    {
        return vec;
    }
    else
    {
        return vec / mag;
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------

__device__ float SpikyKernel_Kernel(const float &_r, const float &_h)
{
    if(fabs(_r) > _h)
    {
        return 0;
    }
    else
    {
        return (15.0f/(CUDART_PI_F * pow(_h, 6))) * pow((_h-_r), 3);
    }
}

__device__ float SpikyKernelGradient_Kernel(const float &_r, const float &_h)
{
    if(fabs(_r) > 2.0f*_h)
    {
        return 0;
    }
    else
    {
        return - (45.0f/(CUDART_PI_F*pow(_h,6))) * pow((_h-_r), 2);
        //return (-1.0f/_h) * (45.0f/(CUDART_PI_F * pow(4.0f*_h, 3))) * (pow(2.0f - (_r/_h), 2));
    }
}

__device__ float3 SpikyKernelGradient_Kernel(const float3 _a, const float3 _b, const float _h)
{
    float3 dir = _a - _b;
    float distance = dist(_a, _b);
    float c = SpikyKernelGradient_Kernel(distance, _h);

    return (c * normalized(dir));
}

__device__ float Poly6Kernel_Kernel(const float &_r, const float &_h)
{
    if (fabs(_r) > 2.0f*_h)
    {
        return 0;
    }

    return (315.0f / (64*CUDART_PI_F*pow(_h,9))) * pow((pow(_h,2)- pow(_r,2)), 3);
}

__device__ float SplineGaussianKernel_Kernel(const float &_r, const float &_h)
{
    if(fabs(_r) > 2.0f*_h)
    {
        return 0;
    }
    else if(fabs(_r) > _h)
    {
        return (1/(CUDART_PI_F * _h)) * 0.25f * pow(2-(_r/_h), 3);
    }
    else
    {
        return (1/(CUDART_PI_F * _h)) * (1.0f - (1.5f*(pow((_r/_h), 2))) + (0.75f*(pow((_r/_h), 3))));
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*
__global__ void SPHSolve_Kernel(const uint maxCellOcc, const uint *cellOcc, const uint *cellPartIdx, float3 *particles, float3 *velocities, const uint numPoints, const uint gridRes, const float smoothingLength, const float dt)
{

    uint thisCellIdx = blockIdx.x + (blockIdx.y * gridDim.x) + (blockIdx.z * gridDim.x * gridDim.y);
    uint thisCellOcc = cellOcc[thisCellIdx];
    uint thisCellPartIdx = cellPartIdx[thisCellIdx];
    uint thisPartLocalIdx = threadIdx.x;
    uint thisPartGlobalIdx = threadIdx.x + thisCellPartIdx;

    const uint maxNeighCellParticles = 27*4;
    uint NULLCELL = (gridRes*gridRes*gridRes)+1;
    __shared__ uint s_neighCellIdx[27];
    __shared__ uint s_neighCellOcc[27];
    __shared__ uint s_neighCellPartIdx[27];
    __shared__ uint s_sharedNeighCellPartIdx[27];
    __shared__ float3 s_neighPositions[maxNeighCellParticles];
    __shared__ float3 s_neighVelocities[maxNeighCellParticles];
    __shared__ uint s_numNeighbours;


    //float3 thisPartNeighbours[maxNeighCellParticles];
    float3 thisParticle = particles[thisPartGlobalIdx];
    float3 thisParticleNew = make_float3(0.0f,0.0f,0.0f);
    float3 thisVelocity = velocities[thisPartGlobalIdx];
    float3 thisVelocityNew = make_float3(0.0f,0.0f,0.0f);


    // Sanity check
    if( thisCellIdx >= gridRes*gridRes*gridRes )
    {
        return;
    }


    // Assign shared info about neighbouring cells
    if (threadIdx.x < 27)
    {
        uint a = (threadIdx.x) / 9;
        uint b = (threadIdx.x - (a*9)) / 3;
        uint c = (threadIdx.x - (a*9) - (b*3));
        int IdxOffset = (c-1) + ((b-1)*gridRes) + ((a-1)*gridRes*gridRes);
        int neighCellIdx = thisCellIdx + IdxOffset;


        // Check for boundary conditions
        if(neighCellIdx >= 0 && neighCellIdx < gridRes*gridRes*gridRes)
        {
            s_neighCellIdx[threadIdx.x] = neighCellIdx;
            s_neighCellOcc[threadIdx.x] = cellOcc[neighCellIdx];
            s_neighCellPartIdx[threadIdx.x] = cellPartIdx[neighCellIdx];
        }
        else
        {
            s_neighCellIdx[threadIdx.x] = NULLCELL;
            s_neighCellOcc[threadIdx.x] = NULLCELL;
        }
    }


    __syncthreads();


    // Prefix-sum shared neighbouring cell particle index
    if(threadIdx.x == 0)
    {
        uint accIdx = 0;
        for(uint i=0;i<27;i++)
        {
            if(s_neighCellOcc[i] != NULLCELL && accIdx+s_neighCellOcc[i]<maxNeighCellParticles)
            {
                s_sharedNeighCellPartIdx[i] = accIdx;
                accIdx += s_neighCellOcc[i];
            }
            else
            {
                s_sharedNeighCellPartIdx[i] = NULLCELL;
            }
        }
        s_numNeighbours = accIdx;

    }


    __syncthreads();


    // Assign surounding particles to shared neighbours
    if (threadIdx.x < 27)
    {
        if(s_neighCellOcc[threadIdx.x] != NULLCELL)
        {
            for(uint i=0;i<s_neighCellOcc[threadIdx.x];i++)
            {
                if(s_sharedNeighCellPartIdx[threadIdx.x] != NULLCELL && s_sharedNeighCellPartIdx[threadIdx.x] + i < maxNeighCellParticles)
                {
                    s_neighPositions[s_sharedNeighCellPartIdx[threadIdx.x] + i] = particles[s_neighCellPartIdx[threadIdx.x] + i];
                    s_neighVelocities[s_sharedNeighCellPartIdx[threadIdx.x] + i] = velocities[s_neighCellPartIdx[threadIdx.x] + i];
               }
            }
        }
    }


    __syncthreads();


    // Sanity check
    if( thisPartLocalIdx >= thisCellOcc || thisPartGlobalIdx >= numPoints)
    {
        return;
    }



    // Time for some funky fluid stuff!




    float3 forceGravity = make_float3(0.0f, -9.1f, 0.0f);
    float3 forcePressure = make_float3(0.0f,0.0f,0.0f);

    //printf("%u\n", s_numNeighbours);
    //printf("%f, %f, %f \n",forcePressure.x,forcePressure.y,forcePressure.z);

    float3 forceTotal = forceGravity + forcePressure;



    thisVelocityNew = thisVelocity + (dt*forceTotal);
    thisParticleNew = thisParticle + (dt*thisVelocityNew);

    // Do SPH fluid stuff here
    float boundary = 0.5f * gridRes * smoothingLength;
    if(thisParticleNew.x < -boundary)
    {
       thisParticleNew.x = -boundary;// + (-boundary - thisParticleNew.x);
       thisVelocityNew = make_float3(0.0f,0.0f,0.0f);
    }
    if(thisParticleNew.x > boundary)
    {
       thisParticleNew.x = boundary;// + (boundary - thisParticleNew.x);
       thisVelocityNew = make_float3(0.0f,0.0f,0.0f);
    }

    if(thisParticleNew.y < -boundary)
    {
       thisParticleNew.y = -boundary;// + (-boundary - thisParticleNew.y);
       thisVelocityNew = make_float3(0.0f,0.0f,0.0f);
    }
    if(thisParticleNew.y > boundary)
    {
       thisParticleNew.y = boundary;// + (boundary - thisParticleNew.y);
       thisVelocityNew = make_float3(0.0f,0.0f,0.0f);
    }

    if(thisParticleNew.z < -boundary)
    {
       thisParticleNew.z = -boundary;// + (-boundary - thisParticleNew.z);
       thisVelocityNew = make_float3(0.0f,0.0f,0.0f);
    }
    if(thisParticleNew.z > boundary)
    {
       thisParticleNew.z = boundary;// + (boundary - thisParticleNew.z);
       thisVelocityNew = make_float3(0.0f,0.0f,0.0f);
    }


    __syncthreads();
    particles[thisPartGlobalIdx] = thisParticleNew;

}
*/

__device__ void HandleBoundaries(float3 &pos, float3 &vel, const float boundary)
{
    float boundaryDamper = 0.4f;

    if(pos.x < -boundary)
    {
       pos.x = -boundary + fabs(fabs(pos.x) - boundary);
       vel = make_float3(boundaryDamper*fabs(vel.x),vel.y,vel.z);
    }
    if(pos.x > boundary)
    {
       pos.x = boundary - fabs(fabs(pos.x) - boundary);
       vel = make_float3(-boundaryDamper*fabs(vel.x),vel.y,vel.z);
    }

    if(pos.y < -boundary)
    {
       pos.y = -boundary + fabs(fabs(pos.y) - boundary);
       vel = make_float3(vel.x,boundaryDamper*fabs(vel.y),vel.z);
    }
    if(pos.y > boundary)
    {
       pos.y = boundary - fabs(fabs(pos.y) - boundary);
       vel = make_float3(vel.x,-boundaryDamper*fabs(vel.y),vel.z);
    }

    if(pos.z < -boundary)
    {
       pos.z = -boundary + fabs(fabs(pos.z) - boundary);
       vel = make_float3(vel.x,vel.y,boundaryDamper*fabs(vel.z));
    }
    if(pos.z > boundary)
    {
       pos.z = boundary - fabs(fabs(pos.z) - boundary);
       vel = make_float3(vel.x,vel.y,-boundaryDamper*fabs(vel.z));
    }
}

__global__ void ParticleHash_Kernel(uint *hash, uint *cellOcc, const float3 *particles, const uint N, const uint gridRes, const float cellWidth)
{
    uint idx = (blockIdx.x * blockDim.x) + threadIdx.x;

    // Sanity check
    if (idx >= N)
    {
        return;
    }

    float gridDim = gridRes * cellWidth;
    float invGridDim = 1.0f / gridDim;
    float3 particle = particles[idx];
    uint hashID;

    // Get normalised particle positions [0-1]
    float normX = (particle.x + (0.5f * gridDim)) * invGridDim;
    float normY = (particle.y + (0.5f * gridDim)) * invGridDim;
    float normZ = (particle.z + (0.5f * gridDim)) * invGridDim;

    if(normX > 1.0f || normX < 0.0f ||
       normY > 1.0f || normY < 0.0f ||
       normZ > 1.0f || normZ < 0.0f )
    {
        hashID = 0;
    }
    else
    {
        // Get hash values for x, y, z
        uint hashX = floor(normX * gridRes);
        uint hashY = floor(normY * gridRes);
        uint hashZ = floor(normZ * gridRes);

        hashID = hashX + (hashY * gridRes) + (hashZ * gridRes * gridRes);
    }

    // Update hash id for this particle
    hash[idx] = hashID;


    // Update cell occupancy for the cell
    atomicAdd(&cellOcc[hashID], 1);


}

__global__ void ComputePressure_kernel(float *pressure, float *density, const float restDensity, const uint *cellOcc, const uint *cellPartIdx, const float3 *particles, const uint numPoints, const float smoothingLength)
{
    uint thisCellIdx = blockIdx.x + (blockIdx.y * gridDim.x) + (blockIdx.z * gridDim.x * gridDim.y);
    uint thisParticleGlobalIdx = cellPartIdx[thisCellIdx] + threadIdx.x;

    int neighCellIdx;
    int neighCellOcc;
    int neighCellPartIdx;
    int neighParticleGlobalIdx;

    int x, y, z;
    int neighLocalIdx;
    float accPressure;
    float accDensity;
    float thisDensity;
    //float densityRatio;

    if(thisParticleGlobalIdx < numPoints && threadIdx.x < cellOcc[thisCellIdx])
    {
        for(z = ((blockIdx.z==0)?0:-1); z < ((blockIdx.z==gridDim.z-1)?0:1); z++)
        {
            for(y = ((blockIdx.y==0)?0:-1); y < ((blockIdx.y==gridDim.y-1)?0:1); y++)
            {
                for(x = ((blockIdx.x==0)?0:-1); x < ((blockIdx.x==gridDim.x-1)?0:1); x++)
                {
                    neighCellIdx = thisCellIdx + x + (y*gridDim.x) + (z*gridDim.x*gridDim.y);
                    neighCellOcc = cellOcc[neighCellIdx];
                    neighCellPartIdx = cellPartIdx[neighCellIdx];
                    for(neighLocalIdx=0; neighLocalIdx<neighCellOcc; neighLocalIdx++)
                    {
                        neighParticleGlobalIdx = neighCellPartIdx + neighLocalIdx;
                        if(neighParticleGlobalIdx == thisParticleGlobalIdx){continue;}
                        float3 thisParticle = particles[thisParticleGlobalIdx];
                        float3 neighParticle = particles[neighParticleGlobalIdx];

                        thisDensity = Poly6Kernel_Kernel(dist(thisParticle, neighParticle), smoothingLength);
                        accDensity += thisDensity;

                        //densityRatio = thisDensity/restDensity;
                        //accPressure += (1.0f * ((float)pow(densityRatio, 7.0f) - 1.0f));
                    }
                }
            }
        }

        //accPressure = (1.0f * ((float)pow((accDensity/restDensity), 7.0f) - 1.0f));
        float k = 0.001f;
        float y = 1.0f;
        accPressure = k * (accDensity - restDensity);
        //accPressure = ((k*restDensity)/y) * ((float)pow((accDensity/restDensity), y) - 1.0f);

        density[thisParticleGlobalIdx] = accDensity;
        pressure[thisParticleGlobalIdx] = accPressure;

        if(isnan(accDensity))
        {
            printf("nan density \n");
        }

        if(isnan(accPressure))
        {
            printf("nan pressure \n");
        }
    }

}

__global__ void ComputePressureForce_kernel(float3 *pressureForce, const float *pressure, const float *density, const float *mass, const float3 *particles, const uint *cellOcc, const uint *cellPartIdx, const uint numPoints, const float smoothingLength)
{
    uint thisCellIdx = blockIdx.x + (blockIdx.y * gridDim.x) + (blockIdx.z * gridDim.x * gridDim.y);
    uint thisParticleGlobalIdx = cellPartIdx[thisCellIdx] + threadIdx.x;


    if(thisParticleGlobalIdx < numPoints && threadIdx.x < cellOcc[thisCellIdx])
    {
        float thisPressure = pressure[thisParticleGlobalIdx];
        if(isnan(thisPressure))
        {
            printf("nan thisPressure \n");
        }

        float thisDensity = density[thisParticleGlobalIdx];
        if(isnan(thisDensity))
        {
            printf("nan thisDensity \n");
        }

        float thisPressOverDensity2 = fabs(thisDensity) < FLT_EPSILON ? thisPressure/(1000000.0f) : thisPressure / (thisDensity * thisDensity);
        if(isnan(thisPressOverDensity2))
        {
            printf("nan this press/den^2 \n");
        }

        float thisMass = mass[thisParticleGlobalIdx];

        int neighCellIdx;
        int neighCellOcc;
        int neighCellPartIdx;
        int neighParticleGlobalIdx;

        int x, y, z;
        int neighLocalIdx;
        float3 accPressureForce = make_float3(0.0f, 0.0f, 0.0f);


        float3 thisParticle = particles[thisParticleGlobalIdx];

        for(z = ((blockIdx.z==0)?0:-1); z < ((blockIdx.z==gridDim.z-1)?0:1); z++)
        {
            for(y = ((blockIdx.y==0)?0:-1); y < ((blockIdx.y==gridDim.y-1)?0:1); y++)
            {
                for(x = ((blockIdx.x==0)?0:-1); x < ((blockIdx.x==gridDim.x-1)?0:1); x++)
                {

                    neighCellIdx = (blockIdx.x + x) + ((blockIdx.y + y) * gridDim.x) + ((blockIdx.z + z) * gridDim.x * gridDim.y);;// thisCellIdx + x + (y*gridDim.x) + (z*gridDim.x*gridDim.y);
                    neighCellOcc = cellOcc[neighCellIdx];
                    neighCellPartIdx = cellPartIdx[neighCellIdx];

                    for(neighLocalIdx=0; neighLocalIdx<neighCellOcc; neighLocalIdx++)
                    {
                        neighParticleGlobalIdx = neighCellPartIdx + neighLocalIdx;
                        if(neighParticleGlobalIdx == thisParticleGlobalIdx){continue;}

                        float3 neighParticle = particles[neighParticleGlobalIdx];
                        float neighPressure = pressure[neighParticleGlobalIdx];
                        float neighDensity = density[neighParticleGlobalIdx];
                        float neighMass = mass[neighParticleGlobalIdx];

                        float pressOverDens = fabs(neighDensity)<FLT_EPSILON ? (thisPressure + neighPressure) / (2000.0f) : (thisPressure + neighPressure) / (2.0f* neighDensity);

                        accPressureForce = accPressureForce + (neighMass * pressOverDens * SpikyKernelGradient_Kernel(thisParticle, neighParticle, smoothingLength));

                    }
                }
            }
        }


        if(isnan(accPressureForce.x) || isnan(accPressureForce.y) || isnan(accPressureForce.z))
        {
            printf("nan accPressure\n");
        }

        pressureForce[thisParticleGlobalIdx] = -1.0f * accPressureForce;
    }
}

__global__ void ComputeForces_kernel(float3 *force, const float3 *externalForce, const float3 *pressureForce, const float *mass, const float3 *particles, const float3 *velocities, const uint *cellOcc, const uint *cellPartIdx, const uint numPoints, const float smoothingLength)
{
    uint thisCellIdx = blockIdx.x + (blockIdx.y * gridDim.x) + (blockIdx.z * gridDim.x * gridDim.y);
    uint thisParticleGlobalIdx = cellPartIdx[thisCellIdx] + threadIdx.x;

    if(thisParticleGlobalIdx < numPoints && threadIdx.x < cellOcc[thisCellIdx])
    {
        // re-initialise forces to zero
        float3 accForce = make_float3(0.0f, 0.0f, 0.0f);

        // Add external force
        accForce = accForce + externalForce[thisCellIdx];

        // Add pressure force
        float3 pf = pressureForce[thisParticleGlobalIdx];
        if(isnan(pf.x) || isnan(pf.y) || isnan(pf.z))
        {
            printf("nan Fp\n");
        }
        else
        {
            accForce = accForce + pressureForce[thisParticleGlobalIdx];
        }

        // Add Viscous force
        float3 viscForce = make_float3(0.0f, 0.0f, 0.0f);
        accForce = accForce + viscForce;


        // Set particle force
        force[thisParticleGlobalIdx] = accForce;
    }
}

__global__ void Integrate_kernel(float3 *force, float3 *particles, float3 *velocities, const float _dt, const uint numPoints)
{
    uint idx = threadIdx.x + (blockIdx.x * blockDim.x);

    if(idx < numPoints)
    {
        // Good old instable Euler integration - ONLY FOR TESTING
        float3 oldPos = particles[idx];
        float3 oldVel = velocities[idx];

        float3 newVel = oldVel + (_dt * force[idx]);
        float3 newPos = oldPos + (_dt * newVel);

        // TODO:
        // Verlet integration
        // RK4 integration


        // Handle boundaries
        float boundary = 0.5f * 32;// gridRes * smoothingLength;
        HandleBoundaries(newPos, newVel, boundary);

        if(isnan(newPos.x) || isnan(newPos.y) || isnan(newPos.z))
        {
            printf("nan pos\n");
        }

        if(isnan(newVel.x) || isnan(newVel.y) || isnan(newVel.z))
        {
            printf("nan vel\n");
        }

        velocities[idx] = newVel;
        particles[idx] = newPos;

    }
}

__global__ void InitParticleAsCube_Kernel(float3 *particles, float3 *velocities, const uint numParticles, const uint numPartsPerAxis, const float scale)
{

    uint x = threadIdx.x + (blockIdx.x * blockDim.x);
    uint y = threadIdx.y + (blockIdx.y * blockDim.y);
    uint z = threadIdx.z + (blockIdx.z * blockDim.z);
    uint idx = x + (y * numPartsPerAxis) + (z * numPartsPerAxis * numPartsPerAxis);

    if(x >= numPartsPerAxis || y >= numPartsPerAxis || z >= numPartsPerAxis || idx >= numParticles)
    {
        return;
    }

    float posX = scale * (x - (0.5f * numPartsPerAxis));
    float posY = scale * (y - (0.5f * numPartsPerAxis));
    float posZ = scale * (z - (0.5f * numPartsPerAxis));

    particles[idx] = make_float3(posX, posY, posZ);
    velocities[idx] = make_float3(0.0f, 0.0f, 0.0f);
}


uint iDivUp(uint a, uint b)
{
    uint c = a/b;
    c += (a%b == 0) ? 0: 1;
    return c;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------


SPHSolverGPU::SPHSolverGPU(FluidProperty *_fluidProperty)
{
    m_fluidProperty = _fluidProperty;

    d_mass.resize(m_fluidProperty->numParticles);
    d_densities.resize(m_fluidProperty->numParticles);
    d_pressures.resize(m_fluidProperty->numParticles);
    d_pressureForces.resize(m_fluidProperty->numParticles);
    d_externalForces.resize(m_fluidProperty->gridResolution*m_fluidProperty->gridResolution*m_fluidProperty->gridResolution);
    d_totalForces.resize(m_fluidProperty->numParticles);
    d_particleHashIds.resize(m_fluidProperty->numParticles);
    d_cellOccupancy.resize(m_fluidProperty->gridResolution*m_fluidProperty->gridResolution*m_fluidProperty->gridResolution);
    d_cellParticleIdx.resize(m_fluidProperty->gridResolution*m_fluidProperty->gridResolution*m_fluidProperty->gridResolution);

    d_mass_ptr = thrust::raw_pointer_cast(&d_mass[0]);
    d_densities_ptr = thrust::raw_pointer_cast(&d_densities[0]);
    d_pressures_ptr = thrust::raw_pointer_cast(&d_pressures[0]);
    d_pressureForces_ptr = thrust::raw_pointer_cast(&d_pressureForces[0]);
    d_externalForces_ptr = thrust::raw_pointer_cast(&d_externalForces[0]);
    d_totalForces_ptr = thrust::raw_pointer_cast(&d_totalForces[0]);
    d_particleHashIds_ptr = thrust::raw_pointer_cast(&d_particleHashIds[0]);
    d_cellOccupancy_ptr = thrust::raw_pointer_cast(d_cellOccupancy.data());
    d_cellParticleIdx_ptr = thrust::raw_pointer_cast(&d_cellParticleIdx[0]);

    m_threadsPerBlock = 1024;
    m_numBlock = iDivUp(m_fluidProperty->numParticles, m_threadsPerBlock);
}


SPHSolverGPU::~SPHSolverGPU()
{

}

void SPHSolverGPU::Init(thrust::device_vector<float3> &_d_particles)
{
    _d_particles.resize(m_fluidProperty->numParticles);
    thrust::fill(d_pressureForces.begin(), d_pressureForces.end(), make_float3(0.0f,0.0f,0.0f));
    thrust::fill(d_externalForces.begin(), d_externalForces.end(), make_float3(0.0f, -9.8f,0.0f));
    thrust::fill(d_totalForces.begin(), d_totalForces.end(), make_float3(0.0f,0.0f,0.0f));
    thrust::fill(d_densities.begin(), d_densities.end(), 0.0f);
    thrust::fill(d_pressures.begin(), d_pressures.end(), 0.0f);
    thrust::fill(d_mass.begin(), d_mass.end(), 1.0f);
    thrust::fill(d_particleHashIds.begin(), d_particleHashIds.end(), 0);
    thrust::fill(d_cellOccupancy.begin(), d_cellOccupancy.end(), 0);
    thrust::fill(d_cellParticleIdx.begin(), d_cellParticleIdx.end(), 0);
}


void SPHSolverGPU::Solve(float _dt, float3* _d_p, float3* _d_v)
{
    d_positions_ptr = _d_p;
    d_velocities_ptr = _d_v;

    thrust::fill(d_cellOccupancy.begin(), d_cellOccupancy.end(), 0);
    thrust::fill(d_externalForces.begin(), d_externalForces.end(), make_float3(0.0f, -9.8f,0.0f));
    thrust::fill(d_pressureForces.begin(), d_pressureForces.end(), make_float3(0.0f,0.0f,0.0f));
    thrust::fill(d_totalForces.begin(), d_totalForces.end(), make_float3(0.0f,0.0f,0.0f));
    thrust::fill(d_densities.begin(), d_densities.end(), 0.0f);
    thrust::fill(d_pressures.begin(), d_pressures.end(), 0.0f);
    thrust::fill(d_mass.begin(), d_mass.end(), 1.0f);

    cudaThreadSynchronize();

    // Get particle hash IDs
    ParticleHash(d_particleHashIds_ptr, d_cellOccupancy_ptr, d_positions_ptr, m_fluidProperty->numParticles, m_fluidProperty->gridResolution, m_fluidProperty->gridCellWidth);
    cudaThreadSynchronize();

    // Sort particles
    thrust::sort_by_key(d_particleHashIds.begin(), d_particleHashIds.end(), thrust::make_zip_iterator(thrust::make_tuple(thrust::device_pointer_cast(d_positions_ptr), thrust::device_pointer_cast(d_velocities_ptr))));

    // Get Cell particle indexes - scatter addresses
    thrust::exclusive_scan(d_cellOccupancy.begin(), d_cellOccupancy.end(), d_cellParticleIdx.begin());

    uint maxCellOcc = thrust::reduce(d_cellOccupancy.begin(), d_cellOccupancy.end(), 0, thrust::maximum<uint>());
    cudaThreadSynchronize();

    // Run SPH solver
    ComputePressure(maxCellOcc, d_pressures_ptr, d_densities_ptr, m_fluidProperty->restDensity, d_cellOccupancy_ptr, d_cellParticleIdx_ptr, d_positions_ptr, m_fluidProperty->numParticles, m_fluidProperty->smoothingLength);
    cudaThreadSynchronize();

    ComputePressureForce(maxCellOcc, d_pressureForces_ptr, d_pressures_ptr, d_densities_ptr, d_mass_ptr, d_positions_ptr, d_cellOccupancy_ptr, d_cellParticleIdx_ptr, m_fluidProperty->numParticles, m_fluidProperty->smoothingLength);
    cudaThreadSynchronize();

    ComputeTotalForce(maxCellOcc, d_totalForces_ptr, d_externalForces_ptr, d_pressureForces_ptr, d_mass_ptr, d_positions_ptr, d_velocities_ptr,d_cellOccupancy_ptr, d_cellParticleIdx_ptr, m_fluidProperty->numParticles, m_fluidProperty->smoothingLength);
    cudaThreadSynchronize();

    Integrate(maxCellOcc, d_totalForces_ptr, d_positions_ptr, d_velocities_ptr, _dt, m_fluidProperty->numParticles);
    cudaThreadSynchronize();


    /*
    std::cout << "\n\nHash: \n";
    thrust::copy(d_particleHashIds.begin(), d_particleHashIds.end(), std::ostream_iterator<uint>(std::cout, " "));
    std::cout << "\n\nOccupancy: \n";
    thrust::copy(d_cellOccupancy.begin(), d_cellOccupancy.end(), std::ostream_iterator<uint>(std::cout, " "));
    std::cout << "\n\nCell particle Ids: \n";
    thrust::copy(d_cellParticleIdx.begin(), d_cellParticleIdx.end(), std::ostream_iterator<uint>(std::cout, " "));
    std::cout << "\n\nMax Cell Occ: "<<maxCellOcc<<"\n";
    std::cout << "\n\nPressure: \n";
    thrust::copy(d_pressures.begin(), d_pressures.end(), std::ostream_iterator<float>(std::cout, " "));
    */

}


void SPHSolverGPU::ParticleHash(uint *hash, uint *cellOcc, float3 *particles, const uint N, const uint gridRes, const float cellWidth)
{
    ParticleHash_Kernel<<<m_numBlock, m_threadsPerBlock>>>(hash, cellOcc, particles, N, gridRes, cellWidth);
}

void SPHSolverGPU::ComputePressure(const uint maxCellOcc, float *pressure, float *density, const float restDensity, const uint *cellOcc, const uint *cellPartIdx, const float3 *particles, const uint numPoints, const float smoothingLength)
{
    dim3 gridDim = dim3(m_fluidProperty->gridResolution, m_fluidProperty->gridResolution, m_fluidProperty->gridResolution);
    uint blockSize = ((maxCellOcc / 32)) * 32 + (maxCellOcc % 32)?32:0;

    ComputePressure_kernel<<<gridDim, blockSize>>>(pressure, density, restDensity, cellOcc, cellPartIdx, particles, numPoints, smoothingLength);
}

void SPHSolverGPU::ComputePressureForce(const uint maxCellOcc, float3 *pressureForce, const float *pressure, const float *density, const float *mass, const float3 *particles, const uint *cellOcc, const uint *cellPartIdx, const uint numPoints, const float smoothingLength)
{
    dim3 gridDim = dim3(m_fluidProperty->gridResolution, m_fluidProperty->gridResolution, m_fluidProperty->gridResolution);
    uint blockSize = ((maxCellOcc / 32)) * 32 + (maxCellOcc % 32)?32:0;

    ComputePressureForce_kernel<<<gridDim, blockSize>>>(pressureForce, pressure, density, mass, particles, cellOcc, cellPartIdx, numPoints, smoothingLength);
}

void SPHSolverGPU::ComputeTotalForce(const uint maxCellOcc, float3 *force, const float3 *externalForce, const float3 *pressureForce, const float *mass, const float3 *particles, const float3 *velocities, const uint *cellOcc, const uint *cellPartIdx, const uint numPoints, const float smoothingLength)
{
    dim3 gridDim = dim3(m_fluidProperty->gridResolution, m_fluidProperty->gridResolution, m_fluidProperty->gridResolution);
    uint blockSize = ((maxCellOcc / 32)) * 32 + (maxCellOcc % 32)?32:0;

    ComputeForces_kernel<<<gridDim, blockSize>>>(force, externalForce, pressureForce, mass, particles, velocities, cellOcc, cellPartIdx, numPoints, smoothingLength);
}

void SPHSolverGPU::Integrate(const uint maxCellOcc, float3 *force, float3 *particles, float3 *velocities, const float _dt, const uint numPoints)
{
    uint threadsPerBlock = 1024;
    uint numBlocks = iDivUp(numPoints, threadsPerBlock);

    Integrate_kernel<<<numBlocks, threadsPerBlock>>>(force, particles, velocities, _dt, numPoints);
}


void SPHSolverGPU::SPHSolve(const uint maxCellOcc, const uint *cellOcc, const uint *cellIds, float3 *particles, float3 *velocities, const uint numPoints, const uint gridRes, const float smoothingLength, const float dt)
{
    //dim3 gridDim = dim3(m_fluidProperty->gridResolution, m_fluidProperty->gridResolution, m_fluidProperty->gridResolution);
    //uint blockSize = ((maxCellOcc / 32)) * 32 + (maxCellOcc % 32)?32:0;

    //SPHSolve_Kernel<<<gridDim, blockSize>>>(maxCellOcc, cellOcc, cellIds, particles, velocities, numPoints, gridRes, smoothingLength, dt);
}

void SPHSolverGPU::InitParticleAsCube(float3 *particles, float3 *velocities, const uint numParticles, const uint numPartsPerAxis, const float scale)
{

    uint threadsPerBlock = 8;
    dim3 blockDim(threadsPerBlock,threadsPerBlock,threadsPerBlock);
    uint blocksPerGrid = iDivUp(numPartsPerAxis,threadsPerBlock);
    dim3 gridDim(blocksPerGrid, blocksPerGrid, blocksPerGrid);

    InitParticleAsCube_Kernel<<<gridDim, blockDim>>>(particles, velocities, numParticles, numPartsPerAxis, scale);

}


