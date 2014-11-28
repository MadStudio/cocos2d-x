/**
Copyright 2013 BlackBerry Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 
Original file from GamePlay3D: http://gameplay3d.org

This file was modified to fit the cocos2d-x project
*/

#include "MathUtil.h"
#include "base/ccMacros.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <cpu-features.h>
#endif

//#define USE_NEON32        : neon 32 code will be used
//#define USE_NEON64        : neon 64 code will be used
//#define INCLUDE_NEON32    : neon 32 code included
//#define INCLUDE_NEON64    : neon 64 code included
//#define USE_SSE           : SSE code used
//#define INCLUDE_SSE       : SSE code included

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    #if defined (__arm64__)
    #define USE_NEON64
    #define INCLUDE_NEON64
    #elif defined (__ARM_NEON__)
    #define USE_NEON32
    #define INCLUDE_NEON32
    #else
    #endif
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    #if defined (__arm64__)
    #define INCLUDE_NEON64
    #elif defined (__ARM_NEON__)
    #define INCLUDE_NEON32
    #else
    #endif
#else

#endif

#if defined (__SSE__)
#define USE_SSE
#define INCLUDE_SSE
#endif

#ifdef INCLUDE_NEON32
#include "MathUtilNeon.inl"
#endif

#ifdef INCLUDE_NEON64
#include "MathUtilNeon64.inl"
#endif

#ifdef INCLUDE_SSE
#include "MathUtilSSE.inl"
#endif

#include "MathUtil.inl"

NS_CC_MATH_BEGIN

void MathUtil::smooth(float* x, float target, float elapsedTime, float responseTime)
{
    GP_ASSERT(x);
    
    if (elapsedTime > 0)
    {
        *x += (target - *x) * elapsedTime / (elapsedTime + responseTime);
    }
}

void MathUtil::smooth(float* x, float target, float elapsedTime, float riseTime, float fallTime)
{
    GP_ASSERT(x);
    
    if (elapsedTime > 0)
    {
        float delta = target - *x;
        *x += delta * elapsedTime / (elapsedTime + (delta > 0 ? riseTime : fallTime));
    }
}

/*
 *md: 20141127 normalize a given vector
 *io: v  vector to be normalized
*/

void MathUtil::normalize(float *v)
{
    float d = sqrtf(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    d = 1.0f/d;
    v[0] *= d;
    v[1] *= d;
    v[2] *= d;
}


/*
 *md: 20141127 caculate normal vector(normalized) for a triangle
 *in: v1,v2,v3  vertexes for the triangle
 *out:normal    normalized normal vector
 */
void MathUtil::triangleNormal(const float *v1, const float *v2, const float *v3, float *normal)
{
    float d1[3],d2[3];
    for (int i = 0; i<3; i++) {
        d1[i] = v1[i] - v2[i];
        d2[i] = v2[i] - v3[i];
    }
    
    normal[0] = d1[1]*d2[2] - d1[2]*d1[1];
    normal[1] = d1[2]*d2[0] - d1[0]*d1[2];
    normal[2] = d1[0]*d2[1] - d1[1]*d1[0];
    
    normalize(normal);
    
}

/*
 *md: 20141128 caculate flat vertexnormal by averaging normals of faces that vetex in
 *in: vx array contains all vertexes
 *in: vxN num of vertexes
 *in: idx array contains indices of vertexes to make a triangle face
 *in: idxN num of indices
 *out:normals   normalized normal vector for all vertexes
 */

void MathUtil::flatVertexNormal(const float *vx,const unsigned short vxN,const unsigned short *idx,const unsigned short idxN, float *normals)
{
    unsigned short idx1,idx2,idx3 = 0;
    float nor[3];
    unsigned short average[vxN];
    float av = 0.0f;
    for (unsigned short i=0; i<vxN; i++) {
        average[i] = 0;
    }
    
    for (unsigned short i =0; i<idxN; i+=3) {
        idx1 = idx[i];
        idx2 = idx[i+1];
        idx3 = idx[i+2];
        average[idx1]++;
        average[idx2]++;
        average[idx3]++;
        triangleNormal(&vx[idx1*3], &vx[idx2*3], &vx[idx3*3], nor);
        normals[idx1*3] += nor[0];
        normals[idx1*3+1] += nor[1];
        normals[idx1*3+2] += nor[2];
        normals[idx2*3] += nor[0];
        normals[idx2*3+1] += nor[1];
        normals[idx2*3+2] += nor[2];
        normals[idx3*3] += nor[0];
        normals[idx3*3+1] += nor[1];
        normals[idx3*3+2] += nor[2];
    }
    
    for (unsigned short i=0; i<vxN; i++) {
        av = 1.0f/(float)average[i];
        normals[i*3] *= av;
        normals[i*3+1] *= av;
        normals[i*3+2] *= av;
        normalize(&normals[i*3]);
    }
    
}

bool MathUtil::isNeon32Enabled()
{
#ifdef USE_NEON32
    return true;
#elif (defined (INCLUDE_NEON32) && (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) )
    class AnrdoidNeonChecker
    {
    public:
        AnrdoidNeonChecker()
        {
            if (android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM && (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0)
                _isNeonEnabled = true;
            else
                _isNeonEnabled = false;
        }
        bool isNeonEnabled() const { return _isNeonEnabled; }
    private:
        bool _isNeonEnabled;
    };
    static AnrdoidNeonChecker checker;
    return checker.isNeonEnabled();
#else
    return false;
#endif
}

bool MathUtil::isNeon64Enabled()
{
#ifdef USE_NEON64
    return true;
#else
    return false;
#endif
}

void MathUtil::addMatrix(const float* m, float scalar, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::addMatrix(m, scalar, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::addMatrix(m, scalar, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::addMatrix(m, scalar, dst);
    else MathUtilC::addMatrix(m, scalar, dst);
#else
    MathUtilC::addMatrix(m, scalar, dst);
#endif
}

void MathUtil::addMatrix(const float* m1, const float* m2, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::addMatrix(m1, m2, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::addMatrix(m1, m2, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::addMatrix(m1, m2, dst);
    else MathUtilC::addMatrix(m1, m2, dst);
#else
    MathUtilC::addMatrix(m1, m2, dst);
#endif
}

void MathUtil::subtractMatrix(const float* m1, const float* m2, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::subtractMatrix(m1, m2, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::subtractMatrix(m1, m2, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::subtractMatrix(m1, m2, dst);
    else MathUtilC::subtractMatrix(m1, m2, dst);
#else
    MathUtilC::subtractMatrix(m1, m2, dst);
#endif
}

void MathUtil::multiplyMatrix(const float* m, float scalar, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::multiplyMatrix(m, scalar, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::multiplyMatrix(m, scalar, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::multiplyMatrix(m, scalar, dst);
    else MathUtilC::multiplyMatrix(m, scalar, dst);
#else
    MathUtilC::multiplyMatrix(m, scalar, dst);
#endif
}

void MathUtil::multiplyMatrix(const float* m1, const float* m2, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::multiplyMatrix(m1, m2, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::multiplyMatrix(m1, m2, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::multiplyMatrix(m1, m2, dst);
    else MathUtilC::multiplyMatrix(m1, m2, dst);
#else
    MathUtilC::multiplyMatrix(m1, m2, dst);
#endif
}

void MathUtil::negateMatrix(const float* m, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::negateMatrix(m, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::negateMatrix(m, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::negateMatrix(m, dst);
    else MathUtilC::negateMatrix(m, dst);
#else
    MathUtilC::negateMatrix(m, dst);
#endif
}

void MathUtil::transposeMatrix(const float* m, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::transposeMatrix(m, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::transposeMatrix(m, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::transposeMatrix(m, dst);
    else MathUtilC::transposeMatrix(m, dst);
#else
    MathUtilC::transposeMatrix(m, dst);
#endif
}

void MathUtil::transformVec4(const float* m, float x, float y, float z, float w, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::transformVec4(m, x, y, z, w, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::transformVec4(m, x, y, z, w, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::transformVec4(m, x, y, z, w, dst);
    else MathUtilC::transformVec4(m, x, y, z, w, dst);
#else
    MathUtilC::transformVec4(m, x, y, z, w, dst);
#endif
}

void MathUtil::transformVec4(const float* m, const float* v, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::transformVec4(m, v, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::transformVec4(m, v, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::transformVec4(m, v, dst);
    else MathUtilC::transformVec4(m, v, dst);
#else
    MathUtilC::transformVec4(m, v, dst);
#endif
}

void MathUtil::crossVec3(const float* v1, const float* v2, float* dst)
{
#ifdef USE_NEON32
    MathUtilNeon::crossVec3(v1, v2, dst);
#elif defined (USE_NEON64)
    MathUtilNeon64::crossVec3(v1, v2, dst);
#elif defined (INCLUDE_NEON32)
    if(isNeon32Enabled()) MathUtilNeon::crossVec3(v1, v2, dst);
    else MathUtilC::crossVec3(v1, v2, dst);
#else
    MathUtilC::crossVec3(v1, v2, dst);
#endif
}

NS_CC_MATH_END
