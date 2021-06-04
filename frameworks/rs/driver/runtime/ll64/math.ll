target datalayout = "e-m:e-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-linux-android"

declare float @llvm.sqrt.f32(float)
declare float @llvm.pow.f32(float, float)
declare float @llvm.fabs.f32(float)
declare <2 x float> @llvm.fabs.v2f32(<2 x float>)
declare <3 x float> @llvm.fabs.v3f32(<3 x float>)
declare <4 x float> @llvm.fabs.v4f32(<4 x float>)

define float @_Z4sqrtf(float %v) nounwind readnone alwaysinline {
  %1 = tail call float @llvm.sqrt.f32(float %v)
  ret float %1
}

define float @_Z3powff(float %v1, float %v2) nounwind readnone alwaysinline {
  %1 = tail call float @llvm.pow.f32(float  %v1, float %v2)
  ret float %1
}
