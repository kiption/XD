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

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "RaytracingHlslCompat.h"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ConstantBuffer<RayGenConstantBuffer> g_rayGenCB : register(b0);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
};

bool IsInsideViewport(float2 p, Viewport viewport)
{
    return (p.x >= viewport.left && p.x <= viewport.right)
        && (p.y >= viewport.top && p.y <= viewport.bottom);
}

// 광선 생성 쉐이더 
// 광선 생성 셰이더 는 광선을 생성하기 위해 TraceRay() 를 호출합니다
[shader("raygeneration")]
void MyRaygenShader()
{
    float2 lerpValues = (float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions();

    // Orthographic projection since we're raytracing in screen space.
    // 화면 공간에서 Ray하기에 직교투영한다.
    float3 rayDir = float3(0, 0, 1);
    // 뷰포트의 4개의 값을, 각각 x,y 계수에 선형보간한다
    float3 origin = float3(
        lerp(g_rayGenCB.viewport.left, g_rayGenCB.viewport.right, lerpValues.x),
        lerp(g_rayGenCB.viewport.top, g_rayGenCB.viewport.bottom, lerpValues.y),
        0.0f);

    // 투영된 공간의 좌표가 뷰포트에 존재한다면 
    // 스텐실 창 내부에서 보간된 DispatchRaysIndex을 렌더링
    if (IsInsideViewport(origin.xy, g_rayGenCB.stencil))
    {
        // Trace the ray.
        // Set the ray's extents.
        RayDesc ray;
        ray.Origin = origin;
        ray.Direction = rayDir;
        // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
        
        // TMin should be kept small to prevent missing geometry at close contact areas.
        
        // 부동 소수점 오류로 인한 앨리어싱 문제를 방지하려면 완전한 0이 아닌 작은 값으로 설정
        // 이렇게 작게 유지하는 이유는 광선이 가까운 영역에 접촉했을 때, 형상이 일그러지지 않게 하기 위함.
        ray.TMin = 0.001;
        ray.TMax = 10000.0;

        // 색상 탑재량 초기화
        RayPayload payload = { float4(0, 0, 0, 0) };
        // TraceRay : 가속 구조에서 적격을 검색하기 위해 광선을 보내는 API 함수
        TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

        // Write the raytraced color to the output texture.
        // 레이트레이싱된 색상을 출력 텍스처에 씁니다.
        RenderTarget[DispatchRaysIndex().xy] = payload.color;
    }
    else
    {
        // Render interpolated DispatchRaysIndex outside the stencil window
        // 스텐실 창 외부에서 보간된 DispatchRaysIndex 렌더링
        RenderTarget[DispatchRaysIndex().xy] = float4(lerpValues, 0, 1);
    }
}

// 히트 적중 쉐이더
[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    // 무게중심
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    // 유효 색상 탑재량
    payload.color = float4(barycentrics, 1);
}

// 히트 미스 쉐이더
[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    // 광선에 맞지 않았을 때, 저장해둘 색상 값
    payload.color = float4(0, 0, 0, 1);
}

#endif // RAYTRACING_HLSL