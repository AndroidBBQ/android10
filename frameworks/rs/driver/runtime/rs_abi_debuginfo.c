/*
 * This file is used to generate the debug info describing
 * the RsExpandKernelDriverInfo structure fields and the interface
 * of the "expanded" kernel function that acts as a boundary
 * between the host part and the accelerator part of RenderScript.
 * This enables the debugger to inspect input and output Allocations
 * and sizes and provide a piece of useful feedback to RS kernel developers.
 */
#include "rs_value_types.rsh"
#include "rsCpuCoreRuntime.h"
typedef void (*ForEachFunc_t)(const struct RsExpandKernelDriverInfo *info, uint32_t x1, uint32_t x2, uint32_t outStride);
ForEachFunc_t rsDebugOuterForeachT;
