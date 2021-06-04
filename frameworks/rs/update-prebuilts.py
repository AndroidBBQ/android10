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

"""Update the prebuilt RenderScript from the build server."""
from __future__ import print_function

import argparse
import inspect
import os
import shutil
import subprocess
import sys


THIS_DIR = os.path.realpath(os.path.dirname(__name__))
ANDROID_DIR = os.path.realpath(os.path.join(THIS_DIR, '../..'))

BRANCH = 'aosp-master'


def android_path(*args):
    return os.path.join(ANDROID_DIR, *args)


class ArgParser(argparse.ArgumentParser):
    def __init__(self):
        super(ArgParser, self).__init__(
            description=inspect.getdoc(sys.modules[__name__]))

        self.add_argument(
            'build_number', metavar='BUILD',
            help='Build number to pull from the build server.')

        self.add_argument(
            '-a', '--android_branch', default=BRANCH,
            help='The Android branch to pull from build server, default: ' + BRANCH + '.')

        self.add_argument(
            '-b', '--bug', type=int,
            help='Bug to reference in commit message.')

        self.add_argument(
            '--use-current-branch', action='store_true',
            help='Do not repo start a new branch for the update.')


def host_to_build_host(host):
    """Gets the build host name for an NDK host tag.

    The Windows builds are done from Linux.
    """
    return {
        'darwin': 'mac',
        'linux': 'linux',
        'windows': 'linux',
    }[host]


def build_name(host):
    """Gets the build name for a given host.

    The build name is either "linux" or "darwin", with any Windows builds
    coming from "linux".
    """
    return {
        'darwin': 'darwin',
        'linux': 'linux',
        'windows': 'linux',
    }[host]


def manifest_name(build_number):
    """Returns the manifest file name for a given build.

    >>> manifest_name('1234')
    'manifest_1234.xml'
    """
    return 'manifest_{}.xml'.format(build_number)


def package_name(build_number, host):
    """Returns the file name for a given package configuration.

    >>> package_name('1234', 'linux')
    'renderscript-1234-linux-x86.tar.bz2'
    """
    return 'renderscript-{}-{}-x86.tar.bz2'.format(build_number, host)


def download_build(host, build_number, android_branch, download_dir):
    filename = package_name(build_number, host)
    return download_file(host, build_number, android_branch, filename, download_dir)


def download_manifest(host, build_number, android_branch, download_dir):
    filename = manifest_name(build_number)
    return download_file(host, build_number, android_branch, filename, download_dir)


def download_file(host, build_number, android_branch, pkg_name, download_dir):
    url_base = 'https://android-build-uber.corp.google.com'
    path = 'builds/{build_branch}-{build_host}-{build_name}/{build_num}'.format(
        build_branch=android_branch,
        build_host=host_to_build_host(host),
        build_name='renderscript',
        build_num=build_number)

    url = '{}/{}/{}'.format(url_base, path, pkg_name)

    TIMEOUT = '60'  # In seconds.
    out_file_path = os.path.join(download_dir, pkg_name)
    with open(out_file_path, 'w') as out_file:
        print('Downloading {} to {}'.format(url, out_file_path))
        subprocess.check_call(
            ['sso_client', '--location', '--request_timeout', TIMEOUT, url],
            stdout=out_file)
    return out_file_path


def extract_package(package, install_dir):
    cmd = ['tar', 'xf', package, '-C', install_dir]
    print('Extracting {}...'.format(package))
    subprocess.check_call(cmd)


def update_renderscript(host, build_number, android_branch, use_current_branch, download_dir, bug):
    host_tag = host + '-x86'
    prebuilt_dir = android_path('prebuilts/renderscript/host', host_tag)
    os.chdir(prebuilt_dir)

    if not use_current_branch:
        subprocess.check_call(
            ['repo', 'start', 'update-renderscript-{}'.format(build_number), '.'])

    package = download_build(host, build_number, android_branch, download_dir)
    manifest = download_manifest(host, build_number, android_branch, download_dir)

    install_subdir = 'current'
    extract_package(package, prebuilt_dir)
    shutil.rmtree(install_subdir)
    shutil.move('renderscript-{}'.format(build_number), install_subdir)
    shutil.copy(manifest, install_subdir)

    print('Adding files to index...')
    subprocess.check_call(['git', 'add', install_subdir])

    print('Committing update...')
    message_lines = [
        'Update prebuilt RenderScript to build {}.'.format(build_number),
        '',
        'Built from {build_branch}, build {build_number}.'.format(
            build_branch=android_branch,
            build_number=build_number),
    ]
    if bug is not None:
        message_lines.append('')
        message_lines.append('Bug: http://b/{}'.format(bug))
    message = '\n'.join(message_lines)
    subprocess.check_call(['git', 'commit', '-m', message])


def main():
    args = ArgParser().parse_args()

    download_dir = os.path.realpath('.download')
    if os.path.isdir(download_dir):
        shutil.rmtree(download_dir)
    os.makedirs(download_dir)

    try:
        hosts = ('darwin', 'linux', 'windows')
        for host in hosts:
            update_renderscript(host, args.build_number, args.android_branch,
                                args.use_current_branch, download_dir, args.bug)
    finally:
        shutil.rmtree(download_dir)


if __name__ == '__main__':
    main()
