//===- ELFAttributeValue.h ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_ELFATTRIBUTEVALUE_H_
#define MCLD_TARGET_ELFATTRIBUTEVALUE_H_

#include <string>

namespace mcld {

/** \class ELFAttributeValue
 *  \brief ELFAttributeValue stroes the value of an attribute tag. The attribtue
 *  tag itself is not stored in this object.
 */
class ELFAttributeValue {
 public:
  // Type of value that an attribute tag holds.
  enum Type {
    // The value contains no data and has unknown type.
    Uninitialized = 0,

    // The value contains integer data.
    Int = 1L << 0,

    // The value contains string data.
    String = 1L << 1,

    // This is for attribute in which "default value" (0 for int type and empty
    // string for string type) has special meaning for them. That is, the
    // default value is "disabled" and meaningful for those attribute.
    NoDefault = 1L << 2,
  };

 public:
  ELFAttributeValue() : m_Type(Uninitialized), m_IntValue(0), m_StringValue() {}

  ~ELFAttributeValue() {}

 public:
  unsigned int type() const { return m_Type; }

  void setType(unsigned int pType) { m_Type = pType; }

  unsigned int getIntValue() const { return m_IntValue; }

  void setIntValue(unsigned int pIntValue) { m_IntValue = pIntValue; }

  const std::string& getStringValue() const { return m_StringValue; }

  void setStringValue(const std::string& pStringValue) {
    m_StringValue = pStringValue;
  }

  void setStringValue(const char* pStringValue, size_t pSize) {
    m_StringValue.assign(pStringValue, pSize);
  }

  void setStringValue(const char* pStringValue) {
    m_StringValue.assign(pStringValue);
  }

  size_t getSize() const;

  inline bool isUninitialized() const { return (m_Type == Uninitialized); }

  inline bool isInitialized() const { return !isUninitialized(); }

  inline bool isIntValue() const { return (m_Type & Int); }

  inline bool isStringValue() const { return (m_Type & String); }

  inline bool hasNoDefault() const { return (m_Type & NoDefault); }

  bool isDefaultValue() const;

  // Returns true if this attribute value should be emitted to the output.
  inline bool shouldEmit() const {
    // Attribute with non-default value should be emitted.
    return !isDefaultValue();
  }

  bool equals(const ELFAttributeValue& pValue) const;

  bool operator==(const ELFAttributeValue& pValue) const {
    return equals(pValue);
  }
  bool operator!=(const ELFAttributeValue& pValue) const {
    return !equals(pValue);
  }

  /// reset - reset this value to the uninitialized state
  void reset() {
    m_Type = Uninitialized;
    m_IntValue = 0;
    m_StringValue.clear();
    return;
  }

 private:
  unsigned int m_Type;

  unsigned int m_IntValue;
  std::string m_StringValue;
};

}  // namespace mcld

#endif  // MCLD_TARGET_ELFATTRIBUTEVALUE_H_
