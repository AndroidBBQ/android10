#!/usr/bin/python

#
# Copyright 2018, The Android Open Source Project
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

import EddParser
from PFWScriptGenerator import PfwScriptTranslator
import hostConfig

import argparse
import re
import sys
import tempfile
import os
import logging
import subprocess
import xml.etree.ElementTree as ET

#
# In order to build the XML Settings file at build time, an instance of the parameter-framework
# shall be started and fed with all the criterion types/criteria that will be used by
# the engineconfigurable.
# This scripts allows generates the settings from the same audio_criterion_types.xml /
# audio_criteria.xml files used at run time by the engineconfigurable
#

def parseArgs():
    argparser = argparse.ArgumentParser(description="Parameter-Framework XML \
        Settings file generator.\n\
        Exit with the number of (recoverable or not) error that occured.")
    argparser.add_argument('--toplevel-config',
            help="Top-level parameter-framework configuration file. Mandatory.",
            metavar="TOPLEVEL_CONFIG_FILE",
            required=True)
    argparser.add_argument('--criteria',
            help="Criteria file, in XML format: \
                  in '<criteria> \
                          <criterion name="" type=""/> \
                      </criteria>' \
        format. Mandatory.",
            metavar="CRITERIA_FILE",
            type=argparse.FileType('r'),
            required=True)
    argparser.add_argument('--criteriontypes',
            help="Criterion types XML file, in \
            '<criterion_types> \
                <criterion_type name="" type=<inclusive|exclusive> values=<value1,value2,...>/> \
             </criterion_types>' \
        format. Mandatory.",
            metavar="CRITERION_TYPE_FILE",
            type=argparse.FileType('r'),
            required=False)
    argparser.add_argument('--initial-settings',
            help="Initial XML settings file (containing a \
        <ConfigurableDomains>  tag",
            nargs='?',
            default=None,
            metavar="XML_SETTINGS_FILE")
    argparser.add_argument('--add-domains',
            help="List of single domain files (each containing a single \
        <ConfigurableDomain> tag",
            metavar="XML_DOMAIN_FILE",
            nargs='*',
            dest='xml_domain_files',
            default=[])
    argparser.add_argument('--add-edds',
            help="List of files in EDD syntax (aka \".pfw\" files)",
            metavar="EDD_FILE",
            type=argparse.FileType('r'),
            nargs='*',
            default=[],
            dest='edd_files')
    argparser.add_argument('--schemas-dir',
            help="Directory of parameter-framework XML Schemas for generation \
        validation",
            default=None)
    argparser.add_argument('--target-schemas-dir',
            help="Ignored. Kept for retro-compatibility")
    argparser.add_argument('--validate',
            help="Validate the settings against XML schemas",
            action='store_true')
    argparser.add_argument('--verbose',
            action='store_true')

    return argparser.parse_args()

#
# Parses audio_criterion_types.xml / audio_criteria.xml files used at run time by the
# engineconfigurable and outputs a dictionnary of criteria.
# For each criteria, the name, type (aka inclusive (bitfield) or exclusive (enum), the values
# are provided.
#
def parseCriteriaAndCriterionTypes(criteriaFile, criterionTypesFile):
    # Parse criteria and criterion types XML files
    #
    criteria_tree = ET.parse(criteriaFile)
    logging.info("Importing criteriaFile {}".format(criteriaFile))
    criterion_types_tree = ET.parse(criterionTypesFile)
    logging.info("Importing criterionTypesFile {}".format(criterionTypesFile))

    criteria_root = criteria_tree.getroot()
    criterion_types_root = criterion_types_tree.getroot()

    all_criteria = []
    for criterion in criteria_root.findall('criterion'):
        criterion_name = criterion.get('name')
        type_name = criterion.get('type')
        logging.info("Importing criterion_name {}".format(criterion_name))
        logging.info("Importing type_name {}".format(type_name))

        for criterion_types in criterion_types_tree.findall('criterion_type'):
            criterion_type_name = criterion_types.get('name')
            if criterion_type_name == type_name:
                criterion_inclusiveness = criterion_types.get('type')

                criterion_values = []

                values_node = criterion_types.find('values')
                if values_node is not None:
                    for value in values_node.findall('value'):
                        criterion_values.append(value.get('literal'))

                if len(criterion_values) == 0:
                    criterion_values.append('')

                logging.info("Importing criterion_type_name {}".format(criterion_type_name))
                logging.info("Importing criterion_inclusiveness {}".format(criterion_inclusiveness))
                logging.info("Importing criterion_values {}".format(criterion_values))

                all_criteria.append({
                    "name" : criterion_name,
                    "inclusive" : criterion_inclusiveness,
                    "values" : criterion_values})
                break

    return all_criteria

#
# Parses the Edd files (aka .pfw extension file), which is a simplified language to write the
# parameter framework settings.
#
def parseEdd(EDDFiles):
    parsed_edds = []

    for edd_file in EDDFiles:
        try:
            root = EddParser.Parser().parse(edd_file)
        except EddParser.MySyntaxError as ex:
            logging.critical(str(ex))
            logging.info("EXIT ON FAILURE")
            exit(2)

        try:
            root.propagate()
        except EddParser.MyPropagationError, ex :
            logging.critical(str(ex))
            logging.info("EXIT ON FAILURE")
            exit(1)

        parsed_edds.append((edd_file.name, root))
    return parsed_edds

#
# Generates all the required commands to be sent to the instance of parameter-framework launched
# at runtime to generate the XML Settings file.
# It takes as input the collection of criteria, the domains and the simplified settings read from
# pfw.
#
def generateDomainCommands(logging, all_criteria, initial_settings, xml_domain_files, parsed_edds):
        # create and inject all the criteria
        logging.info("Creating all criteria")
        for criterion in all_criteria:
            yield ["createSelectionCriterion", criterion['inclusive'],
                   criterion['name']] + criterion['values']

        yield ["start"]

        # Import initial settings file
        if initial_settings:
            logging.info("Importing initial settings file {}".format(initial_settings))
            yield ["importDomainsWithSettingsXML", initial_settings]

        # Import each standalone domain files
        for domain_file in xml_domain_files:
            logging.info("Importing single domain file {}".format(domain_file))
            yield ["importDomainWithSettingsXML", domain_file]

        # Generate the script for each EDD file
        for filename, parsed_edd in parsed_edds:
            logging.info("Translating and injecting EDD file {}".format(filename))
            translator = PfwScriptTranslator()
            parsed_edd.translate(translator)
            for command in translator.getScript():
                yield command

#
# Entry point of the domain generator.
#       -Parses Criterion types and criteria files
#       -Parses settings written in simplified pfw language.
#       -Launches a parameter-framework
#       -Translates the settings into command that can be interpreted by parameter-framework.
#       -Use the exports command and output them in XML Settings file.
#
def main():
    logging.root.setLevel(logging.INFO)
    args = parseArgs()

    all_criteria = parseCriteriaAndCriterionTypes(args.criteria, args.criteriontypes)

    #
    # EDD files (aka ".pfw" files)
    #
    parsed_edds = parseEdd(args.edd_files)

    # We need to modify the toplevel configuration file to account for differences
    # between development setup and target (installation) setup, in particular, the
    # TuningMwith ode must be enforced, regardless of what will be allowed on the target
    fake_toplevel_config = tempfile.NamedTemporaryFile(mode='w', delete=False, suffix=".xml",
                                                       prefix="TMPdomainGeneratorPFConfig_")

    install_path = os.path.dirname(os.path.realpath(args.toplevel_config))
    hostConfig.configure(
            infile=args.toplevel_config,
            outfile=fake_toplevel_config,
            structPath=install_path)
    fake_toplevel_config.close()

    # Create the connector. Pipe its input to us in order to write commands;
    # connect its output to stdout in order to have it dump the domains
    # there; connect its error output to stderr.
    connector = subprocess.Popen(["domainGeneratorConnector",
                            fake_toplevel_config.name,
                            'verbose' if args.verbose else 'no-verbose',
                            'validate' if args.validate else 'no-validate',
                            args.schemas_dir],
                           stdout=sys.stdout, stdin=subprocess.PIPE, stderr=sys.stderr)

    initial_settings = None
    if args.initial_settings:
        initial_settings = os.path.realpath(args.initial_settings)

    for command in generateDomainCommands(logging, all_criteria, initial_settings,
                                       args.xml_domain_files, parsed_edds):
        connector.stdin.write('\0'.join(command))
        connector.stdin.write("\n")

    # Closing the connector's input triggers the domain generation
    connector.stdin.close()
    connector.wait()
    os.remove(fake_toplevel_config.name)
    return connector.returncode

# If this file is directly executed
if __name__ == "__main__":
    sys.exit(main())
