/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Trace debugging

// Always defined, but may be a no-op if trace support is disabled at compile-time
extern void slTraceSetEnabled(unsigned enabled);

#define SL_TRACE_ENTER          0x1
#define SL_TRACE_LEAVE_FAILURE  0x2
#define SL_TRACE_LEAVE_VOID     0x4
#define SL_TRACE_LEAVE_SUCCESS  0x8
#define SL_TRACE_LEAVE          (SL_TRACE_LEAVE_FAILURE | SL_TRACE_LEAVE_VOID | \
                                    SL_TRACE_LEAVE_SUCCESS)
#define SL_TRACE_ALL            (SL_TRACE_ENTER | SL_TRACE_LEAVE)
#ifndef SL_TRACE_DEFAULT
#define SL_TRACE_DEFAULT        (SL_TRACE_LEAVE_FAILURE)
#endif

#ifndef USE_TRACE

#define SL_ENTER_GLOBAL SLresult result;
#define SL_LEAVE_GLOBAL return result;
#define SL_ENTER_INTERFACE SLresult result;
#define SL_LEAVE_INTERFACE return result;
#define SL_ENTER_INTERFACE_VOID
#define SL_LEAVE_INTERFACE_VOID return;

#define XA_ENTER_GLOBAL XAresult result;
#define XA_LEAVE_GLOBAL return result;
#define XA_ENTER_INTERFACE XAresult result;
#define XA_LEAVE_INTERFACE return result;

#else

extern void slTraceEnterGlobal(const char *function);
extern void slTraceLeaveGlobal(const char *function, SLresult result);
extern void slTraceEnterInterface(const char *function);
extern void slTraceLeaveInterface(const char *function, SLresult result);
extern void slTraceEnterInterfaceVoid(const char *function);
extern void slTraceLeaveInterfaceVoid(const char *function);
#define SL_ENTER_GLOBAL SLresult result; slTraceEnterGlobal(__FUNCTION__);
#define SL_LEAVE_GLOBAL slTraceLeaveGlobal(__FUNCTION__, result); return result;
#define SL_ENTER_INTERFACE SLresult result; slTraceEnterInterface(__FUNCTION__);
#define SL_LEAVE_INTERFACE slTraceLeaveInterface(__FUNCTION__, result); return result;
#define SL_ENTER_INTERFACE_VOID slTraceEnterInterfaceVoid(__FUNCTION__);
#define SL_LEAVE_INTERFACE_VOID slTraceLeaveInterfaceVoid(__FUNCTION__); return;

#define XA_ENTER_GLOBAL XAresult result; slTraceEnterGlobal(__FUNCTION__);
#define XA_LEAVE_GLOBAL slTraceLeaveGlobal(__FUNCTION__, result); return result;
#define XA_ENTER_INTERFACE XAresult result; slTraceEnterInterface(__FUNCTION__);
#define XA_LEAVE_INTERFACE slTraceLeaveInterface(__FUNCTION__, result); return result;

#endif
