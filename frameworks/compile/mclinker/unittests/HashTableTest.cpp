//===- HashTableTest.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "HashTableTest.h"
#include "mcld/ADT/HashEntry.h"
#include "mcld/ADT/HashTable.h"
#include <cstdlib>

using namespace std;
using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
HashTableTest::HashTableTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
HashTableTest::~HashTableTest() {
}

// SetUp() will be called immediately before each test.
void HashTableTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void HashTableTest::TearDown() {
}

//==========================================================================//
// Testcases
//
struct IntCompare {
  bool operator()(int X, int Y) const { return (X == Y); }
};

struct PtrCompare {
  bool operator()(const int* X, const int* Y) const { return (X == Y); }
};

struct PtrHash {
  size_t operator()(const int* pKey) const {
    return (unsigned((uintptr_t)pKey) >> 4) ^ (unsigned((uintptr_t)pKey) >> 9);
  }
};

struct IntHash {
  size_t operator()(int pKey) const { return pKey; }
};

struct IntMod3Hash {
  size_t operator()(int pKey) const { return pKey % 3; }
};

TEST_F(HashTableTest, ptr_entry) {
  int A = 1;
  int* pA = &A;

  typedef HashEntry<int*, int, PtrCompare> HashEntryType;
  typedef HashTable<HashEntryType, PtrHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy(0);

  bool exist;
  hashTable->insert(pA, exist);

  EXPECT_FALSE(hashTable->empty());

  HashTableTy::iterator iter;
  iter = hashTable->find(NULL);
  EXPECT_TRUE(iter == hashTable->end());
  delete hashTable;
}

TEST_F(HashTableTest, constructor) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  HashTable<HashEntryType, IntHash, EntryFactory<HashEntryType> > hashTable(16);
  EXPECT_TRUE(17 == hashTable.numOfBuckets());
  EXPECT_TRUE(hashTable.empty());
  EXPECT_TRUE(0 == hashTable.numOfEntries());
}

TEST_F(HashTableTest, allocattion) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, IntHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy(22);

  bool exist;
  int key = 100;
  HashTableTy::entry_type* val = hashTable->insert(key, exist);
  val->setValue(999);
  EXPECT_FALSE(hashTable->empty());
  EXPECT_FALSE(exist);
  EXPECT_FALSE(NULL == val);
  HashTableTy::iterator entry = hashTable->find(key);
  EXPECT_EQ(999, entry.getEntry()->value());
  delete hashTable;
}

TEST_F(HashTableTest, alloc100) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, IntHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy(22);

  bool exist;
  HashTableTy::entry_type* entry = 0;
  for (int key = 0; key < 100; ++key) {
    entry = hashTable->insert(key, exist);
    EXPECT_FALSE(hashTable->empty());
    EXPECT_FALSE(exist);
    EXPECT_FALSE(NULL == entry);
    EXPECT_TRUE(key == entry->key());
    entry->setValue(key + 10);
  }

  EXPECT_FALSE(hashTable->empty());
  EXPECT_TRUE(100 == hashTable->numOfEntries());
  EXPECT_TRUE(197 == hashTable->numOfBuckets());
  delete hashTable;
}

TEST_F(HashTableTest, erase100) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, IntHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy(0);

  bool exist;
  for (unsigned int key = 0; key < 100; ++key)
    hashTable->insert(key, exist);

  EXPECT_FALSE(hashTable->empty());

  int count;
  HashTableTy::iterator iter;
  for (unsigned int key = 0; key < 100; ++key) {
    count = hashTable->erase(key);
    EXPECT_EQ(1, count);
    iter = hashTable->find(key);
    EXPECT_TRUE(iter == hashTable->end());
  }

  EXPECT_TRUE(hashTable->empty());
  delete hashTable;
}

TEST_F(HashTableTest, clear) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, IntHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy(22);

  bool exist;
  for (unsigned int key = 0; key < 100; ++key) {
    hashTable->insert(key, exist);
  }

  hashTable->clear();

  HashTableTy::iterator iter;
  for (unsigned int key = 0; key < 100; ++key) {
    iter = hashTable->find(key);
    EXPECT_TRUE(iter == hashTable->end());
  }

  EXPECT_TRUE(hashTable->empty());
  delete hashTable;
}

TEST_F(HashTableTest, tombstone) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, IntMod3Hash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy();

  bool exist;
  for (unsigned int key = 0; key < 100; ++key) {
    hashTable->insert(key, exist);
  }
  EXPECT_FALSE(hashTable->empty());

  int count;
  HashTableTy::iterator iter;
  for (unsigned int key = 0; key < 20; ++key) {
    count = hashTable->erase(key);
    EXPECT_EQ(1, count);
    iter = hashTable->find(key);
    EXPECT_TRUE(iter == hashTable->end());
  }
  EXPECT_TRUE(80 == hashTable->numOfEntries());

  for (unsigned int key = 20; key < 100; ++key) {
    iter = hashTable->find(key);
    EXPECT_TRUE(iter != hashTable->end());
  }

  for (unsigned int key = 0; key < 20; ++key) {
    hashTable->insert(key, exist);
  }
  EXPECT_TRUE(100 == hashTable->numOfEntries());
  EXPECT_TRUE(197 == hashTable->numOfBuckets());

  delete hashTable;
}

TEST_F(HashTableTest, rehash_test) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, IntHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy(0);

  bool exist;
  HashTableTy::entry_type* entry = 0;
  for (unsigned int key = 0; key < 400000; ++key) {
    entry = hashTable->insert(key, exist);
    entry->setValue(key + 10);
  }

  HashTableTy::iterator iter;
  for (int key = 0; key < 400000; ++key) {
    iter = hashTable->find(key);
    EXPECT_EQ((key + 10), iter.getEntry()->value());
  }

  delete hashTable;
}

TEST_F(HashTableTest, bucket_iterator) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, IntHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy(0);

  bool exist;
  HashTableTy::entry_type* entry = 0;
  for (unsigned int key = 0; key < 400000; ++key) {
    entry = hashTable->insert(key, exist);
    entry->setValue(key + 10);
  }

  HashTableTy::iterator iter, iEnd = hashTable->end();
  int counter = 0;
  for (iter = hashTable->begin(); iter != iEnd; ++iter) {
    EXPECT_EQ(iter.getEntry()->key() + 10, iter.getEntry()->value());
    ++counter;
  }
  EXPECT_EQ(400000, counter);
  delete hashTable;
}

TEST_F(HashTableTest, chain_iterator_single) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, IntHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy();

  bool exist;
  HashTableTy::entry_type* entry = 0;
  for (int key = 0; key < 16; ++key) {
    entry = hashTable->insert(key * 37, exist);
    entry->setValue(key + 10);
  }
  for (int key = 0; key < 16; ++key) {
    int counter = 0;
    HashTableTy::chain_iterator iter, iEnd = hashTable->end(key * 37);
    for (iter = hashTable->begin(key * 37); iter != iEnd; ++iter) {
      EXPECT_EQ(key + 10, iter.getEntry()->value());
      ++counter;
    }
    EXPECT_EQ(1, counter);
  }
  delete hashTable;
}

struct FixHash {
  size_t operator()(int pKey) const { return 10; }
};

TEST_F(HashTableTest, chain_iterator_list) {
  typedef HashEntry<int, int, IntCompare> HashEntryType;
  typedef HashTable<HashEntryType, FixHash, EntryFactory<HashEntryType> >
      HashTableTy;
  HashTableTy* hashTable = new HashTableTy();

  bool exist;
  HashTableTy::entry_type* entry = 0;
  for (unsigned int key = 0; key < 16; ++key) {
    entry = hashTable->insert(key, exist);
    ASSERT_FALSE(exist);
    entry->setValue(key);
  }
  ASSERT_TRUE(16 == hashTable->numOfEntries());
  ASSERT_TRUE(37 == hashTable->numOfBuckets());

  unsigned int key = 0;
  int count = 0;
  HashTableTy::chain_iterator iter, iEnd = hashTable->end(key);
  for (iter = hashTable->begin(key); iter != iEnd; ++iter) {
    count++;
  }
  ASSERT_EQ(16, count);
  delete hashTable;
}
