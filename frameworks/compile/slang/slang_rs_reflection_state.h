/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FRAMEWORKS_COMPILE_SLANG_REFLECTION_STATE_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_REFLECTION_STATE_H_

#include <string>
#include <utility>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "clang/AST/APValue.h"

#include "slang_assert.h"

namespace slang {

class RSContext;
class RSExportForEach;
class RSExportFunc;
class RSExportRecordType;
class RSExportReduce;
class RSExportType;
class RSExportVar;

// ---------------------
// class ReflectionState
// ---------------------
//
// This class is used to collect data from 32-bit compilation for use
// during the reflected code generation that occurs during 64-bit
// compilation.  The data is used for two purposes:
//
// 1) Accommodating rs object handle size differences when laying out
//    data (in particular, variables and records).
// 2) Emitting errors when differences between 32-bit and 64-bit
//    compilation cannot be tolerated in the reflected code (for
//    example, size_t has different sizes, and so cannot be part
//    of any exportable).
//
// The model for using this class is as follows:
// a) Instantiate a class instance.  The instance is in the S_Initial
//    state.
// b) Call openJava32() to move the instance to the S_OpenJava32
//    ("Collecting") state.
// c) Run the reflection pass on all files in 32-bit mode.  Do not
//    actually generate reflected code; but call various methods on
//    the instance (begin*(), declare*(), end*(), etc.) to collect
//    information.
// d) Call closeJava32() to move the instance to the S_ClosedJava32
//    state.
// e) Call openJava64() to move the instance to the S_OpenJava64
//    ("Using") state.
// f) Run the reflection pass on all files in 64-bit mode.  Call the
//    same methods as in step (c), as well as some further methods to
//    query the information collected in step (c) in order to handle
//    layout differences.  All error reporting for 32-bit versus
//    64-bit differences is handled in the methods themselves.
// g) Call closeJava64 to move the instance to the S_ClosedJava64
//    state.
// h) Destroy the instance.
//
// There are two exceptions to this model:
//
// 1) If not doing both 32-bit and 64-bit compilation, then skip steps
//    (b), (d), (e), and (g).  (This is what happens if reflecting C++
//    instead of Java, or reflecting Java but using the -m32 or -m64
//    option.)  In this case, the methods called in steps (c) and (f)
//    are no-ops: They do not collect information, they do not report
//    errors, and they return "no information available" when step (f)
//    asks for 32-bit layout related information.
// 2) The class instance can be moved to the S_Bad state by class
//    ReflectionState::Tentative (see that class for more information)
//    when reflection itself aborts due to some error.  The only legal
//    thing to do with an instance in this state is invoke its
//    destructor.
//
// All exported entities except for Records have slot numbers assigned
// in reflection order.  These slot numbers must match up between
// 32-bit and 64-bit compilation.  Therefore, we (generally) require
// that entities be presented to ReflectionState (via begin*() or
// declare*()) in the same order during the Collecting and Using
// phases.  This presentation order is generally the same as lexical
// order in the user code, which makes it simple to emit meaningful
// diagnostics when the order is inconsistent (for example, 32-bit and
// 64-bit compilation disagree on the name of the kernel in a
// particular slot).  ReflectionState generally builds up an array of
// each sort of entity, in the presentation order.  There are two
// exceptions:
//
// a) Records, as mentioned above.  Exported Records have no slot
//    number, and therefore reflection order doesn't matter.  In
//    practice, Records aren't necessarily reflected in consistent
//    order, because they are determined to be exported as a
//    consequence of determining that other entities are to be
//    exported; and variations between 32-bit and 64-bit compilation
//    can therefore result in inconsistent Record reflection order.
//    Therefore, ReflectionState builds up a map of Records.
// b) ForEach kernels.  ForEach kernels are not necessarily reflected
//    in lexical order (there is some sorting to segregate root
//    kernel, old-style kernels, and new-style kernels).  In order to
//    give meaningful diagnostics for slot order mismatches, it's
//    enough to solve the simpler problem of giving meaningful
//    diagnostics for lexical order mismatches (although this is
//    stricter than necessary because of the sorting that occurs
//    before slot assignment).  Therefore, ReflectionState builds up
//    an array of ForEaches in lexical order rather than in
//    presentation (slot) order, and accesses the array randomly
//    rather than sequentially.
//
class ReflectionState {
 private:
  // Set this to true to turn everything into a no-op, just as if none
  // of the open*() or close*() methods were ever called.
  static const bool kDisabled = false;

 public:
  ReflectionState() :
      mRSC(nullptr),
      mState(S_Initial),
      mForEachOpen(-1),
      mOutputClassOpen(false),
      mRecordsState(RS_Initial),
      mStringSet(nullptr) { }
  ~ReflectionState();

  ReflectionState(const ReflectionState &) = delete;
  void operator=(const ReflectionState &) = delete;

  // For use in the debugger.
  void dump();

  // A possibly-present value describing a property for a 32-bit target.
  // When .first is false, the value is absent, and .second is unspecified.
  typedef std::pair<bool, size_t> Val32;
  static Val32 NoVal32() { return Val32(false, ~size_t(0)); }

  void openJava32(size_t NumFiles);
  void closeJava32();
  void openJava64();
  void closeJava64();

  bool isCollecting() const { return mState==S_OpenJava32; }

  // ----------------------------------------------------------------------

  // Use these methods during the "Collecting" phase to track
  // information about a class being generated -- a script class or a
  // type class.  We call such a class "Divergent" if it needs to have
  // at least one runtime check to distinguish between 32-bit and
  // 64-bit targets.
  //
  //   Indicate that we are beginning to generate the class.
  //
  void beginOutputClass() {
    slangAssert(!mOutputClassOpen && !isClosed());
    mOutputClassOpen = true;
    mOutputClassDivergent = false;
  }
  //
  //   Record the fact that we've learned the class is divergent.
  //
  void setOutputClassDivergent() {
    slangAssert(mOutputClassOpen);
    mOutputClassDivergent = true;
  }
  //
  //   Indicate that we've finished generating the class.  Returns
  //   true IFF we've learned the class is divergent.
  //
  bool endOutputClass() {
    slangAssert(mOutputClassOpen);
    mOutputClassOpen = false;
    return mOutputClassDivergent;
  }

  // ----------------------------------------------------------------------

  // --------------------------------
  // class ReflectionState::Tentative
  // --------------------------------
  //
  // This class aids in error handling.  The model is as follows:
  // a) Instantiate the class with a pointer to a ReflectionState
  //    instance.
  // b) Before destroying the class instance, if there have been no
  //    errors, call the ok() method on the instance.
  // c) When the instance is destroyed, if ok() has not been called on
  //    it, this class will put the ReflectionState into the S_Bad
  //    state.
  //
  // The idea is to "poison" the ReflectionState if we quit reflection
  // early because of some error -- we don't want to get in a
  // situation where we only have partial information from the
  // Collecting phase (because of quitting early) but try to use it
  // during the Using phase.
  //
  friend class Tentative;
  class Tentative {
   public:
    Tentative(ReflectionState *state) : mState(state) { }
    ~Tentative() { if (mState) mState->mState = ReflectionState::S_Bad; }

    void ok() { mState = nullptr; }

    Tentative(const Tentative &) = delete;
    void operator=(const Tentative &) = delete;

   private:
    ReflectionState *mState;
  };

  // ----------------------------------------------------------------------

  // Model for ForEach kernels (per File):
  //
  // a) beginForEaches(number_of_non_dummy_root_kernels_in_file)
  // b) mixture of declareForEachDummyRoot() calls and
  //    beginForEach()..endForEach() calls
  // c) endForEaches()
  //
  // For a given ForEach kernel:
  //
  // b1) beginForEach()
  // b2) call any number of addForEachIn() (one per input)
  // b3) call any number of addForEachParam() (one per param)
  // b4) call addForEachSignatureMetadata() (if it's reflected)
  // b5) call endForEach()
  //
  // b2, b3, b4 can occur in any order

  void beginForEaches(size_t Count);

  void declareForEachDummyRoot(const RSExportForEach *) { /* we don't care */ };

  void beginForEach(const RSExportForEach *EF);

  void addForEachIn(const RSExportForEach *EF, const RSExportType *Type);

  void addForEachParam(const RSExportForEach *EF, const RSExportType *Type);

  void addForEachSignatureMetadata(const RSExportForEach *EF, unsigned Metadata);

  void endForEach();

  void endForEaches();

  // ----------------------------------------------------------------------

  // Model for Invokable functions (per File):
  //
  // a) beginInvokables(number_of_invokables_in_file)
  // b) declareInvokable() for each Invokable (order must be
  //    consistent between 32-bit and 64-bit compile)
  // c) endInvokables()

  void beginInvokables(size_t Count) {
    mInvokablesOrderFatal = false;
    begin(&File::mInvokables, Count);
  }

  void declareInvokable(const RSExportFunc *EF);

  void endInvokables();

  // ----------------------------------------------------------------------

  // Model for reduction kernels (per File):
  //
  // a) beginReduces(number_of_reduction_kernels_in_file)
  // b) declareReduce() for each reduction kernel (order must be
  //    consistent between 32-bit and 64-bit compile)
  // c) endReduces()

  void beginReduces(size_t Count) {
    mReducesOrderFatal = false;
    begin(&File::mReduces, Count);
  }

  void declareReduce(const RSExportReduce *ER, bool IsExportable);

  void endReduces();

  // ----------------------------------------------------------------------

  // Model for records (per File):
  //
  // a) beginRecords()
  // b) declareRecord() for each Record (order doesn't matter)
  // c) endRecords()
  //
  // And at any time during the Using phase, can call getRecord32() to
  // get information from the 32-bit compile (Collecting phase).

  void beginRecords();

  // An "Ordinary" record is anything other than an
  // internally-synthesized helper record.  We do not emit diagnostics
  // for mismatched helper records -- we assume that the constructs
  // from which those helper records were derived are also mismatched,
  // and that we'll get diagnostics for those constructs.
  void declareRecord(const RSExportRecordType *ERT, bool Ordinary = true);

  void endRecords();

  class Record32;

  // During the Using phase, obtain information about a Record from
  // the Collecting phase.  ERT should be from the Using phase, not
  // the Collecting phase.  The value returned from this function is
  // valid for the lifetime of the ReflectionState instance.
  Record32 getRecord32(const RSExportRecordType *ERT);

  // ----------------------------------------------------------------------

  // Model for Variables (per file):
  //
  // a) beginVariables(number_of_exported_variables_in_file)
  // b) declareVariable() for each Variable (order must be consistent
  //    between 32-bit and 64-bit); in the Using phase, returns some
  //    information about the Variable from 32-bit compilation
  // c) endVariables()

  void beginVariables(size_t Count) {
    mVariablesOrderFatal = false;
    begin(&File::mVariables, Count);
  }

  // If isUsing(), returns variable's 32-bit AllocSize; otherwise, returns NoVal32().
  Val32 declareVariable(const RSExportVar *EV);

  void endVariables();

  // ----------------------------------------------------------------------

  // ReflectionState has a notion of "current file".  After an
  // openJava*() or closeJava*() call, there is no current file.
  // Calling the nextFile() method when in the Collecting or Using
  // state "advances" to the next file in the list of files being
  // compiled, whose properties are specified by the arguments to
  // nextFile().  All of the various begin*(), declare*(), end*()
  // etc. calls implicitly refer to entities in the current file.
  //
  // RSC must remain valid until the next call to nextFile() or the
  // next S_* state change.
  void nextFile(const RSContext *RSC, const std::string &PackageName, const std::string &RSSourceFileName);

  // ----------------------------------------------------------------------

 private:
  enum State {
    S_Initial,          // No captured information
    S_OpenJava32,       // Capturing information for 32-bit Java
    S_ClosedJava32,     // Captured  information for 32-bit Java
    S_OpenJava64,       // Capturing information for 64-bit Java
    S_ClosedJava64,     // Captured  information for 64-bit Java
    S_Bad,              // Abnormal termination
  };

  // context associated with compilation of the current file
  const RSContext *mRSC;

  State mState;

  /*== ForEach ==================================================================*/

  // The data in this section is transient during ForEach processing
  // for each File.

  int mForEachOpen;    // if nonnegative, then ordinal of beginForEach() without matching endForEach()
  bool mForEachFatal;  // fatal mismatch in comparing ForEach; do no further comparisons for it

  // Tracks mismatches discovered during the Use phase.
  // There are two possibilities:
  // - if (ordinal + 1) is greater than the number of ForEaches from the Collecting phase,
  //   then this is an "extra" ForEach discovered during the Use phase
  // - otherwise the Collecting phase and the Use phase disagree on the name of the
  //   ForEach at this ordinal position (the Collecting phase's kernel name is
  //   available in mFiles.Current().mForEaches[ordinal].mName)
  llvm::SmallVector<const RSExportForEach *, 0> mForEachesBad;

  // During the Use phase, keep track of how many ForEach ordinals we
  // have seen that correspond to ordinals seen during the Collect
  // phase.  This helps determine whether we have to issue errors at
  // endForEaches().
  size_t mNumForEachesMatchedByOrdinal;

  /*== Invokable ================================================================*/

  // 32-bit and 64-bit compiles need to see invokables in the same
  // order, because of slot number assignment.  Once we see the first
  // name mismatch in the sequence of invokables for a given File, it
  // doesn't make sense to issue further diagnostics regarding
  // invokables for that File.
  bool mInvokablesOrderFatal;

  /*== OutputClass ==============================================================*/

  // This data tracks information about a class being generated -- a
  // script class or a type class.  We call such a class "Divergent"
  // if it needs to have at least one runtime check to distinguish
  // between 32-bit and 64-bit targets.

  bool mOutputClassOpen;  // beginOutputClass() without matching endOutputClass()
  bool mOutputClassDivergent;  // has class been marked divergent?

  /*== Record ===================================================================*/

  // This field enforces necessary discipline on the use of
  // beginRecords()/declareRecord()/endRecord().
  enum {
    RS_Initial,  // no beginRecords() yet for current File
    RS_Open,     // beginRecords() but no endRecords() for current File
    RS_Closed    // endRecords() for current File
  } mRecordsState;

  // During the Use phase, keep track of how many records we have seen
  // that have same-named counterparts seen during the Collect phase.
  // This helps determine whether we have to issue errors at
  // endRecords().
  size_t mNumRecordsMatchedByName;

  /*== Reduce ===================================================================*/

  // 32-bit and 64-bit compiles need to see reduction kernels in the
  // same order, because of slot number assignment.  Once we see the
  // first name mismatch in the sequence of reduction kernels for a
  // given File, it doesn't make sense to issue further diagnostics
  // regarding reduction kernels for that File.
  bool mReducesOrderFatal;

  /*== Variable =================================================================*/

  // 32-bit and 64-bit compiles need to see variables in the same
  // order, because of slot number assignment.  Once we see the first
  // name mismatch in the sequence of variables for a given File, it
  // doesn't make sense to issue further diagnostics regarding
  // variables for that File.
  bool mVariablesOrderFatal;

  /*=============================================================================*/

  bool isActive() const { return isCollecting() || isUsing(); }
  bool isClosed() const { return mState==S_ClosedJava32 || mState==S_ClosedJava64; }
  bool isUsing() const { return mState==S_OpenJava64; }

  // For anything with a type (such as a Variable or a Record field),
  // the type is represented via its name.  To save space, we don't
  // create multiple instances of the same name -- we have a canonical
  // instance in mStringSet, and use a StringRef to refer to it.  The
  // method canon() returns a StringRef to the canonical
  // instance, creating the instance if necessary.
  llvm::StringRef canon(const std::string &String);
  llvm::StringSet<> *mStringSet;

  // Synthesize a name for the specified type.  There should be a
  // one-to-one correspondence between the name and a C type (after
  // typedefs and integer expressions have been "flattened", and
  // considering a struct type to be identified solely by its name).
  static std::string getUniqueTypeName(const RSExportType *T);

  // ------------------------------
  // template class ArrayWithCursor
  // ------------------------------
  //
  // This class represents a fixed-length dynamically-allocated array
  // (length is specified by a method call after instantiation) along
  // with a cursor that traverses the array.  The behavior of the
  // class is very specific to the needs of ReflectionState.
  //
  // The model for using this class is as follows:
  // a) Instantiate a class instance.  The instance is in the
  //    S_Initial state.
  // b) Call BeginCollecting() with an array capacity.  This allocates
  //    the array members and moves the instance to the S_Collecting
  //    state.  The array size (contrast with capacity) is zero, and
  //    the cursor has not been placed.
  // c) Call CollectNext() a number of times equal to the capacity.
  //    Each time CollectNext() is called, it extends the array size
  //    by 1, and advances the cursor to the "new" member.  The idea
  //    is to set the value of the "new" member at this time.
  // d) Call BeginUsing().  This moves the instance to the S_Using
  //    state and "unplaces" the cursor.
  // e) Call UseNext() a number of times equal to the capacity.  Each
  //    time UseNext() is called, it advances the cursor to the next
  //    member (first member, the first time it is called).
  //    The cursor is stepping through the members that were "created"
  //    by CollectNext() during the S_Collecting state; the idea is to
  //    look at their values.
  // f) Destroy the instance.
  //
  template <typename Member> class ArrayWithCursor {
   public:
    ArrayWithCursor() : mState(S_Initial), mMembers(nullptr), mCapacity(0), mSize(0), mCursor(~size_t(0)) { }

    ~ArrayWithCursor() { delete [] mMembers; }

    ArrayWithCursor(const ArrayWithCursor &) = delete;
    void operator=(const ArrayWithCursor &) = delete;

    void BeginCollecting(size_t Size) {
      slangAssert(mState == S_Initial);
      mState = S_Collecting;
      mMembers = new Member[Size];
      mCapacity = Size;
    }
    // Increments the array size, advances the cursor to the new
    // member, and returns a reference to that member.
    Member &CollectNext() {
      slangAssert((mState == S_Collecting) && (mCursor + 1 == mSize) && (mSize < mCapacity));
      ++mSize;
      return mMembers[++mCursor];
    }

    void BeginUsing() {
      slangAssert((mState == S_Collecting) && (mCursor + 1 == mSize) && (mSize == mCapacity));
      mState = S_Using;
      mCursor = ~size_t(0);
    }
    // Advances the cursor to the next member, and returns a reference
    // to that member.
    Member &UseNext() {
      slangAssert((mState == S_Using) && (mCursor + 1 < mSize));
      return mMembers[++mCursor];
    }

    // Is the cursor on the last array member?
    bool isFinished() const {
      return mCursor + 1 == mSize;
    }

    size_t Size() const { return mSize; }

    // Return a reference to the member under the cursor.
    Member &Current() {
      slangAssert(mCursor < mSize);
      return mMembers[mCursor];
    }
    const Member &Current() const {
      slangAssert(mCursor < mSize);
      return mMembers[mCursor];
    }
    // Return the cursor position (zero-based).  Cursor must have been
    // placed (i.e., if we're Collecting, we must have called
    // CollectNext() at least once; and if we're Using, we must have
    // called UseNext() at least once).
    size_t CurrentIdx() const {
      slangAssert(mCursor < mSize);
      return mCursor;
    }

    // Return a reference to the specified member.  Must be within the
    // array size (not merely within its capacity).
    Member &operator[](size_t idx) {
      slangAssert(idx < mSize);
      return mMembers[idx];
    }
    const Member &operator[](size_t idx) const {
      slangAssert(idx < mSize);
      return mMembers[idx];
    }

   private:
    enum State { S_Initial, S_Collecting, S_Using };
    State mState;

    Member *mMembers;
    size_t mCapacity;
    size_t mSize;
    size_t mCursor;
  };


  struct File {
    File() : mForEaches(nullptr) { }
    ~File() { delete [] mForEaches; }

    File(const File &) = delete;
    void operator=(const File &) = delete;

    std::string mPackageName;
    std::string mRSSourceFileName;

    struct ForEach {
      ForEach() : mState(S_Initial) { }
      ForEach(const ForEach &) = delete;
      void operator=(const ForEach &) = delete;

      enum {
        S_Initial,    // ForEach has been instantiated
        S_Collected,  // beginForEach() has been called while Collecting
        S_UseMatched  // beginForEach() has been called while Using,
                      //   and found this ForEach
      } mState;

      std::string mName;

      // Types.  mIns[] and mOut can be null in case we have an
      // old-style kernel with a void* input or output.
      ArrayWithCursor<llvm::StringRef> mIns;
      ArrayWithCursor<llvm::StringRef> mParams;
      llvm::StringRef mOut;
      bool mHasOut;  // to distinguish between no output and void* output.

      unsigned mSignatureMetadata;
      bool mIsKernel;  // new-style (by-value) rather than old-style
    };
    ForEach *mForEaches;  // indexed by ordinal (lexical order)
    size_t mForEachCount;

    struct Invokable {
      Invokable() : mParams(nullptr) { }
      ~Invokable() { delete [] mParams; }

      Invokable(const Invokable &) = delete;
      void operator=(const Invokable &) = delete;

      std::string mName;
      llvm::StringRef *mParams;  // Types
      size_t mParamCount;
    };
    ArrayWithCursor<Invokable> mInvokables;

    // There are two things we need to do with a Record:
    // - Support structure sizes and layouts that differ between
    //   32-bit and 64-bit compilation.
    // - Do consistency checking between 32-bit and 64-bit compilation.
    //
    // TODO: Move this out of File to avoid duplication?  That is,
    //       instead of tracking Records on a per-File basis, instead
    //       track them globally?
    //
    //       (Because of ODR, we shouldn't have inconsistencies
    //       between Files.)
    //
    struct Record {
      Record() : mFields(nullptr) { }
      ~Record() { delete [] mFields; }

      Record(const Record &) = delete;
      void operator=(const Record &) = delete;

      struct Field {
        std::string mName;
        llvm::StringRef mType;
        size_t mPrePadding;   // this.OffsetInParent - (prev.OffsetInParent + prev.AllocSize)
        size_t mPostPadding;  // this.AllocSize - this.StoreSize
        size_t mOffset;       // this.OffsetInParent
        size_t mStoreSize;    // this.StoreSize
      };
      Field *mFields;
      size_t mFieldCount;
      size_t mPostPadding;    // padding after the end of the padded
                              //   last field
      size_t mAllocSize;
      bool mOrdinary;         // anything other than an
                              //   internally-synthesized helper
                              //   record.  We do not emit diagnostics
                              //   for inconsistent helper records.
      bool mMatchedByName;    // has declareRecord() been called on
                              //   this record during the Using phase?
    };
    llvm::StringMap<Record> mRecords;

    struct Reduce {
      Reduce() : mAccumIns(nullptr) { }
      ~Reduce() { delete [] mAccumIns; }

      Reduce(const Reduce &) = delete;
      void operator=(const Reduce &) = delete;

      std::string mName;

      // only apply to exportable
      llvm::StringRef *mAccumIns;  // Types
      size_t mAccumInCount;
      llvm::StringRef mResult;  // Type

      bool mIsExportable;
    };
    ArrayWithCursor<Reduce> mReduces;

    struct Variable {
      Variable() : mInitializers(nullptr) { }
      ~Variable() { delete [] mInitializers; }

      Variable(const Variable &) = delete;
      void operator=(const Variable &) = delete;

      std::string mName;
      llvm::StringRef mType;
      clang::APValue *mInitializers;
      size_t mInitializerCount;
      size_t mAllocSize;
      bool mIsConst;
    };
    ArrayWithCursor<Variable> mVariables;

  };
  ArrayWithCursor<File> mFiles;

  // Utility template -- common pattern used by many begin*() methods.
  template <typename Member>
  void begin(ArrayWithCursor<Member> File::*Array, size_t Count) {
    slangAssert(!isClosed());
    if (!isActive())
      return;

    auto &file = mFiles.Current();
    if (isCollecting())
      (file.*Array).BeginCollecting(Count);
    if (isUsing())
      (file.*Array).BeginUsing();
  }

 public:

  // This class represents 32-bit layout information built up during
  // the Collecting phase, for use during the Using phase.  It
  // provides an interface between class ReflectionState and client
  // code that actually performs reflection.
  class Record32 {
    friend class ReflectionState;

   public:
    Record32() : mRecord(nullptr) { }

    Val32 getRecordPostPadding() const {
      if (!mRecord)
        return NoVal32();
      return Val32(true, mRecord->mPostPadding);
    }

    Val32 getRecordAllocSize() const {
      if (!mRecord)
        return NoVal32();
      return Val32(true, mRecord->mAllocSize);
    }

    std::pair<Val32, Val32> getFieldPreAndPostPadding(unsigned idx) const {
      if (!mRecord || idx >= mRecord->mFieldCount)
        return std::make_pair(NoVal32(), NoVal32());
      const File::Record::Field &field = mRecord->mFields[idx];
      return std::make_pair(Val32(true, field.mPrePadding), Val32(true, field.mPostPadding));
    }

    std::pair<Val32, Val32> getFieldOffsetAndStoreSize(unsigned idx) const {
      if (!mRecord || idx >= mRecord->mFieldCount)
        return std::make_pair(NoVal32(), NoVal32());
      const File::Record::Field &field = mRecord->mFields[idx];
      return std::make_pair(Val32(true, field.mOffset), Val32(true, field.mStoreSize));
    }

   private:
    Record32(const File::Record *Record) : mRecord(Record) { }
    const File::Record *mRecord;
  };
};

}

#endif  // _FRAMEWORKS_COMPILE_SLANG_REFLECTION_STATE_H_  NOLINT
