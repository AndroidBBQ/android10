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
summary: Input/Output Functions
description:
 These functions are used to:<ul>
 <li>Send information to the Java client, and</li>
#TODO We need better documentation for:
 <li>Send the processed allocation or receive the next allocation to process.</li></ul>
end:

function: rsAllocationIoReceive
version: 16
ret: void
arg: rs_allocation a, "Allocation to work on."
summary: Receive new content from the queue
description:
 Receive a new set of contents from the queue.
#TODO We need better documentation.

 This function should not be called from inside a kernel, or from any function
 that may be called directly or indirectly from a kernel. Doing so would cause a
 runtime error.
test: none
end:

function: rsAllocationIoSend
version: 16
ret: void
arg: rs_allocation a, "Allocation to work on."
summary: Send new content to the queue
description:
 Send the contents of the Allocation to the queue.
#TODO We need better documentation.

 This function should not be called from inside a kernel, or from any function
 that may be called directly or indirectly from a kernel. Doing so would cause a
 runtime error.
test: none
end:

function: rsSendToClient
ret: bool
arg: int cmdID
summary: Send a message to the client, non-blocking
description:
 Sends a message back to the client.  This call does not block.
 It returns true if the message was sent and false if the
 message queue is full.

 A message ID is required.  The data payload is optional.

 See <a href='http://developer.android.com/reference/android/renderscript/RenderScript.RSMessageHandler.html'>RenderScript.RSMessageHandler</a>.
test: none
end:

function: rsSendToClient
ret: bool
arg: int cmdID
arg: const void* data, "Application specific data."
arg: uint len, "Length of the data, in bytes."
test: none
end:

function: rsSendToClientBlocking
ret: void
arg: int cmdID
summary: Send a message to the client, blocking
description:
 Sends a message back to the client.  This function will block
 until there is room on the message queue for this message.
 This function may return before the message was delivered and
 processed by the client.

 A message ID is required.  The data payload is optional.

 See <a href='http://developer.android.com/reference/android/renderscript/RenderScript.RSMessageHandler.html'>RenderScript.RSMessageHandler</a>.
test: none
end:

function: rsSendToClientBlocking
ret: void
arg: int cmdID
arg: const void* data, "Application specific data."
arg: uint len, "Length of the data, in bytes."
test: none
end:
