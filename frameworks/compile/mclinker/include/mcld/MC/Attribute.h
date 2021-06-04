//===- Attribute.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MC_ATTRIBUTE_H_
#define MCLD_MC_ATTRIBUTE_H_

namespace mcld {

class AttributeSet;

/** \class AttributeBase
 *  \brief AttributeBase provides the real storage for attributes of options.
 *
 *  Attributes are options affecting the link editing of input files.
 *  Some options affects the input files mentioned on the command line after
 *  them. For example, --whole-archive option affects archives mentioned on
 *  the command line after the --whole-archve option. We call such options
 *  "attributes of input files"
 *
 *  AttributeBase is the storage for attributes of input files. Each input
 *  file (@see mcld::Input in MCLinker) has a pointer of an attribute. Since
 *  most attributes of input files are identical, our design lets input files
 *  which have identical attributes share common attribute. AttributeBase is
 *  the shared storage for attribute.
 */
class AttributeBase {
 public:
  AttributeBase()
      : m_WholeArchive(false),
        m_AsNeeded(false),
        m_AddNeeded(true),
        m_Static(false) {}

  AttributeBase(const AttributeBase& pBase)
      : m_WholeArchive(pBase.m_WholeArchive),
        m_AsNeeded(pBase.m_AsNeeded),
        m_AddNeeded(pBase.m_AddNeeded),
        m_Static(pBase.m_Static) {}

  virtual ~AttributeBase() {}

  // ----- observers  ----- //
  // represent GNU ld --whole-archive/--no-whole-archive options
  bool isWholeArchive() const { return m_WholeArchive; }

  // represent GNU ld --as-needed/--no-as-needed options
  bool isAsNeeded() const { return m_AsNeeded; }

  // represent GNU ld --add-needed/--no-add-needed options
  bool isAddNeeded() const { return m_AddNeeded; }

  // represent GNU ld -static option
  bool isStatic() const { return m_Static; }

  // represent GNU ld -call_shared option
  bool isDynamic() const { return !m_Static; }

 public:
  bool m_WholeArchive : 1;
  bool m_AsNeeded : 1;
  bool m_AddNeeded : 1;
  bool m_Static : 1;
};

/** \class Attribute
 *  \brief The base class of attributes. Providing the raw operations of an
 *  attributes
 *
 *  For conventience and producing less bugs, we move the stoarges of attributes
 *  onto AttributeBase, and modifiers remains with the class Attribute.
 */
class Attribute : public AttributeBase {
 public:
  // -----  modifiers  ----- //
  void setWholeArchive() { m_WholeArchive = true; }

  void unsetWholeArchive() { m_WholeArchive = false; }

  void setAsNeeded() { m_AsNeeded = true; }

  void unsetAsNeeded() { m_AsNeeded = false; }

  void setAddNeeded() { m_AddNeeded = true; }

  void unsetAddNeeded() { m_AddNeeded = false; }

  void setStatic() { m_Static = true; }

  void setDynamic() { m_Static = false; }
};

/** \class AttrConstraint
 *  \brief AttrConstarint is the constraint of a system.
 *
 *  Some systems can not enable certain attributes of a input file.
 *  For example, systems which have no shared libraries can not enable
 *  --call_shared options. We call the ability of enabling attributes
 *  as the constraint of attributes of a system.
 *
 *  Systems enable attributes at the target implementation of SectLinker.
 *
 *  @see SectLinker
 */
class AttrConstraint : public AttributeBase {
 public:
  void enableWholeArchive() { m_WholeArchive = true; }

  void disableWholeArchive() { m_WholeArchive = false; }

  void enableAsNeeded() { m_AsNeeded = true; }

  void disableAsNeeded() { m_AsNeeded = false; }

  void enableAddNeeded() { m_AddNeeded = true; }

  void disableAddNeeded() { m_AddNeeded = false; }

  void setSharedSystem() { m_Static = false; }

  void setStaticSystem() { m_Static = true; }

  bool isSharedSystem() const { return !m_Static; }

  bool isStaticSystem() const { return m_Static; }

  bool isLegal(const Attribute& pAttr) const;
};

/** \class AttributeProxy
 *  \brief AttributeProxys is the illusion of private attribute of each
 *  input file.
 *
 *  We designers want to hide the details of sharing common attributes
 *  between input files. We want input files under the illusion that they
 *  have their own private attributes to simplify the linking algorithms.
 *
 *  AttributeProxy hides the reality of sharing. An input file can change
 *  its attribute without explicit searching of existing attributes
 *  as it has a private ownership of the attribute. AttributeProxy does
 *  the searching in the AttributeSet and changes the pointer of
 *  the attribute of the input file. If the searching fails, AttributeProxy
 *  requests a new attribute from the AttributeSet.
 */
class AttributeProxy {
 public:
  AttributeProxy(AttributeSet& pParent,
                 const Attribute& pBase,
                 const AttrConstraint& pConstraint);

  ~AttributeProxy();

  // ----- observers  ----- //
  bool isWholeArchive() const;

  bool isAsNeeded() const;

  bool isAddNeeded() const;

  bool isStatic() const;

  bool isDynamic() const;

  const Attribute* attr() const { return m_pBase; }

  // -----  modifiers  ----- //
  void setWholeArchive();
  void unsetWholeArchive();
  void setAsNeeded();
  void unsetAsNeeded();
  void setAddNeeded();
  void unsetAddNeeded();
  void setStatic();
  void setDynamic();

  AttributeProxy& assign(Attribute* pBase);

 private:
  AttributeSet& m_AttrPool;
  const Attribute* m_pBase;
  const AttrConstraint& m_Constraint;
};

// -----  comparisons  ----- //
inline bool operator==(const Attribute& pLHS, const Attribute& pRHS) {
  return ((pLHS.isWholeArchive() == pRHS.isWholeArchive()) &&
          (pLHS.isAsNeeded() == pRHS.isAsNeeded()) &&
          (pLHS.isAddNeeded() == pRHS.isAddNeeded()) &&
          (pLHS.isStatic() == pRHS.isStatic()));
}

inline bool operator!=(const Attribute& pLHS, const Attribute& pRHS) {
  return !(pLHS == pRHS);
}

}  // namespace mcld

#endif  // MCLD_MC_ATTRIBUTE_H_
