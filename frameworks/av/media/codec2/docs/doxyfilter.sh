#!/usr/bin/env python3
import re, sys

global in_comment, current, indent, hold
in_comment, current, indent, hold = False, None, '', []

class ChangeCStyleCommentsToDoxy:
    def dump_hold():
        global hold
        for h in hold:
            print(h, end='')
        hold[:] = []

    def doxy_hold():
        global current, hold
        if current == '//':
            for h in hold:
                print(re.sub(r'^( *//(?!/))', r'\1/', h), end='')
        else:
            first = True
            for h in hold:
                if first:
                    h = re.sub(r'^( */[*](?![*]))', r'\1*', h)
                    first = False
                print(h, end='')
        hold[:] = []

    def process_comment(t, ind, line):
        global current, indent, hold
        if t != current or ind not in (indent, indent + ' '):
            dump_hold()
            current, indent = t, ind
        hold.append(line)

    def process_line(ind, line):
        global current, indent
        if ind in (indent, ''):
            doxy_hold()
        else:
            dump_hold()
        current, indent = None, None
        print(line, end='')

    def process(self, input, path):
        for line in input:
            ind = re.match(r'^( *)', line).group(1)
            if in_comment:
                # TODO: this is not quite right, but good enough
                m = re.match(r'^ *[*]/', line)
                if m:
                    process_comment('/*', ind, line)
                    in_comment = False
                else:
                    process_comment('/*', ind, line)
                continue
            m = re.match(r'^ *//', line)
            if m:
                # one-line comment
                process_comment('//', ind, line)
                continue
            m = re.match(r'^ */[*]', line)
            if m:
                # multi-line comment
                process_comment('/*', ind, line)
                # TODO: this is not quite right, but good enough
                in_comment = not re.match(r'^ *[*]/', line)
                continue
            process_line(ind, line)

class AutoGroup:
    def process(self, input, path):
        if '/codec2/include/' in path:
            group = 'API Codec2 API'
        elif False:
            return
        elif '/codec2/vndk/' in path:
            group = 'VNDK Platform provided glue'
        elif '/codec2/tests/' in path:
            group = 'Tests Unit tests'
        else:
            group = 'Random Misc. sandbox'

        print('#undef __APPLE__')

        for line in input:
            if re.match(r'^namespace android {', line):
                print(line, end='')
                print()
                print(r'/// \addtogroup {}'.format(group))
                print(r'/// @{')
                continue
            elif re.match(r'^} +// +namespace', line):
                print(r'/// @}')
                print()
            print(line, end='')

P = AutoGroup()
for path in sys.argv[1:]:
    with open(path, 'rt') as input:
        P.process(input, path)
