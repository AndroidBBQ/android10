#!/usr/bin/env python
#
# Copyright (C) 2016 The Android Open Source Project
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
from __future__ import print_function

import argparse
import glob
import multiprocessing
import os
import shutil
import subprocess
import sys
import re


THIS_DIR = os.path.realpath(os.path.dirname(__file__))
ORIG_ENV = dict(os.environ)


def android_path(*args):
    out_dir = os.path.realpath(os.path.join(THIS_DIR, '../..', *args))
    return out_dir


def build_path(*args):
    # Our multistage build directories will be placed under OUT_DIR if it is in
    # the environment. By default they will be placed under
    # $ANDROID_BUILD_TOP/out.
    top_out = ORIG_ENV.get('OUT_DIR', android_path('out'))
    if not os.path.isabs(top_out):
        top_out = os.path.realpath(top_out)
    out_dir = os.path.join(top_out, *args)
    return out_dir


def install_file(src, dst):
    print('Copying ' + src)
    shutil.copy2(src, dst)


def install_directory(src, dst):
    print('Copying ' + src)
    shutil.copytree(src, dst)


def build(out_dir):
    products = (
        'aosp_arm',
        'aosp_arm64',
        # 'aosp_mips',
        # 'aosp_mips64',
        'aosp_x86',
        'aosp_x86_64',
    )
    for product in products:
        build_product(out_dir, product)


def build_product(out_dir, product):
    env = dict(ORIG_ENV)
    env['FORCE_BUILD_LLVM_COMPONENTS'] = 'true'
    env['FORCE_BUILD_RS_COMPAT'] = 'true'
    env['OUT_DIR'] = out_dir
    env['SKIP_LLVM_TESTS'] = 'true'
    env['SOONG_ALLOW_MISSING_DEPENDENCIES'] = 'true'
    env['TARGET_BUILD_VARIANT'] = 'userdebug'
    env['TARGET_PRODUCT'] = product

    jobs_arg = '-j{}'.format(multiprocessing.cpu_count())
    targets = [
        # PHONY target specified in frameworks/rs/Android.mk.
        'rs-prebuilts-full',
        # We have to explicitly specify the jar for JACK to build.
        android_path('out/target/common/obj/JAVA_LIBRARIES/' +
            'android-support-v8-renderscript_intermediates/classes.jar')
    ]
    subprocess.check_call(
        ['make', jobs_arg] + targets, cwd=android_path(), env=env)


def package_toolchain(build_dir, build_name, host, dist_dir):
    package_name = 'renderscript-' + build_name
    install_host_dir = build_path('install', host)
    install_dir = os.path.join(install_host_dir, package_name)

    # Remove any previously installed toolchain so it doesn't pollute the
    # build.
    if os.path.exists(install_host_dir):
        shutil.rmtree(install_host_dir)

    install_toolchain(build_dir, install_dir, host)

    tarball_name = package_name + '-' + host
    package_path = os.path.join(dist_dir, tarball_name) + '.tar.bz2'
    print('Packaging ' + package_path)
    args = [
        'tar', '-cjC', install_host_dir, '-f', package_path, package_name
    ]
    subprocess.check_call(args)


def install_toolchain(build_dir, install_dir, host):
    install_built_host_files(build_dir, install_dir, host)
    install_clang_headers(build_dir, install_dir, host)
    install_built_device_files(build_dir, install_dir, host)
    install_license_files(install_dir)
    # We need to package libwinpthread-1.dll for Windows. This is explicitly
    # linked whenever pthreads is used, and the build system doesn't allow
    # us to link just that library statically (ldflags are stripped out
    # of ldlibs and vice-versa).
    # Bug: http://b/34273721
    if host.startswith('windows'):
        install_winpthreads(install_dir)


def install_winpthreads(install_dir):
      """Installs the winpthreads runtime to the Windows bin directory."""
      lib_name = 'libwinpthread-1.dll'
      mingw_dir = android_path(
          'prebuilts/gcc/linux-x86/host/x86_64-w64-mingw32-4.8')
      # RenderScript NDK toolchains for Windows only contains 32-bit binaries.
      lib_path = os.path.join(mingw_dir, 'x86_64-w64-mingw32/lib32', lib_name)

      lib_install = os.path.join(install_dir, 'bin', lib_name)
      install_file(lib_path, lib_install)


def install_built_host_files(build_dir, install_dir, host):
    is_windows = host.startswith('windows')
    is_darwin = host.startswith('darwin-x86')
    bin_ext = '.exe' if is_windows else ''

    if is_windows:
        lib_ext = '.dll'
    elif is_darwin:
        lib_ext = '.dylib'
    else:
        lib_ext = '.so'

    built_files = [
        'bin/llvm-rs-cc' + bin_ext,
        'bin/bcc_compat' + bin_ext,
    ]

    if is_windows:
        built_files.extend([
            'lib/libbcc' + lib_ext,
            'lib/libbcinfo' + lib_ext,
            'lib/libclang_android' + lib_ext,
            'lib/libLLVM_android' + lib_ext,
        ])
    else:
        built_files.extend([
            'lib64/libbcc' + lib_ext,
            'lib64/libbcinfo' + lib_ext,
            'lib64/libclang_android' + lib_ext,
            'lib64/libLLVM_android' + lib_ext,
            'lib64/libc++' + lib_ext,
        ])

    for built_file in built_files:
        dirname = os.path.dirname(built_file)
        # Put dlls and exes into bin/ for windows.
        # Bug: http://b/34273721
        if is_windows:
            dirname = 'bin'
        install_path = os.path.join(install_dir, dirname)
        if not os.path.exists(install_path):
            os.makedirs(install_path)

        built_path = os.path.join(build_dir, 'host', host, built_file)
        install_file(built_path, install_path)

        file_name = os.path.basename(built_file)

        # Only strip bin files (not libs) on darwin.
        if not is_darwin or built_file.startswith('bin/'):
            subprocess.check_call(
                ['strip', os.path.join(install_path, file_name)])


def install_clang_headers(build_dir, install_dir, host):
    def should_copy(path):
        if os.path.basename(path) in ('Makefile', 'CMakeLists.txt'):
            return False
        _, ext = os.path.splitext(path)
        if ext == '.mk':
            return False
        return True

    headers_src = android_path('external/clang/lib/Headers')
    headers_dst = os.path.join(
        install_dir, 'clang-include')
    os.makedirs(headers_dst)
    for header in os.listdir(headers_src):
        if not should_copy(header):
            continue
        install_file(os.path.join(headers_src, header), headers_dst)

    install_file(android_path('bionic/libc/include/stdatomic.h'), headers_dst)


def install_built_device_files(build_dir, install_dir, host):
    product_to_arch = {
        'generic': 'arm',
        'generic_arm64': 'arm64',
        # 'generic_mips': 'mips',
        # 'generic_mips64': 'mips64el',
        'generic_x86': 'x86',
        'generic_x86_64': 'x86_64',
    }

    bc_lib = 'librsrt'

    static_libs = {
        'libRScpp_static',
        'libcompiler_rt'
    }

    shared_libs = {
        'libRSSupport',
        'libRSSupportIO',
        'libblasV8',
    }

    for product, arch in product_to_arch.items():
        lib_dir = os.path.join(install_dir, 'platform', arch)
        os.makedirs(lib_dir)

        # Copy librsrt_ARCH.bc.
        lib_name = bc_lib + '_' + arch + '.bc'
        if not host.startswith('windows'):
            built_lib = os.path.join(build_dir, 'host', host, 'lib64', lib_name)
        else:
            built_lib = os.path.join(build_dir, 'host', 'linux-x86', 'lib64', lib_name)
        install_file(built_lib, os.path.join(lib_dir, bc_lib + '.bc'))

        # Copy static libs and share libs.
        product_dir = os.path.join(build_dir, 'target/product', product)
        static_lib_dir = os.path.join(product_dir, 'obj/STATIC_LIBRARIES')
        shared_lib_dir = os.path.join(product_dir, 'obj/SHARED_LIBRARIES')
        for static_lib in static_libs:
            built_lib = os.path.join(
                static_lib_dir, static_lib + '_intermediates/' + static_lib + '.a')
            lib_name = static_lib + '.a'
            install_file(built_lib, os.path.join(lib_dir, lib_name))
        for shared_lib in shared_libs:
            built_lib = os.path.join(
                shared_lib_dir, shared_lib + '_intermediates/' + shared_lib + '.so')
            lib_name = shared_lib + '.so'
            install_file(built_lib, os.path.join(lib_dir, lib_name))

    # Copy renderscript-v8.jar.
    lib_dir = os.path.join(install_dir, 'platform')
    jar_dir = os.path.join(build_dir, 'target/common/obj/JAVA_LIBRARIES/'
        'android-support-v8-renderscript_intermediates/classes.jar')
    install_file(jar_dir, os.path.join(lib_dir, 'renderscript-v8.jar'))

    # Copy RS runtime headers.
    headers_dst_base = os.path.join(install_dir, 'platform', 'rs')

    headers_src = android_path('frameworks/rs/script_api/include')
    headers_dst = os.path.join(headers_dst_base, 'scriptc')
    install_directory(headers_src, headers_dst)

    # Copy RS C++ API headers.
    headers_src = android_path('frameworks/rs/cpp/util')
    headers_dst = os.path.join(headers_dst_base, 'cpp/util')
    install_directory(headers_src, headers_dst)
    install_file(android_path('frameworks/rs/rsDefines.h'), headers_dst_base)
    install_file(android_path('frameworks/rs/cpp/RenderScript.h'), os.path.join(headers_dst_base, 'cpp'))
    install_file(android_path('frameworks/rs/cpp/rsCppStructs.h'), os.path.join(headers_dst_base, 'cpp'))


def install_license_files(install_dir):
    projects = (
        'external/clang',
        'external/compiler-rt',
        'external/llvm',
        'frameworks/compile/slang',
        'frameworks/compile/libbcc',
        # 'frameworks/rs', # No notice license file found.
    )

    notices = []
    for project in projects:
        project_path = android_path(project)
        license_pattern = os.path.join(project_path, 'MODULE_LICENSE_*')
        for license_file in glob.glob(license_pattern):
            install_file(license_file, install_dir)
        with open(os.path.join(project_path, 'NOTICE')) as notice_file:
            notices.append(notice_file.read())
    with open(os.path.join(install_dir, 'NOTICE'), 'w') as notice_file:
        notice_file.write('\n'.join(notices))


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument(
        '--build-name', default='dev', help='Release name for the package.')

    return parser.parse_args()


def main():
    args = parse_args()

    if sys.platform.startswith('linux'):
        hosts = ['linux-x86', 'windows-x86']
    elif sys.platform == 'darwin':
        hosts = ['darwin-x86']
    else:
        raise RuntimeError('Unsupported host: {}'.format(sys.platform))

    out_dir = build_path()
    build(out_dir=out_dir)

    dist_dir = ORIG_ENV.get('DIST_DIR', out_dir)
    for host in hosts:
        package_toolchain(out_dir, args.build_name, host, dist_dir)


if __name__ == '__main__':
    main()
