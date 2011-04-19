// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include <utility>
using std::pair;

#include "babudb/profiles/string_key.h"
#include "index_writer.h"
#include "index.h"
#include "yield/platform/memory_mapped_file.h"
using namespace YIELD;
using namespace babudb;

#include "babudb/test.h"

TEST_TMPDIR(ImmutableIndex,babudb)
{
  ImmutableIndexWriter* writer = ImmutableIndex::Create(testPath("testdb-testidx"), 4, 64 * 1024);
  DataHolder k1("key1"), v1("value1"), k2("key2"), v2("value2");
  DataHolder k3("key3"), v3("value3"), k4("key4"), v4("value4");
  writer->Add(k1,v1);
  writer->Add(k2,v2);
  writer->Add(k3,v3);
  writer->Add(k4,v4);
  writer->Finalize();
  delete writer;

  StringOrder myorder;
  ImmutableIndex::DiskIndices indices = ImmutableIndex::FindIndices(
      testPath("testdb-testidx"));
  ImmutableIndex* index = ImmutableIndex::LoadLatestIntactIndex(
      indices, myorder);

  EXPECT_TRUE(index->Find(Buffer(k1)) != index->end());
  EXPECT_TRUE(index->Find(Buffer(k3)) != index->end());
  EXPECT_TRUE(index->Find(Buffer(k2)) != index->end());
  EXPECT_TRUE(index->Find(Buffer(k4)) != index->end());

  DataHolder prefix("ke");
  EXPECT_TRUE(index->Find(Buffer(prefix)) != index->end());

  // search keys directly
  EXPECT_TRUE(!index->Lookup(k1).isEmpty());
  EXPECT_TRUE(!index->Lookup(k2).isEmpty());
  EXPECT_TRUE(index->Lookup(k2) == v2);
  EXPECT_TRUE(!index->Lookup(k3).isEmpty());
  EXPECT_TRUE(!index->Lookup(k4).isEmpty());

  // not founds
  DataHolder before("a");
  EXPECT_TRUE(index->Lookup(before).isEmpty());
  DataHolder middle("key21");
  EXPECT_TRUE(index->Lookup(middle).isEmpty());
  DataHolder after("x");
  EXPECT_TRUE(index->Lookup(after).isEmpty());


  // and iteration
  ImmutableIndex::iterator i = index->begin();
  EXPECT_TRUE(!(*i).first.isEmpty());
  EXPECT_TRUE(i != index->end());

  ++i;
  EXPECT_TRUE(!(*i).first.isEmpty());
  EXPECT_TRUE(i != index->end());

  ++i;
  EXPECT_TRUE(!(*i).first.isEmpty());
  EXPECT_TRUE(i != index->end());

  ++i;
  EXPECT_TRUE(!(*i).first.isEmpty());
  EXPECT_TRUE(i != index->end());

  ++i;
  EXPECT_TRUE(i == index->end());

  // matches
  ImmutableIndex::iterator pi = index->Find(prefix);
  EXPECT_TRUE(pi != index->end());
  EXPECT_TRUE((*pi).first == "key1");
  EXPECT_TRUE(pi != index->end());
  EXPECT_TRUE(index->Find(DataHolder("kez")) == index->end() );
}
