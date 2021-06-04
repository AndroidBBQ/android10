#!/usr/bin/python

#
# Copyright 2019, The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import re
import sys
import tempfile
import os
import logging
import subprocess
import xml.etree.ElementTree as ET
import xml.etree.ElementInclude as EI
import xml.dom.minidom as MINIDOM
from collections import OrderedDict

#
# Helper script that helps to feed at build time the XML Product Strategies Structure file file used
# by the engineconfigurable to start the parameter-framework.
# It prevents to fill them manually and avoid divergences with android.
#
# The Product Strategies Structure file is fed from the audio policy engine configuration file
# in order to discover all the strategies available for the current platform.
#           --audiopolicyengineconfigurationfile <path/to/audio_policy_engine_configuration.xml>
#
# The reference file of ProductStrategies structure must also be set as an input of the script:
#           --productstrategiesstructurefile <path/to/structure/file/ProductStrategies.xml.in>
#
# At last, the output of the script shall be set also:
#           --outputfile <path/to/out/<system|vendor|odm>/etc/ProductStrategies.xml>
#

def parseArgs():
    argparser = argparse.ArgumentParser(description="Parameter-Framework XML \
        product strategies structure file generator.\n\
        Exit with the number of (recoverable or not) error that occured.")
    argparser.add_argument('--audiopolicyengineconfigurationfile',
            help="Android Audio Policy Engine Configuration file, Mandatory.",
            metavar="(AUDIO_POLICY_ENGINE_CONFIGURATION_FILE)",
            type=argparse.FileType('r'),
            required=True)
    argparser.add_argument('--productstrategiesstructurefile',
            help="Product Strategies Structure XML base file, Mandatory.",
            metavar="STRATEGIES_STRUCTURE_FILE",
            type=argparse.FileType('r'),
            required=True)
    argparser.add_argument('--outputfile',
            help="Product Strategies Structure output file, Mandatory.",
            metavar="STRATEGIES_STRUCTURE_OUTPUT_FILE",
            type=argparse.FileType('w'),
            required=True)
    argparser.add_argument('--verbose',
            action='store_true')

    return argparser.parse_args()


def generateXmlStructureFile(strategies, strategyStructureInFile, outputFile):

    logging.info("Importing strategyStructureInFile {}".format(strategyStructureInFile))
    strategies_in_tree = ET.parse(strategyStructureInFile)

    strategies_root = strategies_in_tree.getroot()
    strategy_components = strategies_root.find('ComponentType')

    for strategy_name in strategies:
        context_mapping = "".join(map(str, ["Name:", strategy_name]))
        strategy_pfw_name = strategy_name.replace('STRATEGY_', '').lower()
        strategy_component_node = ET.SubElement(strategy_components, "Component", Name=strategy_pfw_name, Type="ProductStrategy", Mapping=context_mapping)

    xmlstr = ET.tostring(strategies_root, encoding='utf8', method='xml')
    reparsed = MINIDOM.parseString(xmlstr)
    prettyXmlStr = reparsed.toprettyxml(newl='\r\n')
    prettyXmlStr = os.linesep.join([s for s in prettyXmlStr.splitlines() if s.strip()])
    outputFile.write(prettyXmlStr.encode('utf-8'))

def capitalizeLine(line):
    return ' '.join((w.capitalize() for w in line.split(' ')))


#
# Parse the audio policy configuration file and output a dictionary of device criteria addresses
#
def parseAndroidAudioPolicyEngineConfigurationFile(audiopolicyengineconfigurationfile):

    logging.info("Checking Audio Policy Engine Configuration file {}".format(audiopolicyengineconfigurationfile))
    #
    # extract all product strategies name from audio policy engine configuration file
    #
    strategy_names = []

    oldWorkingDir = os.getcwd()
    print "Current working directory %s" % oldWorkingDir

    newDir = os.path.join(oldWorkingDir , audiopolicyengineconfigurationfile.name)

    policy_engine_in_tree = ET.parse(audiopolicyengineconfigurationfile)
    os.chdir(os.path.dirname(os.path.normpath(newDir)))

    print "new working directory %s" % os.getcwd()

    policy_engine_root = policy_engine_in_tree.getroot()
    EI.include(policy_engine_root)

    os.chdir(oldWorkingDir)

    for strategy in policy_engine_root.iter('ProductStrategy'):
        strategy_names.append(strategy.get('name'))

    return strategy_names


def main():
    logging.root.setLevel(logging.INFO)
    args = parseArgs()

    strategies = parseAndroidAudioPolicyEngineConfigurationFile(args.audiopolicyengineconfigurationfile)

    product_strategies_structure = args.productstrategiesstructurefile

    generateXmlStructureFile(strategies, product_strategies_structure, args.outputfile)

# If this file is directly executed
if __name__ == "__main__":
    sys.exit(main())
