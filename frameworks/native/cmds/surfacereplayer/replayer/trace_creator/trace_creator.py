#!/usr/bin/python
from subprocess import call
import os
proto_path = os.environ['ANDROID_BUILD_TOP'] + "/frameworks/native/cmds/surfacereplayer/proto/src/"
call(["aprotoc", "-I=" + proto_path, "--python_out=.", proto_path + "trace.proto"])

from trace_pb2 import *

trace = Trace()

def main():
    global trace
    while(1):
        option = main_menu()

        if option == 0:
            break

        increment = trace.increment.add()
        increment.time_stamp  = int(input("Time stamp of action: "))

        if option == 1:
           transaction(increment)
        elif option == 2:
            surface_create(increment)
        elif option == 3:
            surface_delete(increment)
        elif option == 4:
            display_create(increment)
        elif option == 5:
            display_delete(increment)
        elif option == 6:
            buffer_update(increment)
        elif option == 7:
            vsync_event(increment)
        elif option == 8:
            power_mode_update(increment)

    seralizeTrace()

def seralizeTrace():
    with open("trace.dat", 'wb') as f:
        f.write(trace.SerializeToString())


def main_menu():
    print ("")
    print ("What would you like to do?")
    print ("1. Add transaction")
    print ("2. Add surface creation")
    print ("3. Add surface deletion")
    print ("4. Add display creation")
    print ("5. Add display deletion")
    print ("6. Add buffer update")
    print ("7. Add VSync event")
    print ("8. Add power mode update")
    print ("0. Finish and serialize")
    print ("")

    return int(input("> "))

def transaction_menu():
    print ("")
    print ("What kind of transaction?")
    print ("1. Position Change")
    print ("2. Size Change")
    print ("3. Alpha Change")
    print ("4. Layer Change")
    print ("5. Crop Change")
    print ("6. Final Crop Change")
    print ("7. Matrix Change")
    print ("8. Override Scaling Mode Change")
    print ("9. Transparent Region Hint Change")
    print ("10. Layer Stack Change")
    print ("11. Hidden Flag Change")
    print ("12. Opaque Flag Change")
    print ("13. Secure Flag Change")
    print ("14. Deferred Transaction Change")
    print ("15. Display - Surface Change")
    print ("16. Display - Layer Stack Change")
    print ("17. Display - Size Change")
    print ("18. Display - Projection Change")
    print ("0. Finished adding Changes to this transaction")
    print ("")

    return int(input("> "))

def transaction(increment):
    global trace

    increment.transaction.synchronous \
            = bool(input("Is transaction synchronous (True/False): "))
    increment.transaction.animation \
            = bool(input("Is transaction animated (True/False): "))

    while(1):
        option = transaction_menu()

        if option == 0:
            break

        change = None
        if option <= 14:
            change = increment.transaction.surface_change.add()
        elif option >= 15 and option <= 18:
            change = increment.transaction.display_change.add()

        change.id = int(input("ID of layer/display to undergo a change: "))

        if option == 1:
            change.position.x, change.position.y = position()
        elif option == 2:
            change.size.w, change.size.h = size()
        elif option == 3:
            change.alpha.alpha = alpha()
        elif option == 4:
            change.layer.layer = layer()
        elif option == 5:
            change.crop.rectangle.left,  change.crop.rectangle.top, \
            change.crop.rectangle.right, change.crop.rectangle.bottom = crop()
        elif option == 6:
            change.final_crop.rectangle.left, \
            change.final_crop.rectangle.top,  \
            change.final_crop.rectangle.right,\
            change.final_crop.rectangle.bottom = final_crop()
        elif option == 7:
            change.matrix.dsdx,\
            change.matrix.dtdx,\
            change.matrix.dsdy,\
            change.matrix.dtdy = layer()
        elif option == 8:
            change.override_scaling_mode.override_scaling_mode \
                                     = override_scaling_mode()
        elif option == 9:
            for rect in transparent_region_hint():
                new = increment.transparent_region_hint.region.add()
                new.left = rect[0]
                new.top = rect[1]
                new.right = rect[2]
                new.bottom = rect[3]
        elif option == 10:
            change.layer_stack.layer_stack = layer_stack()
        elif option == 11:
            change.hidden_flag.hidden_flag = hidden_flag()
        elif option == 12:
            change.opaque_flag.opaque_flag = opaque_flag()
        elif option == 13:
            change.secure_flag.secure_flag = secure_flag()
        elif option == 14:
            change.deferred_transaction.layer_id, \
            change.deferred_transaction.frame_number = deferred_transaction()
        elif option == 15:
            change.surface.buffer_queue_id, \
            change.surface.buffer_queue_name = surface()
        elif option == 16:
            change.layer_stack.layer_stack = layer_stack()
        elif option == 17:
            change.size.w, change.size.h = size()
        elif option == 18:
            projection(change)

def surface_create(increment):
    increment.surface_creation.id = int(input("Enter id: "))
    n = str(raw_input("Enter name: "))
    increment.surface_creation.name = n
    increment.surface_creation.w = input("Enter w: ")
    increment.surface_creation.h = input("Enter h: ")

def surface_delete(increment):
    increment.surface_deletion.id = int(input("Enter id: "))

def display_create(increment):
    increment.display_creation.id = int(input("Enter id: "))
    increment.display_creation.name = str(raw_input("Enter name: "))
    increment.display_creation.display_id = int(input("Enter display ID: "))
    increment.display_creation.is_secure = bool(input("Enter if secure: "))

def display_delete(increment):
    increment.surface_deletion.id = int(input("Enter id: "))

def buffer_update(increment):
    increment.buffer_update.id = int(input("Enter id: "))
    increment.buffer_update.w = int(input("Enter w: "))
    increment.buffer_update.h = int(input("Enter h: "))
    increment.buffer_update.frame_number = int(input("Enter frame_number: "))

def vsync_event(increment):
    increment.vsync_event.when = int(input("Enter when: "))

def power_mode_update(increment):
    increment.power_mode_update.id = int(input("Enter id: "))
    increment.power_mode_update.mode = int(input("Enter mode: "))

def position():
    x = input("Enter x: ")
    y = input("Enter y: ")

    return float(x), float(y)

def size():
    w = input("Enter w: ")
    h = input("Enter h: ")

    return int(w), int(h)

def alpha():
    alpha = input("Enter alpha: ")

    return float(alpha)

def layer():
    layer = input("Enter layer: ")

    return int(layer)

def crop():
    return rectangle()

def final_crop():
    return rectangle()

def matrix():
    dsdx = input("Enter dsdx: ")
    dtdx = input("Enter dtdx: ")
    dsdy = input("Enter dsdy: ")
    dtdy = input("Enter dtdy: ")

    return float(dsdx)

def override_scaling_mode():
    mode = input("Enter override scaling mode: ")

    return int(mode)

def transparent_region_hint():
    num = input("Enter number of rectangles in region: ")

    return [rectangle() in range(x)]

def layer_stack():
    layer_stack = input("Enter layer stack: ")

    return int(layer_stack)

def hidden_flag():
    flag = input("Enter hidden flag state (True/False): ")

    return bool(flag)

def opaque_flag():
    flag = input("Enter opaque flag state (True/False): ")

    return bool(flag)

def secure_flag():
    flag = input("Enter secure flag state (True/False): ")

    return bool(flag)

def deferred_transaction():
    layer_id = input("Enter layer_id: ")
    frame_number = input("Enter frame_number: ")

    return int(layer_id), int(frame_number)

def surface():
    id = input("Enter id: ")
    name = raw_input("Enter name: ")

    return int(id), str(name)

def projection(change):
    change.projection.orientation = input("Enter orientation: ")
    print("Enter rectangle for viewport")
    change.projection.viewport.left, \
    change.projection.viewport.top,  \
    change.projection.viewport.right,\
    change.projection.viewport.bottom = rectangle()
    print("Enter rectangle for frame")
    change.projection.frame.left, \
    change.projection.frame.top,  \
    change.projection.frame.right,\
    change.projection.frame.bottom = rectangle()

def rectangle():
    left = input("Enter left: ")
    top = input("Enter top: ")
    right = input("Enter right: ")
    bottom = input("Enter bottom: ")

    return int(left), int(top), int(right), int(bottom)

if __name__ == "__main__":
    main()
