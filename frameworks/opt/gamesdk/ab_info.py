from __future__ import print_function
import google.protobuf as pb

print('python protobuf contents:')
for attr in dir(pb):
    if attr == '__builtins__':
        continue
    print('{}: {}'.format(attr, getattr(pb, attr)))
