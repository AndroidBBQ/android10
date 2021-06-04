#
# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

header:
summary: Atomic Update Functions
description:
 To update values shared between multiple threads, use the functions below.
 They ensure that the values are atomically updated, i.e. that the memory
 reads, the updates, and the memory writes are done in the right order.

 These functions are slower than their non-atomic equivalents, so use
 them only when synchronization is needed.

 Note that in RenderScript, your code is likely to be running in separate
 threads even though you did not explicitely create them.  The RenderScript
 runtime will very often split the execution of one kernel across multiple
 threads.  Updating globals should be done with atomic functions.  If possible,
 modify your algorithm to avoid them altogether.
end:

function: rsAtomicAdd
version: 14
ret: int32_t, "Value of *addr prior to the operation."
arg: volatile int32_t* addr, "Address of the value to modify."
arg: int32_t value, "Amount to add."
summary: Thread-safe addition
description:
 Atomicly adds a value to the value at addr, i.e. <code>*addr += value</code>.
test: none
end:

function: rsAtomicAdd
version: 20
ret: int32_t
arg: volatile uint32_t* addr
arg: uint32_t value
test: none
end:

function: rsAtomicAnd
version: 14
ret: int32_t, "Value of *addr prior to the operation."
arg: volatile int32_t* addr, "Address of the value to modify."
arg: int32_t value, "Value to and with."
summary: Thread-safe bitwise and
description:
 Atomicly performs a bitwise and of two values, storing the result back at addr,
 i.e. <code>*addr &amp;= value</code>.
test: none
end:

function: rsAtomicAnd
version: 20
ret: int32_t
arg: volatile uint32_t* addr
arg: uint32_t value
test: none
end:

function: rsAtomicCas
version: 14
ret: int32_t, "Value of *addr prior to the operation."
arg: volatile int32_t* addr, "Address of the value to compare and replace if the test passes."
arg: int32_t compareValue, "Value to test *addr against."
arg: int32_t newValue, "Value to write if the test passes."
summary: Thread-safe compare and set
description:
 If the value at addr matches compareValue then the newValue is written at addr,
 i.e. <code>if (*addr == compareValue) { *addr = newValue; }</code>.

 You can check that the value was written by checking that the value returned
 by rsAtomicCas() is compareValue.
test: none
end:

function: rsAtomicCas
version: 14
ret: uint32_t
arg: volatile uint32_t* addr
arg: uint32_t compareValue
arg: uint32_t newValue
test: none
end:

function: rsAtomicDec
version: 14
ret: int32_t, "Value of *addr prior to the operation."
arg: volatile int32_t* addr, "Address of the value to decrement."
summary: Thread-safe decrement
description:
 Atomicly subtracts one from the value at addr.  This is equivalent to <code>@rsAtomicSub(addr, 1)</code>.
test: none
end:

function: rsAtomicDec
version: 20
ret: int32_t
arg: volatile uint32_t* addr
test: none
end:

function: rsAtomicInc
version: 14
ret: int32_t, "Value of *addr prior to the operation."
arg: volatile int32_t* addr, "Address of the value to increment."
summary: Thread-safe increment
description:
 Atomicly adds one to the value at addr.  This is equivalent to <code>@rsAtomicAdd(addr, 1)</code>.
test: none
end:

function: rsAtomicInc
version: 20
ret: int32_t
arg: volatile uint32_t* addr
test: none
end:

function: rsAtomicMax
version: 14
ret: uint32_t, "Value of *addr prior to the operation."
arg: volatile uint32_t* addr, "Address of the value to modify."
arg: uint32_t value, "Comparison value."
summary: Thread-safe maximum
description:
 Atomicly sets the value at addr to the maximum of *addr and value, i.e.
 <code>*addr = max(*addr, value)</code>.
test: none
end:

function: rsAtomicMax
version: 14
ret: int32_t
arg: volatile int32_t* addr
arg: int32_t value
test: none
end:

function: rsAtomicMin
version: 14
ret: uint32_t, "Value of *addr prior to the operation."
arg: volatile uint32_t* addr, "Address of the value to modify."
arg: uint32_t value, "Comparison value."
summary: Thread-safe minimum
description:
 Atomicly sets the value at addr to the minimum of *addr and value, i.e.
 <code>*addr = min(*addr, value)</code>.
test: none
end:

function: rsAtomicMin
version: 14
ret: int32_t
arg: volatile int32_t* addr
arg: int32_t value
test: none
end:

function: rsAtomicOr
version: 14
ret: int32_t, "Value of *addr prior to the operation."
arg: volatile int32_t* addr, "Address of the value to modify."
arg: int32_t value, "Value to or with."
summary: Thread-safe bitwise or
description:
 Atomicly perform a bitwise or two values, storing the result at addr,
 i.e. <code>*addr |= value</code>.
test: none
end:

function: rsAtomicOr
version: 20
ret: int32_t
arg: volatile uint32_t* addr
arg: uint32_t value
test: none
end:

function: rsAtomicSub
version: 14
ret: int32_t, "Value of *addr prior to the operation."
arg: volatile int32_t* addr, "Address of the value to modify."
arg: int32_t value, "Amount to subtract."
summary: Thread-safe subtraction
description:
 Atomicly subtracts a value from the value at addr, i.e. <code>*addr -= value</code>.
test: none
end:

function: rsAtomicSub
version: 20
ret: int32_t
arg: volatile uint32_t* addr
arg: uint32_t value
test: none
end:

function: rsAtomicXor
version: 14
ret: int32_t, "Value of *addr prior to the operation."
arg: volatile int32_t* addr, "Address of the value to modify."
arg: int32_t value, "Value to xor with."
summary: Thread-safe bitwise exclusive or
description:
 Atomicly performs a bitwise xor of two values, storing the result at addr,
 i.e. <code>*addr ^= value</code>.
test: none
end:

function: rsAtomicXor
version: 20
ret: int32_t
arg: volatile uint32_t* addr
arg: uint32_t value
test: none
end:
