#!/usr/bin/env python
#
# Copyright 2014 The Android Open Source Project
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

from __future__ import print_function
from operator import itemgetter
import collections
import os.path
import re
import sys


# Avoid endlessly adding to the path if this module is imported multiple
# times, e.g. in an interactive session
regpath = os.path.join(sys.path[0], "registry")
if sys.path[1] != regpath:
    sys.path.insert(1, regpath)
import reg


AEP_EXTENSIONS = [
    'GL_KHR_blend_equation_advanced',
    'GL_KHR_debug',
    'GL_KHR_texture_compression_astc_ldr',
    'GL_OES_sample_shading',
    'GL_OES_sample_variables',
    'GL_OES_shader_image_atomic',
    'GL_OES_shader_multisample_interpolation',
    'GL_OES_texture_stencil8',
    'GL_OES_texture_storage_multisample_2d_array',
    'GL_EXT_copy_image',
    'GL_EXT_draw_buffers_indexed',
    'GL_EXT_geometry_shader',
    'GL_EXT_gpu_shader5',
    'GL_EXT_primitive_bounding_box',
    'GL_EXT_shader_io_blocks',
    'GL_EXT_tessellation_shader',
    'GL_EXT_texture_border_clamp',
    'GL_EXT_texture_buffer',
    'GL_EXT_texture_cube_map_array',
    'GL_EXT_texture_sRGB_decode']


def nonestr(s):
    return s if s else ""

def parseProto(elem):
    type = nonestr(elem.text)
    name = None
    for subelem in elem:
        text = nonestr(subelem.text)
        if subelem.tag == 'name':
            name = text
        else:
            type += text
            type += nonestr(subelem.tail)
    return (type.strip(), name)

def parseParam(elem):
    name = elem.find('name').text
    declaration = ''.join(elem.itertext())
    return (name, declaration)

# Format a list of (type, declaration) tuples as a C-style parameter list
def fmtParams(params):
    if not params:
        return 'void'
    return ', '.join(p[1] for p in params)

# Format a list of (type, declaration) tuples as a C-style argument list
def fmtArgs(params):
    return ', '.join(p[0] for p in params)

def overrideSymbolName(sym, apiname):
    # The wrapper intercepts various glGet and glGetString functions and
    # (sometimes) calls the generated thunk which dispatches to the
    # driver's implementation
    wrapped_get_syms = {
        'gles1' : [
            'glGetString'
        ],
        'gles2' : [
            'glGetString',
            'glGetStringi',
            'glGetBooleanv',
            'glGetFloatv',
            'glGetIntegerv',
            'glGetInteger64v',
        ],
    }
    if sym in wrapped_get_syms.get(apiname):
        return '__' + sym
    else:
        return sym


# Generate API trampoline templates:
#   <rtype> API_ENTRY(<name>)(<params>) {
#       CALL_GL_API(<name>, <args>);
#       // or
#       CALL_GL_API_RETURN(<name>, <args>);
#   }
class TrampolineGen(reg.OutputGenerator):
    def __init__(self):
        reg.OutputGenerator.__init__(self, sys.stderr, sys.stderr, None)

    def genCmd(self, cmd, name):
        if re.search('Win32', name):
            return
        reg.OutputGenerator.genCmd(self, cmd, name)

        rtype, fname = parseProto(cmd.elem.find('proto'))
        params = [parseParam(p) for p in cmd.elem.findall('param')]
        call = 'CALL_GL_API' if rtype == 'void' else 'CALL_GL_API_RETURN'
        print('%s API_ENTRY(%s)(%s) {\n'
              '    %s(%s%s%s);\n'
              '}'
              % (rtype, overrideSymbolName(fname, self.genOpts.apiname),
                 fmtParams(params), call, fname,
                 ', ' if len(params) > 0 else '',
                 fmtArgs(params)),
              file=self.outFile)



# Collect all API prototypes across all families, remove duplicates,
# emit to entries.in and enums.in files.
class ApiGenerator(reg.OutputGenerator):
    def __init__(self):
        reg.OutputGenerator.__init__(self, sys.stderr, sys.stderr, None)
        self.cmds = []
        self.enums = collections.OrderedDict()

    def genCmd(self, cmd, name):
        if re.search('Win32', name):
            return
        reg.OutputGenerator.genCmd(self, cmd, name)
        rtype, fname = parseProto(cmd.elem.find('proto'))
        params = [parseParam(p) for p in cmd.elem.findall('param')]
        self.cmds.append({'rtype': rtype, 'name': fname, 'params': params})

    def genEnum(self, enuminfo, name):
        reg.OutputGenerator.genEnum(self, enuminfo, name)
        value = enuminfo.elem.get('value')

        # Skip bitmask enums. Pattern matches:
        # - GL_DEPTH_BUFFER_BIT
        # - GL_MAP_INVALIDATE_BUFFER_BIT_EXT
        # - GL_COLOR_BUFFER_BIT1_QCOM
        # but not
        # - GL_DEPTH_BITS
        # - GL_QUERY_COUNTER_BITS_EXT
        #
        # TODO: Assuming a naming pattern and using a regex is what the
        # old glenumsgen script did. But the registry XML knows which enums are
        # parts of bitmask groups, so we should just use that. I'm not sure how
        # to get the information out though, and it's not critical right now,
        # so leaving for later.
        if re.search('_BIT($|\d*_)', name):
            return
        if re.search('D3D|WIN32', name):
            return

        # Skip non-hex values (GL_TRUE, GL_FALSE, header guard junk)
        if not re.search('0x[0-9A-Fa-f]+', value):
            return

        # Append 'u' or 'ull' type suffix if present
        type = enuminfo.elem.get('type')
        if type and type != 'i':
            value += type

        if value not in self.enums:
            self.enums[value] = name

    def finish(self):
        # sort by function name, remove duplicates
        self.cmds.sort(key=itemgetter('name'))
        cmds = []
        for cmd in self.cmds:
            if len(cmds) == 0 or cmd != cmds[-1]:
                cmds.append(cmd)
        self.cmds = cmds

    # Write entries.in
    def writeEntries(self, outfile):
        for cmd in self.cmds:
            print('GL_ENTRY(%s, %s, %s)'
                  % (cmd['rtype'], cmd['name'], fmtParams(cmd['params'])),
                  file=outfile)

    # Write enums.in
    def writeEnums(self, outfile):
        for enum in self.enums.iteritems():
            print('GL_ENUM(%s,%s)' % (enum[0], enum[1]), file=outfile)


# Generate .spec entries for use by legacy 'gen' script
class SpecGenerator(reg.OutputGenerator):
    def __init__(self):
        reg.OutputGenerator.__init__(self, sys.stderr, sys.stderr, None)

    def genCmd(self, cmd, name):
        reg.OutputGenerator.genCmd(self, cmd, name)
        rtype, fname = parseProto(cmd.elem.find('proto'))
        params = [parseParam(p) for p in cmd.elem.findall('param')]

        print('%s %s ( %s )' % (rtype, fname, fmtParams(params)),
              file=self.outFile)


if __name__ == '__main__':
    registry = reg.Registry()
    registry.loadFile('registry/gl.xml')

    registry.setGenerator(TrampolineGen())
    TRAMPOLINE_OPTIONS = [
        reg.GeneratorOptions(
            apiname             = 'gles1',
            profile             = 'common',
            filename            = '../../libs/GLES_CM/gl_api.in'),
        reg.GeneratorOptions(
            apiname             = 'gles1',
            profile             = 'common',
            emitversions        = None,
            defaultExtensions   = 'gles1',
            filename            = '../../libs/GLES_CM/glext_api.in'),
        reg.GeneratorOptions(
            apiname             = 'gles2',
            profile             = 'common',
            filename            = '../../libs/GLES2/gl2_api.in'),
        reg.GeneratorOptions(
            apiname             = 'gles2',
            profile             = 'common',
            emitversions        = None,
            defaultExtensions   = 'gles2',
            filename            = '../../libs/GLES2/gl2ext_api.in')]
    for opts in TRAMPOLINE_OPTIONS:
        registry.apiGen(opts)

    # Generate a GLESv1_CM entries separately to avoid extra driver loading time
    apigen = ApiGenerator()
    registry.setGenerator(apigen)
    API_OPTIONS = [
        # Generate non-extension versions of each API first, then extensions,
        # so that if an extension enum was later standardized, we see the non-
        # suffixed version first.
        reg.GeneratorOptions(
            apiname             = 'gles1',
            profile             = 'common'),
        reg.GeneratorOptions(
            apiname             = 'gles1',
            profile             = 'common',
            emitversions        = None,
            defaultExtensions   = 'gles1')]
    for opts in API_OPTIONS:
        registry.apiGen(opts)
    apigen.finish()
    with open('../../libs/entries_gles1.in', 'w') as f:
        apigen.writeEntries(f)

    apigen = ApiGenerator()
    registry.setGenerator(apigen)
    API_OPTIONS = [
        # Generate non-extension versions of each API first, then extensions,
        # so that if an extension enum was later standardized, we see the non-
        # suffixed version first.
        reg.GeneratorOptions(
            apiname             = 'gles1',
            profile             = 'common'),
        reg.GeneratorOptions(
            apiname             = 'gles2',
            profile             = 'common'),
        reg.GeneratorOptions(
            apiname             = 'gles1',
            profile             = 'common',
            emitversions        = None,
            defaultExtensions   = 'gles1'),
        reg.GeneratorOptions(
            apiname             = 'gles2',
            profile             = 'common',
            emitversions        = None,
            defaultExtensions   = 'gles2')]
    for opts in API_OPTIONS:
        registry.apiGen(opts)
    apigen.finish()
    with open('../../libs/entries.in', 'w') as f:
        apigen.writeEntries(f)
    with open('../../libs/enums.in', 'w') as f:
        apigen.writeEnums(f)

    registry.setGenerator(SpecGenerator())
    SPEC_OPTIONS = [
        reg.GeneratorOptions(
            apiname             = 'gles2',
            profile             = 'common',
            versions            = '3\.1',
            filename            = '../glgen/specs/gles11/GLES31.spec'),
        reg.GeneratorOptions(
            apiname             = 'gles2',
            profile             = 'common',
            emitversions        = None,
            defaultExtensions   = None,
            addExtensions       = '^({})$'.format('|'.join(AEP_EXTENSIONS)),
            filename            = '../glgen/specs/gles11/GLES31Ext.spec'),
        reg.GeneratorOptions(
            apiname             = 'gles2',
            profile             = 'common',
            versions            = '3\.2',
            filename            = '../glgen/specs/gles11/GLES32.spec')]
    # SpecGenerator creates a good starting point, but the CFunc.java parser is
    # so terrible that the .spec file needs a lot of manual massaging before
    # it works. Commenting this out to avoid accidentally overwriting all the
    # manual modifications.
    #
    # Eventually this script should generate the Java and JNI code directly,
    # skipping the intermediate .spec step, and obsoleting the existing
    # ../glgen system.
    #
    # for opts in SPEC_OPTIONS:
    #     registry.apiGen(opts)

