target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i686-unknown-linux"

declare <2 x i64> @llvm.x86.sse2.psll.dq(<2 x i64>, i32) nounwind readnone
declare <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float>, <4 x float>) nounwind readnone
declare float @llvm.sqrt.f32(float) nounwind readnone

define float @_Z3dotDv4_fS_(<4 x float> %lhs, <4 x float> %rhs) nounwind readnone {
  %1 = fmul <4 x float> %lhs, %rhs
  %2 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %1, <4 x float> %1) nounwind readnone
  %3 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %2, <4 x float> %2) nounwind readnone
  %4 = extractelement <4 x float> %3, i32 0
  ret float %4
}

define float @_Z3dotDv3_fS_(<3 x float> %lhs, <3 x float> %rhs) nounwind readnone {
  %1 = fmul <3 x float> %lhs, %rhs
  %2 = shufflevector <3 x float> %1, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %3 = bitcast <4 x float> %2 to <2 x i64>
  %4 = tail call <2 x i64> @llvm.x86.sse2.psll.dq(<2 x i64> %3, i32 32)
  %5 = bitcast <2 x i64> %4 to <4 x float>
  %6 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %5, <4 x float> %5) nounwind readnone
  %7 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %6, <4 x float> %6) nounwind readnone
  %8 = extractelement <4 x float> %7, i32 0
  ret float %8
}

define float @_Z3dotDv2_fS_(<2 x float> %lhs, <2 x float> %rhs) nounwind readnone {
  %1 = fmul <2 x float> %lhs, %rhs
  %2 = shufflevector <2 x float> %1, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %3 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %2, <4 x float> %2) nounwind readnone
  %4 = extractelement <4 x float> %3, i32 0
  ret float %4
}

define float @_Z3dotff(float %lhs, float %rhs) nounwind readnone {
  %1 = fmul float %lhs, %rhs
  ret float %1
}

define float @_Z6lengthDv4_f(<4 x float> %in) nounwind readnone alwaysinline {
  %1 = fmul <4 x float> %in, %in
  %2 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %1, <4 x float> %1) nounwind readnone
  %3 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %2, <4 x float> %2) nounwind readnone
  %4 = extractelement <4 x float> %3, i32 0
  %5 = tail call float @llvm.sqrt.f32(float %4) nounwind readnone
  ret float %5
}

define float @_Z6lengthDv3_f(<3 x float> %in) nounwind readnone alwaysinline {
  %1 = fmul <3 x float> %in, %in
  %2 = shufflevector <3 x float> %1, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %3 = bitcast <4 x float> %2 to <2 x i64>
  %4 = tail call <2 x i64> @llvm.x86.sse2.psll.dq(<2 x i64> %3, i32 32)
  %5 = bitcast <2 x i64> %4 to <4 x float>
  %6 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %5, <4 x float> %5) nounwind readnone
  %7 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %6, <4 x float> %6) nounwind readnone
  %8 = extractelement <4 x float> %7, i32 0
  %9 = tail call float @llvm.sqrt.f32(float %8) nounwind readnone
  ret float %9
}

define float @_Z6lengthDv2_f(<2 x float> %in) nounwind readnone alwaysinline {
  %1 = fmul <2 x float> %in, %in
  %2 = shufflevector <2 x float> %1, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %3 = tail call <4 x float> @llvm.x86.sse3.hadd.ps(<4 x float> %2, <4 x float> %2) nounwind readnone
  %4 = extractelement <4 x float> %3, i32 0
  %5 = tail call float @llvm.sqrt.f32(float %4) nounwind readnone
  ret float %5
}

define float @_Z6lengthf(float %in) nounwind readnone alwaysinline {
  %1 = bitcast float %in to i32
  %2 = and i32 %1, 2147483647
  %3 = bitcast i32 %2 to float
  ret float %3
}
