# Copyright (C) 2017 The Android Open Source Project
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
# Virtual touchpad for the VR virtual display
device.internal = 1

touch.deviceType = touchScreen

# Have input flinger treat injected scroll events like a G1 ball
# rather than the default mouse wheel, because the latter requires
# a visible pointer for targeting.
device.type = rotaryEncoder
device.res = 1.0e+2
device.scalingFactor = 1.0e-2

# This displayID matches the unique ID of the virtual display created for VR.
# This will indicate to input flinger than it should link this input device
# with the virtual display.
touch.displayId = virtual:android:277f1a09-b88d-4d1e-8716-796f114d080b
