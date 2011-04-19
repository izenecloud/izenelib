// This file is part of babudb/cpp
//
// Copyright (c) 2008, Felix Hupfeld, Jan Stender, Bjoern Kolbeck, Mikael Hoegqvist, Zuse Institute Berlin.
// Copyright (c) 2009, 2010 Felix Hupfeld
// Licensed under the BSD License, see LICENSE file for details.
//
// Author: Felix Hupfeld (felix@storagebox.org)

#include <utility>
using std::pair;

#include "babudb/profiles/string_key.h"
#include "yield/platform/memory_mapped_file.h"
#include "index/index_writer.h"
#include "index/index.h"
#include "index/merger.h"
#include "log_index.h"
using namespace YIELD;
using namespace babudb;

#include "babudb/test.h"

TEST_TMPDIR(ImmutableIndexWriter,babudb)
{
  // create ImmutableIndex from LogIndex
  StringOrder sorder;
  IndexMerger* merger = new IndexMerger(testPath("testdb-testidx"), sorder);

  merger->Add(1, DataHolder("key1"), DataHolder("data1"));
  merger->Add(2, DataHolder("key2"), DataHolder("data2"));
  merger->Add(3, DataHolder("key3"), DataHolder("data3"));
  merger->Add(4, DataHolder("key4"), DataHolder("data4"));
	
  merger->Run();
  delete merger;

	// load it and check
  ImmutableIndex::DiskIndices indices = ImmutableIndex::FindIndices(
      testPath("testdb-testidx"));
  ImmutableIndex* loadedindex = ImmutableIndex::LoadLatestIntactIndex(indices, sorder);
  EXPECT_EQUAL(loadedindex->GetLastLSN(), 4);
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key1")).isEmpty());
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key2")).isEmpty());
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key3")).isEmpty());
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key4")).isEmpty());

	// create another LogIndex
  merger = new IndexMerger(testPath("testdb-testidx"), sorder, loadedindex);

  merger->Add(5, DataHolder("key12"), DataHolder("data12"));
  merger->Add(6, DataHolder("key22"), DataHolder("data22"));
  merger->Add(7, DataHolder("key32"), DataHolder("data32"));
  merger->Add(8, DataHolder("key42"), DataHolder("data42"));
	
  merger->Run();
  delete merger;
  delete loadedindex;

  // load it and check
  indices = ImmutableIndex::FindIndices(testPath("testdb-testidx"));
  loadedindex = ImmutableIndex::LoadLatestIntactIndex(indices, sorder);
  EXPECT_EQUAL(loadedindex->GetLastLSN(), 8);
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key1")).isEmpty());
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key2")).isEmpty());
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key3")).isEmpty());

  EXPECT_TRUE(loadedindex->Lookup(DataHolder("key123")).isEmpty());

  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key12")).isEmpty());
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key22")).isEmpty());
  EXPECT_TRUE(!loadedindex->Lookup(DataHolder("key32")).isEmpty());
}

TEST_TMPDIR(ImmutableIndexWriterEmpty,babudb)
{
  // Create an empty index
  ImmutableIndexWriter* writer = ImmutableIndex::Create(
      testPath("testdb-empty"), 9, 64*1024);
  writer->Finalize();
  delete writer;
  
  // And load it again
  StringOrder sorder;
  ImmutableIndex::DiskIndices indices = ImmutableIndex::FindIndices(
      testPath("testdb-empty"));
  ImmutableIndex* loadedindex = ImmutableIndex::LoadLatestIntactIndex(indices, sorder);
  EXPECT_EQUAL(loadedindex->GetLastLSN(), 9);
  EXPECT_TRUE(loadedindex->Lookup(DataHolder("key123")).isEmpty());
}
