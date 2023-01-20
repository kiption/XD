//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

struct Viewport
{
    float left;
    float top;
    float right;
    float bottom;
};

// 사용할 광선에 대한 상수 버퍼 구조체 
struct RayGenConstantBuffer
{
    Viewport viewport;
    Viewport stencil;
};

#endif // RAYTRACINGHLSLCOMPAT_H