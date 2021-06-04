#!/usr/bin/env python
#
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

import getopt
import json
import string
import sys

def generate_header_file(filename, with_guard, printer):
    f = open(filename, 'w')
    orig_stdout = sys.stdout
    sys.stdout = f
    print '// DO NOT MODIFY. AUTO-GENERATED.\n'
    if with_guard:
        print "#pragma once"
        print
        print "namespace android {"
        print "namespace spirit {"
        print
    printer()
    if with_guard:
        print "} // namespace spirit"
        print "} // namespace android"
    f.close()
    sys.stdout = orig_stdout



################################################################################
#
# Generate Builder class: the .h and .cpp files.
#
################################################################################

def factory_method_name(opname, outlined):
    if outlined:
        return "Builder::Make%s" % opname_noprefix(opname)
    else:
        return "Make%s" % opname_noprefix(opname)



def factory_method_prototype(inst, outlined):
    opname = inst['opname']
    operands = inst.get('operands')
    str = "%s *%s(" % (class_name(opname), factory_method_name(opname, outlined))
    first = True;
    for type, var, quantifier, comment in generate_member_list(operands):
        if var != "mResult":
            param = var[1:]
            if first:
                first = False
            else:
                str += ', '
            if quantifier == '?':
                str += '%s *%s=nullptr' % (type, param)
            elif quantifier == '*':
                vecTy = "std::vector<%s>" % type
                str += '%s %s=%s()' % (vecTy, param, vecTy)
            else:
                str +=  '%s %s' % (type, param)
    str += ')'
    return str



def factory_method_body(inst):
    opname = inst['opname']
    operands = inst.get('operands')
    clazz = class_name(opname)
    str = "%s *ret = new %s(" % (clazz, clazz)
    first = True
    for type, var, quantifier, comment in generate_member_list(operands):
        if var != "mResult" and quantifier != '*' and quantifier != '?':
            param = var[1:]
            if first:
                first = False
            else:
                str += ', '
            str += param
    str += """);
    if (!ret) {
        return nullptr;
    }
"""
    str += """
    if (ret->hasResult()) {
      ret->setId(Module::getCurrentModule()->nextId());
    }
"""
    for type, var, quantifier, comment in generate_member_list(operands):
        param = var[1:]
        # TODO: use vector::swap() or move instead of copy
        if quantifier == '?' or quantifier == '*':
            str += "    ret->%s = %s;\n" % (var, param)
    str += "    return ret;"
    return str



def print_factory_method(inst):
    print """%s {
    %s
}""" % (factory_method_prototype(inst, False),
        factory_method_body(inst))



def print_factory_methods(insts):
    for inst in insts:
        print_factory_method(inst)



################################################################################
#
# Generate type defintions
#
################################################################################

def enum_enumerants(ty, enumerants):
    str = ""
    for enumerant in enumerants:
        name = enumerant['enumerant']
        val = enumerant['value']
        if name[0].isalpha():
            str += "  %s = %sU,\n" % (name, val)
        else:
            str += "  %s%s = %sU,\n" % (ty, name, val)
    return str


def generate_enum(ty):
    typeName = ty['kind']
    print """enum class %s : uint32_t {\n%s};
""" % (typeName,
       enum_enumerants(typeName, ty['enumerants']))


def generate_composite_fields(bases):
    str = ""
    i = 0
    for field in bases:
        str += "  %s mField%d;\n" % (field, i)
        i = i + 1
    return str



def print_type_definitions(operand_kinds):
    for ty in operand_kinds:
        category = ty['category']
        if category == 'BitEnum' or category == 'ValueEnum':
            generate_enum(ty)
        elif category == 'Composite':
            print "struct %s {\n%s};\n" % (ty['kind'],
                                           generate_composite_fields(ty['bases']))



################################################################################
#
# Generate class defintions for all instructions
#
################################################################################

def opname_noprefix(opname):
    return opname[2:]

def class_name(opname):
    return "%sInst" % opname_noprefix(opname)



def generate_member_list(operands):
    members = []
    if operands is None:
        return members
    index = 1
    for operand in operands:
        type = operand['kind']
        if type == 'IdResultType' or type == 'IdResult':
            varName = "m%s" % type[2:]
        else:
            varName = "mOperand%d" % index
            index = index + 1
        quantifier = operand.get('quantifier')
        comment = operand.get('name');
        members.append((type, varName, quantifier, comment))
    return members

def fixed_word_count(member_list):
    wc = 1 # for the first word of opcode and word count
    for type, var, quant, comment in member_list:
        if quant != '?' and quant != '*':
            wc += 1
    return wc

def string_for_members(opname, member_list):
    if member_list == []:
        return ""
    member_str = "\n  static constexpr OpCode mOpCode=%s;\n" % opname
    for type, var, quantifier, comment in member_list:
        if comment is not None and comment.find('\n') != -1:
            member_str += "  /* %s\n  */\n" % comment
        member_str += "  "
        if quantifier == '?':
            type = type + '*';
        elif quantifier == '*':
            type = 'std::vector<%s>' % type;
        member_str += "%s %s;" % (type, var)
        if comment is not None and comment.find('\n') == -1:
            member_str += "  // %s" % comment
        member_str += "\n"
    return member_str

def string_for_constructor(opname, opcode, members):
    # Default constructor
    initializer = "Instruction(%s, %d)" % (opname, fixed_word_count(members))
    first = True
    for type, var, quantifier, comment in members:
        if quantifier == '?':
            initializer += ", %s(nullptr)" % var
    str = "%s() : %s {}" % (class_name(opname), initializer)

    # Constructor with values for members
    if members == [] or (len(members) == 1 and members[0][0]=='IdResult'):
        return str
    nonOptionalOperandExists = False
    for type, var, quantifier, comment in members:
        if quantifier is None:
            nonOptionalOperandExists = True
    if not nonOptionalOperandExists:
        return str
    params = ""
    initializer = "Instruction(%s, %d)" % (opname, fixed_word_count(members))
    first = True
    for type, var, quantifier, comment in members:
        if var != "mResult" and quantifier != '*':
            initializer += ", "
            if quantifier == '?':
                initializer += "%s(nullptr)" % var
            else:
                if first:
                    first = False
                else:
                    params += ", "
                param = var[1:]  # remove the prefix "m"
                params += "%s %s" % (type, param)
                initializer += "%s(%s)" % (var, param)
    if params != "":
        str += "\n  %s(%s) :\n    %s {}" % (class_name(opname), params, initializer)
    str += "\n  virtual ~%s() {}" % class_name(opname)
    return str

def string_for_serializer_body(opcode, members):
    body =  "setWordCount();\n"
    body += "    OS << mCodeAndCount;\n"
    for type, var, quantifier, comment in members:
        if quantifier == '?':
            body += "    if (%s!=nullptr) { OS << *%s; }\n" % (var, var)
        elif quantifier == '*':
            body += """    for (auto val : %s) { OS << val; }\n""" % var
        else:
            body += "    OS << %s;\n" % var
    body += "    SerializeExtraOperands(OS);\n"
    return body

def string_for_deserializer_body(name, members):
    body = "return DeserializeFirstWord(IS, %s)" % name
    for type, var, quantifier, comment in members:
        body += " &&\n           "
        if quantifier == '?':
            body += "DeserializeOptionallyOne(IS, &%s)" % var
        elif quantifier == '*':
            body += "DeserializeZeroOrMoreOperands(IS, &%s)" % var
        else:
            body += "DeserializeExactlyOne(IS, &%s)" % var
    body += " &&\n           DeserializeExtraOperands(IS);\n"
    return body

def string_for_get_word_count(members):
    str = """uint16_t getWordCount() const override {
    uint16_t count = mFixedWordCount;\n"""
    for type, var, quantifier, comment in members:
        if quantifier == '?':
            str += "    if (%s) count += WordCount(*%s);\n" % (var, var)
        elif quantifier == '*':
            str += "    if (!%s.empty()) count += WordCount(%s[0]) * %s.size();\n" % (var, var, var)
        elif type == 'LiteralString':
            str += "    count += WordCount(%s) - 1;\n" % var

    str += """    count += mExtraOperands.size();
    return count;
  }"""
    return str

def string_for_accept():
    return """
  void accept(IVisitor *v) override { v->visit(this); }
"""

def has_result(members):
    for type, val, quantifier, comment in members:
        if type == 'IdResult':
            return True
    return False


def string_for_has_result(hasResult):
    if hasResult:
        retVal = "true"
    else:
        retVal = "false"
    return "bool hasResult() const override { return %s; }" % retVal

def string_for_get_all_refs(members):
    str = """std::vector<const IdRef*> getAllIdRefs() const override {
    std::vector<const IdRef*> ret = {"""
    first = True
    # TODO: what if references are in * operands?
    for type, var, quantifier, comment in members:
        if type == 'IdRef' or type == 'IdResultType' or type == 'IdMemorySemantics' or type == 'IdScope':
            if quantifier == '*':
                pass
            else:
                if first:
                    first = False
                else:
                    str += ", "
                if quantifier == '?':
                    str += "%s" % var
                else:
                    str += "&%s" % var
    str += """};
"""
    for type, var, quantifier, comment in members:
        if type == 'IdRef' or type == 'IdResultType' or type == 'IdMemorySemantics' or type == 'IdScope':
            if quantifier == '*':
                str+="""
    for(const auto &ref : %s) {
        ret.push_back(&ref);
    }
""" % var
    str += """
    return ret;
  }"""
    return str

def string_for_get_set_id(hasResult):
    if hasResult:
        return """IdResult getId() const override { return mResult; }
  void setId(IdResult id) override { mResult = id; }"""
    else:
        retVal = "0"
        return """IdResult getId() const override { return 0; }
  void setId(IdResult) override {}"""


def print_instruction_class(inst):
    opname = inst['opname']
    opcode = inst['opcode']
    operands = inst.get('operands')
    members = generate_member_list(operands)
    hasResult = has_result(members)
    print """class %s : public Instruction {
 public:
  %s

  void Serialize(OutputWordStream &OS) const override {
    %s  }

  bool DeserializeInternal(InputWordStream &IS) override {
    %s  }

  void accept(IVisitor *v) override { v->visit(this); }

  %s

  %s

  %s

  %s
%s};
""" % (class_name(opname),
       string_for_constructor(opname, opcode, members),
       string_for_serializer_body(opcode, members),
       string_for_deserializer_body(opname, members),
       string_for_get_word_count(members),
       string_for_has_result(hasResult),
       string_for_get_set_id(hasResult),
       string_for_get_all_refs(members),
       string_for_members(opname, members))

def print_all_instruction_classes(insts):
    for inst in insts:
        print_instruction_class(inst)

################################################################################
#
# Generate opcode enum
#
################################################################################

def print_opcode_enum(insts):
    print "enum OpCode {"
    for inst in insts:
        opname = inst['opname']
        opcode = inst['opcode']
        print "  %s = %d," % (opname, opcode)
    print "};"



################################################################################
#
# Generate dispatching code
#
################################################################################

def print_dispatches(insts):
    for inst in insts:
        opname = inst['opname']
        print "HANDLE_INSTRUCTION(%s,%s)" % (opname, class_name(opname))

def print_type_inst_dispatches(insts):
    for inst in insts:
        opname = inst['opname']
        if opname[:6] == "OpType":
            print "HANDLE_INSTRUCTION(%s, %s)" % (opname, class_name(opname))

def print_const_inst_dispatches(insts):
    for inst in insts:
        opname = inst['opname']
        if opname[:10] == "OpConstant":
            print "HANDLE_INSTRUCTION(%s, %s)" % (opname, class_name(opname))

def print_enum_dispatches(operand_kinds):
    for ty in operand_kinds:
        category = ty['category']
        if category == 'BitEnum' or category == 'ValueEnum':
            print "HANDLE_ENUM(%s)" % ty['kind']



################################################################################
#
# main
#
################################################################################

def main():
    try:
        opts, args = getopt.getopt(sys.argv[2:],"h",["instructions=",
                                                     "types=",
                                                     "opcodes=",
                                                     "instruction_dispatches=",
                                                     "enum_dispatches=",
                                                     "type_inst_dispatches=",
                                                     "const_inst_dispatches=",
                                                     "factory_methods="])
    except getopt.GetoptError:
        print sys.argv[0], ''
        sys.exit(2)

    with open(sys.argv[1]) as grammar_file:
        grammar = json.load(grammar_file)

    instructions = grammar['instructions']

    for opt, arg in opts:
        if opt == "--instructions":
            generate_header_file(arg, True, lambda: print_all_instruction_classes(instructions))
        elif opt == "--types":
            kinds = grammar['operand_kinds']
            generate_header_file(arg, True, lambda: print_type_definitions(kinds))
        elif opt == "--opcodes":
            generate_header_file(arg, True, lambda: print_opcode_enum(instructions))
        elif opt == "--instruction_dispatches":
            generate_header_file(arg, False, lambda: print_dispatches(instructions))
        elif opt == "--enum_dispatches":
            kinds = grammar['operand_kinds']
            generate_header_file(arg, False, lambda: print_enum_dispatches(kinds))
        elif opt == "--type_inst_dispatches":
            generate_header_file(arg, False, lambda: print_type_inst_dispatches(instructions))
        elif opt == "--const_inst_dispatches":
            generate_header_file(arg, False, lambda: print_const_inst_dispatches(instructions))
        elif opt == "--factory_methods":
            generate_header_file(arg, False, lambda: print_factory_methods(instructions))

if __name__ == '__main__':
    main()
