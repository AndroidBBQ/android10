target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i686-unknown-linux"

declare <4 x float> @llvm.x86.sse.min.ps(<4 x float>, <4 x float>)
declare <4 x float> @llvm.x86.sse.max.ps(<4 x float>, <4 x float>)
declare <4 x float> @llvm.x86.sse.min.ss(<4 x float>, <4 x float>)
declare <4 x float> @llvm.x86.sse.max.ss(<4 x float>, <4 x float>)

declare float @llvm.sqrt.f32(float) nounwind readnone
declare <2 x float> @llvm.sqrt.v2f32(<2 x float>) nounwind readnone
declare <3 x float> @llvm.sqrt.v3f32(<3 x float>) nounwind readnone
declare <4 x float> @llvm.sqrt.v4f32(<4 x float>) nounwind readnone

declare float @llvm.exp.f32(float) nounwind readonly
declare float @llvm.pow.f32(float, float) nounwind readonly

define <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %in, <4 x float> %low, <4 x float> %high) nounwind readnone alwaysinline {
  %1 = tail call <4 x float> @llvm.x86.sse.min.ps(<4 x float> %in, <4 x float> %high) nounwind readnone
  %2 = tail call <4 x float> @llvm.x86.sse.max.ps(<4 x float> %1, <4 x float> %low) nounwind readnone
  ret <4 x float> %2
}

define <3 x float> @_Z5clampDv3_fS_S_(<3 x float> %in, <3 x float> %low, <3 x float> %high) nounwind readnone alwaysinline {
  %1 = shufflevector <3 x float> %in, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %2 = shufflevector <3 x float> %low, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %3 = shufflevector <3 x float> %high, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %4 = tail call <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %1, <4 x float> %2, <4 x float> %3) nounwind readnone
  %5 = shufflevector <4 x float> %4, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x float> %5
}

define <2 x float> @_Z5clampDv2_fS_S_(<2 x float> %in, <2 x float> %low, <2 x float> %high) nounwind readnone alwaysinline {
  %1 = shufflevector <2 x float> %in, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %2 = shufflevector <2 x float> %low, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %3 = shufflevector <2 x float> %high, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %4 = tail call <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %1, <4 x float> %2, <4 x float> %3) nounwind readnone
  %5 = shufflevector <4 x float> %4, <4 x float> undef, <2 x i32> <i32 0, i32 1>
  ret <2 x float> %5
}

define float @_Z5clampfff(float %in, float %low, float %high) nounwind readnone alwaysinline {
  %1 = insertelement <4 x float> undef, float %in, i32 0
  %2 = insertelement <4 x float> undef, float %low, i32 0
  %3 = insertelement <4 x float> undef, float %high, i32 0
  %4 = tail call <4 x float> @llvm.x86.sse.min.ss(<4 x float> %1, <4 x float> %3) nounwind readnone
  %5 = tail call <4 x float> @llvm.x86.sse.max.ss(<4 x float> %4, <4 x float> %2) nounwind readnone
  %6 = extractelement <4 x float> %5, i32 0
  ret float %6
}

define <4 x float> @_Z5clampDv4_fff(<4 x float> %in, float %low, float %high) nounwind readonly {
  %1 = insertelement <4 x float> undef, float %low, i32 0
  %2 = insertelement <4 x float> %1, float %low, i32 1
  %3 = insertelement <4 x float> %2, float %low, i32 2
  %4 = insertelement <4 x float> %3, float %low, i32 3
  %5 = insertelement <4 x float> undef, float %high, i32 0
  %6 = insertelement <4 x float> %5, float %high, i32 1
  %7 = insertelement <4 x float> %6, float %high, i32 2
  %8 = insertelement <4 x float> %7, float %high, i32 3
  %9 = tail call <4 x float> @_Z5clampDv4_fS_S_(<4 x float> %in, <4 x float> %4, <4 x float> %8) nounwind readnone
  ret <4 x float> %9
}

define <3 x float> @_Z5clampDv3_fff(<3 x float> %in, float %low, float %high) nounwind readonly {
  %1 = insertelement <3 x float> undef, float %low, i32 0
  %2 = insertelement <3 x float> %1, float %low, i32 1
  %3 = insertelement <3 x float> %2, float %low, i32 2
  %4 = insertelement <3 x float> undef, float %high, i32 0
  %5 = insertelement <3 x float> %4, float %high, i32 1
  %6 = insertelement <3 x float> %5, float %high, i32 2
  %7 = tail call <3 x float> @_Z5clampDv3_fS_S_(<3 x float> %in, <3 x float> %3, <3 x float> %6) nounwind readnone
  ret <3 x float> %7
}

define <2 x float> @_Z5clampDv2_fff(<2 x float> %in, float %low, float %high) nounwind readonly {
  %1 = insertelement <2 x float> undef, float %low, i32 0
  %2 = insertelement <2 x float> %1, float %low, i32 1
  %3 = insertelement <2 x float> undef, float %high, i32 0
  %4 = insertelement <2 x float> %3, float %high, i32 1
  %5 = tail call <2 x float> @_Z5clampDv2_fS_S_(<2 x float> %in, <2 x float> %2, <2 x float> %4) nounwind readnone
  ret <2 x float> %5
}

define float @_Z4sqrtf(float %in) nounwind readnone alwaysinline {
  %1 = tail call float @llvm.sqrt.f32(float %in) nounwind readnone
  ret float %1
}

define <2 x float> @_Z4sqrtDv2_f(<2 x float> %in) nounwind readnone alwaysinline {
  %1 = tail call <2 x float> @llvm.sqrt.v2f32(<2 x float> %in) nounwind readnone
  ret <2 x float> %1
}

define <3 x float> @_Z4sqrtDv3_f(<3 x float> %in) nounwind readnone alwaysinline {
  %1 = tail call <3 x float> @llvm.sqrt.v3f32(<3 x float> %in) nounwind readnone
  ret <3 x float> %1
}

define <4 x float> @_Z4sqrtDv4_f(<4 x float> %in) nounwind readnone alwaysinline {
  %1 = tail call <4 x float> @llvm.sqrt.v4f32(<4 x float> %in) nounwind readnone
  ret <4 x float> %1
}

define float @_Z3powff(float %v1, float %v2) nounwind readnone alwaysinline {
  %1 = tail call float @llvm.pow.f32(float  %v1, float %v2)
  ret float %1
}
