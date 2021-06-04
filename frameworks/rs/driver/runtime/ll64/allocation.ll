target datalayout = "e-m:e-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-linux-android"

%struct.rs_allocation = type { i64*, i64*, i64*, i64* }

declare i8* @rsOffset(%struct.rs_allocation* nocapture readonly %a, i32 %sizeOf, i32 %x, i32 %y, i32 %z)
declare i8* @rsOffsetNs(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z)

; The loads and stores in this file are annotated with RenderScript-specific
; information for the type based alias analysis, such that the TBAA analysis
; understands that loads and stores from two allocations with different types
; can never access the same memory element. This is different from C, where
; a char or uchar load/store is special as it can alias with about everything.
;
; The TBAA tree in this file has the the node "RenderScript Distinct TBAA" as
; its root.
; This means all loads/stores that share this common root can be proven to not
; alias. However, the alias analysis still has to assume MayAlias between
; memory accesses in this file and memory accesses annotated with the C/C++
; TBAA metadata.
; A node named "RenderScript TBAA" wraps our distinct TBAA root node.
; If we can ensure that all accesses to elements loaded from RenderScript
; allocations are either annotated with the RenderScript TBAA information or
; not annotated at all, but never annotated with the C/C++ metadata, we
; can add the "RenderScript TBAA" tree under the C/C++ TBAA tree. This enables
; TBAA to prove that an access to data from the RenderScript allocation
; does not alias with a load/store accessing something not part of a RenderScript
; allocation.
; We do this by swapping the second operand of "RenderScript TBAA" with the node
; for "Simple C/C++ TBAA", thus connecting these TBAA groups. The other root
; node (with no children) can then safely be dropped from the analysis.

!13 = !{!"RenderScript Distinct TBAA"}
!14 = !{!"RenderScript TBAA", !13}
!15 = !{!"allocation", !14}

!21 = !{!"char", !15}
define void @rsSetElementAtImpl_char(%struct.rs_allocation* nocapture readonly %a, i8 signext %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 1, i32 %x, i32 %y, i32 %z) #2
  store i8 %val, i8* %1, align 1, !tbaa !21
  ret void
}

define signext i8 @rsGetElementAtImpl_char(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 1, i32 %x, i32 %y, i32 %z) #2
  %2 = load i8, i8* %1, align 1, !tbaa !21
  ret i8 %2
}

!22 = !{!"char2", !15}
define void @rsSetElementAtImpl_char2(%struct.rs_allocation* nocapture readonly %a, i16 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i8>*
  %3 = bitcast i16 %val to <2 x i8>
  store <2 x i8> %3, <2 x i8>* %2, align 2, !tbaa !22
  ret void
}

define <2 x i8> @rsGetElementAtImpl_char2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i8>*
  %3 = load <2 x i8>, <2 x i8>* %2, align 2, !tbaa !22
  ret <2 x i8> %3
}

!23 = !{!"char3", !15}
define void @rsSetElementAtImpl_char3(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i32 %val to <4 x i8>
  %3 = shufflevector <4 x i8> %2, <4 x i8> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x i8>*
  store <4 x i8> %3, <4 x i8>* %4, align 4, !tbaa !23
  ret void
}

define <3 x i8> @rsGetElementAtImpl_char3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = load <4 x i8>, <4 x i8>* %2, align 4, !tbaa !23
  %4 = shufflevector <4 x i8> %3, <4 x i8> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i8> %4
}

!24 = !{!"char4", !15}
define void @rsSetElementAtImpl_char4(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = bitcast i32 %val to <4 x i8>
  store <4 x i8> %3, <4 x i8>* %2, align 4, !tbaa !24
  ret void
}

define <4 x i8> @rsGetElementAtImpl_char4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = load <4 x i8>, <4 x i8>* %2, align 4, !tbaa !24
  ret <4 x i8> %3
}

!25 = !{!"uchar", !15}
define void @rsSetElementAtImpl_uchar(%struct.rs_allocation* nocapture readonly %a, i8 zeroext %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 1, i32 %x, i32 %y, i32 %z) #2
  store i8 %val, i8* %1, align 1, !tbaa !25
  ret void
}

define zeroext i8 @rsGetElementAtImpl_uchar(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 1, i32 %x, i32 %y, i32 %z) #2
  %2 = load i8, i8* %1, align 1, !tbaa !25
  ret i8 %2
}

!26 = !{!"uchar2", !15}
define void @rsSetElementAtImpl_uchar2(%struct.rs_allocation* nocapture readonly %a, i16 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i8>*
  %3 = bitcast i16 %val to <2 x i8>
  store <2 x i8> %3, <2 x i8>* %2, align 2, !tbaa !26
  ret void
}

define <2 x i8> @rsGetElementAtImpl_uchar2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i8>*
  %3 = load <2 x i8>, <2 x i8>* %2, align 2, !tbaa !26
  ret <2 x i8> %3
}

!27 = !{!"uchar3", !15}
define void @rsSetElementAtImpl_uchar3(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i32 %val to <4 x i8>
  %3 = shufflevector <4 x i8> %2, <4 x i8> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x i8>*
  store <4 x i8> %3, <4 x i8>* %4, align 4, !tbaa !27
  ret void
}

define <3 x i8> @rsGetElementAtImpl_uchar3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = load <4 x i8>, <4 x i8>* %2, align 4, !tbaa !27
  %4 = shufflevector <4 x i8> %3, <4 x i8> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i8> %4
}

!28 = !{!"uchar4", !15}
define void @rsSetElementAtImpl_uchar4(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = bitcast i32 %val to <4 x i8>
  store <4 x i8> %3, <4 x i8>* %2, align 4, !tbaa !28
  ret void
}

define <4 x i8> @rsGetElementAtImpl_uchar4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = load <4 x i8>, <4 x i8>* %2, align 4, !tbaa !28
  ret <4 x i8> %3
}

!29 = !{!"short", !15}
define void @rsSetElementAtImpl_short(%struct.rs_allocation* nocapture readonly %a, i16 signext %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i16*
  store i16 %val, i16* %2, align 2, !tbaa !29
  ret void
}

define signext i16 @rsGetElementAtImpl_short(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i16*
  %3 = load i16, i16* %2, align 2, !tbaa !29
  ret i16 %3
}

!30 = !{!"short2", !15}
define void @rsSetElementAtImpl_short2(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i16>*
  %3 = bitcast i32 %val to <2 x i16>
  store <2 x i16> %3, <2 x i16>* %2, align 4, !tbaa !30
  ret void
}

define <2 x i16> @rsGetElementAtImpl_short2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i16>*
  %3 = load <2 x i16>, <2 x i16>* %2, align 4, !tbaa !30
  ret <2 x i16> %3
}

!31 = !{!"short3", !15}
define void @rsSetElementAtImpl_short3(%struct.rs_allocation* nocapture readonly %a, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast <2 x i32> %val to <4 x i16>
  %3 = shufflevector <4 x i16> %2, <4 x i16> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x i16>*
  store <4 x i16> %3, <4 x i16>* %4, align 8, !tbaa !31
  ret void
}

define <3 x i16> @rsGetElementAtImpl_short3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  %3 = load <4 x i16>, <4 x i16>* %2, align 8, !tbaa !31
  %4 = shufflevector <4 x i16> %3, <4 x i16> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i16> %4
}

!32 = !{!"short4", !15}
define void @rsSetElementAtImpl_short4(%struct.rs_allocation* nocapture readonly %a, <4 x i16> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  store <4 x i16> %val, <4 x i16>* %2, align 8, !tbaa !32
  ret void
}

define <4 x i16> @rsGetElementAtImpl_short4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  %3 = load <4 x i16>, <4 x i16>* %2, align 8, !tbaa !32
  ret <4 x i16> %3
}

!33 = !{!"ushort", !15}
define void @rsSetElementAtImpl_ushort(%struct.rs_allocation* nocapture readonly %a, i16 zeroext %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i16*
  store i16 %val, i16* %2, align 2, !tbaa !33
  ret void
}

define zeroext i16 @rsGetElementAtImpl_ushort(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i16*
  %3 = load i16, i16* %2, align 2, !tbaa !33
  ret i16 %3
}

!34 = !{!"ushort2", !15}
define void @rsSetElementAtImpl_ushort2(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i16>*
  %3 = bitcast i32 %val to <2 x i16>
  store <2 x i16> %3, <2 x i16>* %2, align 4, !tbaa !34
  ret void
}

define <2 x i16> @rsGetElementAtImpl_ushort2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i16>*
  %3 = load <2 x i16>, <2 x i16>* %2, align 4, !tbaa !34
  ret <2 x i16> %3
}

!35 = !{!"ushort3", !15}
define void @rsSetElementAtImpl_ushort3(%struct.rs_allocation* nocapture readonly %a, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast <2 x i32> %val to <4 x i16>
  %3 = shufflevector <4 x i16> %2, <4 x i16> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x i16>*
  store <4 x i16> %3, <4 x i16>* %4, align 8, !tbaa !35
  ret void
}

define <3 x i16> @rsGetElementAtImpl_ushort3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  %3 = load <4 x i16>, <4 x i16>* %2, align 8, !tbaa !35
  %4 = shufflevector <4 x i16> %3, <4 x i16> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i16> %4
}

!36 = !{!"ushort4", !15}
define void @rsSetElementAtImpl_ushort4(%struct.rs_allocation* nocapture readonly %a, <4 x i16> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  store <4 x i16> %val, <4 x i16>* %2, align 8, !tbaa !36
  ret void
}

define <4 x i16> @rsGetElementAtImpl_ushort4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  %3 = load <4 x i16>, <4 x i16>* %2, align 8, !tbaa !36
  ret <4 x i16> %3
}

!37 = !{!"int", !15}
define void @rsSetElementAtImpl_int(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i32*
  store i32 %val, i32* %2, align 4, !tbaa !37
  ret void
}

define i32 @rsGetElementAtImpl_int(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i32*
  %3 = load i32, i32* %2, align 4, !tbaa !37
  ret i32 %3
}

!38 = !{!"int2", !15}
define void @rsSetElementAtImpl_int2(%struct.rs_allocation* nocapture readonly %a, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i32>*
  store <2 x i32> %val, <2 x i32>* %2, align 8, !tbaa !38
  ret void
}

define <2 x i32> @rsGetElementAtImpl_int2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i32>*
  %3 = load <2 x i32>, <2 x i32>* %2, align 8, !tbaa !38
  ret <2 x i32> %3
}

!39 = !{!"int3", !15}
define void @rsSetElementAtImpl_int3(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = shufflevector <4 x i32> %val, <4 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %3 = bitcast i8* %1 to <4 x i32>*
  store <4 x i32> %2, <4 x i32>* %3, align 16, !tbaa !39
  ret void
}

define <3 x i32> @rsGetElementAtImpl_int3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  %3 = load <4 x i32>, <4 x i32>* %2, align 8, !tbaa !39
  %4 = shufflevector <4 x i32> %3, <4 x i32> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i32> %4
}

!40 = !{!"int4", !15}
define void @rsSetElementAtImpl_int4(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  store <4 x i32> %val, <4 x i32>* %2, align 16, !tbaa !40
  ret void
}

define <4 x i32> @rsGetElementAtImpl_int4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  %3 = load <4 x i32>, <4 x i32>* %2, align 16, !tbaa !40
  ret <4 x i32> %3
}

!41 = !{!"uint", !15}
define void @rsSetElementAtImpl_uint(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i32*
  store i32 %val, i32* %2, align 4, !tbaa !41
  ret void
}

define i32 @rsGetElementAtImpl_uint(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i32*
  %3 = load i32, i32* %2, align 4, !tbaa !41
  ret i32 %3
}

!42 = !{!"uint2", !15}
define void @rsSetElementAtImpl_uint2(%struct.rs_allocation* nocapture readonly %a, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i32>*
  store <2 x i32> %val, <2 x i32>* %2, align 8, !tbaa !42
  ret void
}

define <2 x i32> @rsGetElementAtImpl_uint2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i32>*
  %3 = load <2 x i32>, <2 x i32>* %2, align 8, !tbaa !42
  ret <2 x i32> %3
}

!43 = !{!"uint3", !15}
define void @rsSetElementAtImpl_uint3(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = shufflevector <4 x i32> %val, <4 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %3 = bitcast i8* %1 to <4 x i32>*
  store <4 x i32> %2, <4 x i32>* %3, align 16, !tbaa !43
  ret void
}

define <3 x i32> @rsGetElementAtImpl_uint3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  %3 = load <4 x i32>, <4 x i32>* %2, align 8, !tbaa !43
  %4 = shufflevector <4 x i32> %3, <4 x i32> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x i32> %4
}

!44 = !{!"uint4", !15}
define void @rsSetElementAtImpl_uint4(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  store <4 x i32> %val, <4 x i32>* %2, align 16, !tbaa !44
  ret void
}

define <4 x i32> @rsGetElementAtImpl_uint4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  %3 = load <4 x i32>, <4 x i32>* %2, align 16, !tbaa !44
  ret <4 x i32> %3
}

!45 = !{!"long", !15}
define void @rsSetElementAtImpl_long(%struct.rs_allocation* nocapture readonly %a, i64 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i64*
  store i64 %val, i64* %2, align 8, !tbaa !45
  ret void
}

define i64 @rsGetElementAtImpl_long(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i64*
  %3 = load i64, i64* %2, align 8, !tbaa !45
  ret i64 %3
}

!46 = !{!"long2", !15}
define void @rsSetElementAtImpl_long2(%struct.rs_allocation* nocapture readonly %a, <2 x i64> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i64>*
  store <2 x i64> %val, <2 x i64>* %2, align 16, !tbaa !46
  ret void
}

define <2 x i64> @rsGetElementAtImpl_long2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i64>*
  %3 = load <2 x i64>, <2 x i64>* %2, align 16, !tbaa !46
  ret <2 x i64> %3
}

!47 = !{!"long3", !15}
define void @rsSetElementAtImpl_long3(%struct.rs_allocation* nocapture readonly %a, <3 x i64>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = load <3 x i64>, <3 x i64>* %val
  %3 = shufflevector <3 x i64> %2, <3 x i64> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x i64>*
  store <4 x i64> %3, <4 x i64>* %4, align 32, !tbaa !47
  ret void
}

define void @rsGetElementAtImpl_long3(<3 x i64>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i64>*
  %3 = load <4 x i64>, <4 x i64>* %2, align 32
  %4 = bitcast <3 x i64>* %agg.result to <4 x i64>*
  store <4 x i64> %3, <4 x i64>* %4, align 32, !tbaa !47
  ret void
}

!48 = !{!"long4", !15}
define void @rsSetElementAtImpl_long4(%struct.rs_allocation* nocapture readonly %a, <4 x i64>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = load <4 x i64>, <4 x i64>* %val
  %3 = bitcast i8* %1 to <4 x i64>*
  store <4 x i64> %2, <4 x i64>* %3, align 32, !tbaa !48
  ret void
}

define void @rsGetElementAtImpl_long4(<4 x i64>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i64>*
  %3 = load <4 x i64>, <4 x i64>* %2, align 32, !tbaa !15
  store <4 x i64> %3, <4 x i64>* %agg.result, align 32, !tbaa !48
  ret void
}

!49 = !{!"ulong", !15}
define void @rsSetElementAtImpl_ulong(%struct.rs_allocation* nocapture readonly %a, i64 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i64*
  store i64 %val, i64* %2, align 8, !tbaa !49
  ret void
}

define i64 @rsGetElementAtImpl_ulong(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to i64*
  %3 = load i64, i64* %2, align 8, !tbaa !49
  ret i64 %3
}

!50 = !{!"ulong2", !15}
define void @rsSetElementAtImpl_ulong2(%struct.rs_allocation* nocapture readonly %a, <2 x i64> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i64>*
  store <2 x i64> %val, <2 x i64>* %2, align 16, !tbaa !50
  ret void
}

define <2 x i64> @rsGetElementAtImpl_ulong2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i64>*
  %3 = load <2 x i64>, <2 x i64>* %2, align 16, !tbaa !50
  ret <2 x i64> %3
}

!51 = !{!"ulong3", !15}
define void @rsSetElementAtImpl_ulong3(%struct.rs_allocation* nocapture readonly %a, <3 x i64>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = load <3 x i64>, <3 x i64>* %val
  %3 = shufflevector <3 x i64> %2, <3 x i64> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x i64>*
  store <4 x i64> %3, <4 x i64>* %4, align 32, !tbaa !51
  ret void
}

define void @rsGetElementAtImpl_ulong3(<3 x i64>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i64>*
  %3 = load <4 x i64>, <4 x i64>* %2, align 32
  %4 = bitcast <3 x i64>* %agg.result to <4 x i64>*
  store <4 x i64> %3, <4 x i64>* %4, align 32, !tbaa !51
  ret void
}

!52 = !{!"ulong4", !15}
define void @rsSetElementAtImpl_ulong4(%struct.rs_allocation* nocapture readonly %a, <4 x i64>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = load <4 x i64>, <4 x i64>* %val
  %3 = bitcast i8* %1 to <4 x i64>*
  store <4 x i64> %2, <4 x i64>* %3, align 32, !tbaa !52
  ret void
}

define void @rsGetElementAtImpl_ulong4(<4 x i64>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i64>*
  %3 = load <4 x i64>, <4 x i64>* %2, align 32, !tbaa !15
  store <4 x i64> %3, <4 x i64>* %agg.result, align 32, !tbaa !52
  ret void
}

!53 = !{!"float", !15}
define void @rsSetElementAtImpl_float(%struct.rs_allocation* nocapture readonly %a, float %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to float*
  store float %val, float* %2, align 4, !tbaa !53
  ret void
}

define float @rsGetElementAtImpl_float(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to float*
  %3 = load float, float* %2, align 4, !tbaa !53
  ret float %3
}

!54 = !{!"float2", !15}
define void @rsSetElementAtImpl_float2(%struct.rs_allocation* nocapture readonly %a, <2 x float> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x float>*
  store <2 x float> %val, <2 x float>* %2, align 8, !tbaa !54
  ret void
}

define <2 x float> @rsGetElementAtImpl_float2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x float>*
  %3 = load <2 x float>, <2 x float>* %2, align 8, !tbaa !54
  ret <2 x float> %3
}

!55 = !{!"float3", !15}
define void @rsSetElementAtImpl_float3(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast <4 x i32> %val to <4 x float>
  %3 = shufflevector <4 x float> %2, <4 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x float>*
  store <4 x float> %3, <4 x float>* %4, align 16, !tbaa !55
  ret void
}

define <3 x float> @rsGetElementAtImpl_float3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x float>*
  %3 = load <4 x float>, <4 x float>* %2, align 8, !tbaa !55
  %4 = shufflevector <4 x float> %3, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x float> %4
}

!56 = !{!"float4", !15}
define void @rsSetElementAtImpl_float4(%struct.rs_allocation* nocapture readonly %a, <4 x float> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x float>*
  store <4 x float> %val, <4 x float>* %2, align 16, !tbaa !56
  ret void
}

define <4 x float> @rsGetElementAtImpl_float4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x float>*
  %3 = load <4 x float>, <4 x float>* %2, align 16, !tbaa !56
  ret <4 x float> %3
}

!57 = !{!"double", !15}
define void @rsSetElementAtImpl_double(%struct.rs_allocation* nocapture readonly %a, double %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to double*
  store double %val, double* %2, align 8, !tbaa !57
  ret void
}

define double @rsGetElementAtImpl_double(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to double*
  %3 = load double, double* %2, align 8, !tbaa !57
  ret double %3
}

!58 = !{!"double2", !15}
define void @rsSetElementAtImpl_double2(%struct.rs_allocation* nocapture readonly %a, <2 x double> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x double>*
  store <2 x double> %val, <2 x double>* %2, align 16, !tbaa !58
  ret void
}

define <2 x double> @rsGetElementAtImpl_double2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 16, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x double>*
  %3 = load <2 x double>, <2 x double>* %2, align 16, !tbaa !58
  ret <2 x double> %3
}

!59 = !{!"double3", !15}
define void @rsSetElementAtImpl_double3(%struct.rs_allocation* nocapture readonly %a, <3 x double>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = load <3 x double>, <3 x double>* %val
  %3 = shufflevector <3 x double> %2, <3 x double> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x double>*
  store <4 x double> %3, <4 x double>* %4, align 32, !tbaa !59
  ret void
}


define void @rsGetElementAtImpl_double3(<3 x double>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x double>*
  %3 = load <4 x double>, <4 x double>* %2, align 32
  %4 = bitcast <3 x double>* %agg.result to <4 x double>*
  store <4 x double> %3, <4 x double>* %4, align 32, !tbaa !59
  ret void
}

!60 = !{!"double4", !15}
define void @rsSetElementAtImpl_double4(%struct.rs_allocation* nocapture readonly %a, <4 x double>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = load <4 x double>, <4 x double>* %val
  %3 = bitcast i8* %1 to <4 x double>*
  store <4 x double> %2, <4 x double>* %3, align 32, !tbaa !60
  ret void
}
define void @rsGetElementAtImpl_double4(<4 x double>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a, i32 32, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x double>*
  %3 = load <4 x double>, <4 x double>* %2, align 32, !tbaa !15
  store <4 x double> %3, <4 x double>* %agg.result, align 32, !tbaa !60
  ret void
}

!61 = !{!"half", !15}
define void @rsSetElementAtImpl_half(%struct.rs_allocation* nocapture readonly %a.coerce, half %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a.coerce, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to half*
  store half %val, half* %2, align 2, !tbaa !61
  ret void
}

define half @rsGetElementAtImpl_half(%struct.rs_allocation* nocapture readonly %a.coerce, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a.coerce, i32 2, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to half*
  %3 = load half, half* %2, align 2, !tbaa !61
  ret half %3
}

!62 = !{!"half2", !15}
define void @rsSetElementAtImpl_half2(%struct.rs_allocation* nocapture readonly %a.coerce, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a.coerce, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x half>*
  %3 = bitcast i32 %val to <2 x half>
  store <2 x half> %3, <2 x half>* %2, align 4, !tbaa !62
  ret void
}

define <2 x half> @rsGetElementAtImpl_half2(%struct.rs_allocation* nocapture readonly %a.coerce, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a.coerce, i32 4, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x half>*
  %3 = load <2 x half>, <2 x half>* %2, align 4, !tbaa !62
  ret <2 x half> %3
}

!63 = !{!"half3", !15}
define void @rsSetElementAtImpl_half3(%struct.rs_allocation* nocapture readonly %a.coerce, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a.coerce, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast <2 x i32> %val to <4 x half>
  %3 = shufflevector <4 x half> %2, <4 x half> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %4 = bitcast i8* %1 to <4 x half>*
  store <4 x half> %3, <4 x half>* %4, align 8, !tbaa !63
  ret void
}

define <3 x half> @rsGetElementAtImpl_half3(%struct.rs_allocation* nocapture readonly %a.coerce, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a.coerce, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x half>*
  %3 = load <4 x half>, <4 x half>* %2, align 8, !tbaa !63
  %4 = shufflevector <4 x half> %3, <4 x half> undef, <3 x i32> <i32 0, i32 1, i32 2>
  ret <3 x half> %4
}

!64 = !{!"half4", !15}
define void @rsSetElementAtImpl_half4(%struct.rs_allocation* nocapture readonly %a.coerce, <4 x half> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a.coerce, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x half>*
  store <4 x half> %val, <4 x half>* %2, align 8, !tbaa !64
  ret void
}

define <4 x half> @rsGetElementAtImpl_half4(%struct.rs_allocation* nocapture readonly %a.coerce, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffset(%struct.rs_allocation* %a.coerce, i32 8, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x half>*
  %3 = load <4 x half>, <4 x half>* %2, align 8, !tbaa !64
  ret <4 x half> %3
}


define void @__rsAllocationVLoadXImpl_long4(<4 x i64>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i64>*
  %3 = load <4 x i64>, <4 x i64>* %2, align 8
  store <4 x i64> %3, <4 x i64>* %agg.result
  ret void
}
define void @__rsAllocationVLoadXImpl_long3(<3 x i64>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i64>*
  %3 = load <3 x i64>, <3 x i64>* %2, align 8
  store <3 x i64> %3, <3 x i64>* %agg.result
  ret void
}
define <2 x i64> @__rsAllocationVLoadXImpl_long2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i64>*
  %3 = load <2 x i64>, <2 x i64>* %2, align 8
  ret <2 x i64> %3
}

define void @__rsAllocationVLoadXImpl_ulong4(<4 x i64>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i64>*
  %3 = load <4 x i64>, <4 x i64>* %2, align 8
  store <4 x i64> %3, <4 x i64>* %agg.result
  ret void
}
define void @__rsAllocationVLoadXImpl_ulong3(<3 x i64>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i64>*
  %3 = load <3 x i64>, <3 x i64>* %2, align 8
  store <3 x i64> %3, <3 x i64>* %agg.result
  ret void
}
define <2 x i64> @__rsAllocationVLoadXImpl_ulong2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i64>*
  %3 = load <2 x i64>, <2 x i64>* %2, align 8
  ret <2 x i64> %3
}

define <4 x i32> @__rsAllocationVLoadXImpl_int4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  %3 = load <4 x i32>, <4 x i32>* %2, align 4
  ret <4 x i32> %3
}
define <3 x i32> @__rsAllocationVLoadXImpl_int3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i32>*
  %3 = load <3 x i32>, <3 x i32>* %2, align 4
  ret <3 x i32> %3
}
define <2 x i32> @__rsAllocationVLoadXImpl_int2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i32>*
  %3 = load <2 x i32>, <2 x i32>* %2, align 4
  ret <2 x i32> %3
}

define <4 x i32> @__rsAllocationVLoadXImpl_uint4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  %3 = load <4 x i32>, <4 x i32>* %2, align 4
  ret <4 x i32> %3
}
define <3 x i32> @__rsAllocationVLoadXImpl_uint3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i32>*
  %3 = load <3 x i32>, <3 x i32>* %2, align 4
  ret <3 x i32> %3
}
define <2 x i32> @__rsAllocationVLoadXImpl_uint2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i32>*
  %3 = load <2 x i32>, <2 x i32>* %2, align 4
  ret <2 x i32> %3
}

define <4 x i16> @__rsAllocationVLoadXImpl_short4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  %3 = load <4 x i16>, <4 x i16>* %2, align 2
  ret <4 x i16> %3
}
define <3 x i16> @__rsAllocationVLoadXImpl_short3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i16>*
  %3 = load <3 x i16>, <3 x i16>* %2, align 2
  ret <3 x i16> %3
}
define <2 x i16> @__rsAllocationVLoadXImpl_short2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i16>*
  %3 = load <2 x i16>, <2 x i16>* %2, align 2
  ret <2 x i16> %3
}

define <4 x i16> @__rsAllocationVLoadXImpl_ushort4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  %3 = load <4 x i16>, <4 x i16>* %2, align 2
  ret <4 x i16> %3
}
define <3 x i16> @__rsAllocationVLoadXImpl_ushort3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i16>*
  %3 = load <3 x i16>, <3 x i16>* %2, align 2
  ret <3 x i16> %3
}
define <2 x i16> @__rsAllocationVLoadXImpl_ushort2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i16>*
  %3 = load <2 x i16>, <2 x i16>* %2, align 2
  ret <2 x i16> %3
}

define <4 x i8> @__rsAllocationVLoadXImpl_char4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = load <4 x i8>, <4 x i8>* %2, align 1
  ret <4 x i8> %3
}
define <3 x i8> @__rsAllocationVLoadXImpl_char3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i8>*
  %3 = load <3 x i8>, <3 x i8>* %2, align 1
  ret <3 x i8> %3
}
define <2 x i8> @__rsAllocationVLoadXImpl_char2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i8>*
  %3 = load <2 x i8>, <2 x i8>* %2, align 1
  ret <2 x i8> %3
}

define <4 x i8> @__rsAllocationVLoadXImpl_uchar4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = load <4 x i8>, <4 x i8>* %2, align 1
  ret <4 x i8> %3
}
define <3 x i8> @__rsAllocationVLoadXImpl_uchar3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i8>*
  %3 = load <3 x i8>, <3 x i8>* %2, align 1
  ret <3 x i8> %3
}
define <2 x i8> @__rsAllocationVLoadXImpl_uchar2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i8>*
  %3 = load <2 x i8>, <2 x i8>* %2, align 1
  ret <2 x i8> %3
}

define <4 x float> @__rsAllocationVLoadXImpl_float4(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x float>*
  %3 = load <4 x float>, <4 x float>* %2, align 4
  ret <4 x float> %3
}
define <3 x float> @__rsAllocationVLoadXImpl_float3(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x float>*
  %3 = load <3 x float>, <3 x float>* %2, align 4
  ret <3 x float> %3
}
define <2 x float> @__rsAllocationVLoadXImpl_float2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x float>*
  %3 = load <2 x float>, <2 x float>* %2, align 4
  ret <2 x float> %3
}

define void @__rsAllocationVLoadXImpl_double4(<4 x double>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x double>*
  %3 = load <4 x double>, <4 x double>* %2, align 8
  store <4 x double> %3, <4 x double>* %agg.result
  ret void
}
define void @__rsAllocationVLoadXImpl_double3(<3 x double>* noalias nocapture sret %agg.result, %struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x double>*
  %3 = load <3 x double>, <3 x double>* %2, align 8
  store <3 x double> %3, <3 x double>* %agg.result
  ret void
}
define <2 x double> @__rsAllocationVLoadXImpl_double2(%struct.rs_allocation* nocapture readonly %a, i32 %x, i32 %y, i32 %z) #0 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x double>*
  %3 = load <2 x double>, <2 x double>* %2, align 8
  ret <2 x double> %3
}


define void @__rsAllocationVStoreXImpl_long4(%struct.rs_allocation* nocapture readonly %a, <4 x i64>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = load <4 x i64>, <4 x i64>* %val
  %3 = bitcast i8* %1 to <4 x i64>*
  store <4 x i64> %2, <4 x i64>* %3, align 8
  ret void
}
define void @__rsAllocationVStoreXImpl_long3(%struct.rs_allocation* nocapture readonly %a, <3 x i64>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = load <3 x i64>, <3 x i64>* %val
  %3 = bitcast i8* %1 to <3 x i64>*
  store <3 x i64> %2, <3 x i64>* %3, align 8
  ret void
}
define void @__rsAllocationVStoreXImpl_long2(%struct.rs_allocation* nocapture readonly %a, <2 x i64> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i64>*
  store <2 x i64> %val, <2 x i64>* %2, align 8
  ret void
}

define void @__rsAllocationVStoreXImpl_ulong4(%struct.rs_allocation* nocapture readonly %a, <4 x i64>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = load <4 x i64>, <4 x i64>* %val
  %3 = bitcast i8* %1 to <4 x i64>*
  store <4 x i64> %2, <4 x i64>* %3, align 8
  ret void
}
define void @__rsAllocationVStoreXImpl_ulong3(%struct.rs_allocation* nocapture readonly %a, <3 x i64>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = load <3 x i64>, <3 x i64>* %val
  %3 = bitcast i8* %1 to <3 x i64>*
  store <3 x i64> %2, <3 x i64>* %3, align 8
  ret void
}
define void @__rsAllocationVStoreXImpl_ulong2(%struct.rs_allocation* nocapture readonly %a, <2 x i64> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i64>*
  store <2 x i64> %val, <2 x i64>* %2, align 8
  ret void
}

define void @__rsAllocationVStoreXImpl_int4(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  store <4 x i32> %val, <4 x i32>* %2, align 4
  ret void
}
define void @__rsAllocationVStoreXImpl_int3(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i32>*
  %3 = shufflevector <4 x i32> %val, <4 x i32> undef, <3 x i32> <i32 0, i32 1, i32 2>
  store <3 x i32> %3, <3 x i32>* %2, align 4
  ret void
}
define void @__rsAllocationVStoreXImpl_int2(%struct.rs_allocation* nocapture readonly %a, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i32>*
  store <2 x i32> %val, <2 x i32>* %2, align 4
  ret void
}

define void @__rsAllocationVStoreXImpl_uint4(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i32>*
  store <4 x i32> %val, <4 x i32>* %2, align 4
  ret void
}
define void @__rsAllocationVStoreXImpl_uint3(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i32>*
  %3 = shufflevector <4 x i32> %val, <4 x i32> undef, <3 x i32> <i32 0, i32 1, i32 2>
  store <3 x i32> %3, <3 x i32>* %2, align 4
  ret void
}
define void @__rsAllocationVStoreXImpl_uint2(%struct.rs_allocation* nocapture readonly %a, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i32>*
  store <2 x i32> %val, <2 x i32>* %2, align 4
  ret void
}

define void @__rsAllocationVStoreXImpl_short4(%struct.rs_allocation* nocapture readonly %a, <4 x i16> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  store <4 x i16> %val, <4 x i16>* %2, align 2
  ret void
}
define void @__rsAllocationVStoreXImpl_short3(%struct.rs_allocation* nocapture readonly %a, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i16>*
  %3 = bitcast <2 x i32> %val to <4 x i16>
  %4 = shufflevector <4 x i16> %3, <4 x i16> undef, <3 x i32> <i32 0, i32 1, i32 2>
  store <3 x i16> %4, <3 x i16>* %2, align 2
  ret void
}
define void @__rsAllocationVStoreXImpl_short2(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i16>*
  %3 = bitcast i32 %val to <2 x i16>
  store <2 x i16> %3, <2 x i16>* %2, align 2
  ret void
}

define void @__rsAllocationVStoreXImpl_ushort4(%struct.rs_allocation* nocapture readonly %a, <4 x i16> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i16>*
  store <4 x i16> %val, <4 x i16>* %2, align 2
  ret void
}
define void @__rsAllocationVStoreXImpl_ushort3(%struct.rs_allocation* nocapture readonly %a, <2 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i16>*
  %3 = bitcast <2 x i32> %val to <4 x i16>
  %4 = shufflevector <4 x i16> %3, <4 x i16> undef, <3 x i32> <i32 0, i32 1, i32 2>
  store <3 x i16> %4, <3 x i16>* %2, align 2
  ret void
}
define void @__rsAllocationVStoreXImpl_ushort2(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i16>*
  %3 = bitcast i32 %val to <2 x i16>
  store <2 x i16> %3, <2 x i16>* %2, align 2
  ret void
}

define void @__rsAllocationVStoreXImpl_char4(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = bitcast i32 %val to <4 x i8>
  store <4 x i8> %3, <4 x i8>* %2, align 1
  ret void
}
define void @__rsAllocationVStoreXImpl_char3(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i8>*
  %3 = bitcast i32 %val to <4 x i8>
  %4 = shufflevector <4 x i8> %3, <4 x i8> undef, <3 x i32> <i32 0, i32 1, i32 2>
  store <3 x i8> %4, <3 x i8>* %2, align 1
  ret void
}
define void @__rsAllocationVStoreXImpl_char2(%struct.rs_allocation* nocapture readonly %a, i16 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i8>*
  %3 = bitcast i16 %val to <2 x i8>
  store <2 x i8> %3, <2 x i8>* %2, align 8
  ret void
}

define void @__rsAllocationVStoreXImpl_uchar4(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x i8>*
  %3 = bitcast i32 %val to <4 x i8>
  store <4 x i8> %3, <4 x i8>* %2, align 1
  ret void
}
define void @__rsAllocationVStoreXImpl_uchar3(%struct.rs_allocation* nocapture readonly %a, i32 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x i8>*
  %3 = bitcast i32 %val to <4 x i8>
  %4 = shufflevector <4 x i8> %3, <4 x i8> undef, <3 x i32> <i32 0, i32 1, i32 2>
  store <3 x i8> %4, <3 x i8>* %2, align 1
  ret void
}
define void @__rsAllocationVStoreXImpl_uchar2(%struct.rs_allocation* nocapture readonly %a, i16 %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x i8>*
  %3 = bitcast i16 %val to <2 x i8>
  store <2 x i8> %3, <2 x i8>* %2, align 8
  ret void
}

define void @__rsAllocationVStoreXImpl_float4(%struct.rs_allocation* nocapture readonly %a, <4 x float> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <4 x float>*
  store <4 x float> %val, <4 x float>* %2, align 4
  ret void
}
define void @__rsAllocationVStoreXImpl_float3(%struct.rs_allocation* nocapture readonly %a, <4 x i32> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <3 x float>*
  %3 = bitcast <4 x i32> %val to <4 x float>
  %4 = shufflevector <4 x float> %3, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2>
  store <3 x float> %4, <3 x float>* %2, align 4
  ret void
}
define void @__rsAllocationVStoreXImpl_float2(%struct.rs_allocation* nocapture readonly %a, <2 x float> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x float>*
  store <2 x float> %val, <2 x float>* %2, align 4
  ret void
}

define void @__rsAllocationVStoreXImpl_double4(%struct.rs_allocation* nocapture readonly %a, <4 x double>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = load <4 x double>, <4 x double>* %val
  %3 = bitcast i8* %1 to <4 x double>*
  store <4 x double> %2, <4 x double>* %3, align 8
  ret void
}
define void @__rsAllocationVStoreXImpl_double3(%struct.rs_allocation* nocapture readonly %a, <3 x double>* %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = load <3 x double>, <3 x double>* %val
  %3 = bitcast i8* %1 to <3 x double>*
  store <3 x double> %2, <3 x double>* %3, align 8
  ret void
}
define void @__rsAllocationVStoreXImpl_double2(%struct.rs_allocation* nocapture readonly %a, <2 x double> %val, i32 %x, i32 %y, i32 %z) #1 {
  %1 = tail call i8* @rsOffsetNs(%struct.rs_allocation* %a, i32 %x, i32 %y, i32 %z) #2
  %2 = bitcast i8* %1 to <2 x double>*
  store <2 x double> %val, <2 x double>* %2, align 8
  ret void
}


attributes #0 = { nounwind readonly "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nobuiltin }

