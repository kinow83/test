// Copyright (c) 2007, Google Inc.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// ---
//
// This tests mostly that we can #include the files correctly
// and have them work.  This unittest purposefully does not
// #include <config.h>; it's meant to emulate what a 'regular
// install' of sparsehash would be able to see.

#include <sparsehash/internal/sparseconfig.h>
#include <config.h>
#include <stdio.h>
#include <sparsehash/sparse_hash_set>
#include <sparsehash/sparse_hash_map>
#include <sparsehash/dense_hash_set>
#include <sparsehash/dense_hash_map>
#include <sparsehash/template_util.h>
#include <sparsehash/type_traits.h>

#include "sampling.h"

using std::string;


#define CHECK_IFF(cond, when) do {                                      \
  if (when) {                                                           \
    if (!(cond)) {                                                      \
      puts("ERROR: " #cond " failed when " #when " is true\n");         \
      exit(1);                                                          \
    }                                                                   \
  } else {                                                              \
    if (cond) {                                                         \
      puts("ERROR: " #cond " succeeded when " #when " is false\n");     \
      exit(1);                                                          \
    }                                                                   \
  }                                                                     \
} while (0)

struct eqstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return //(s1 == s2) || 
			(s1 && s2 && strcmp(s1, s2) == 0);
	}
};

int main(int argc, char** argv) {
  // Run with an argument to get verbose output
  const bool verbose = argc > 1;
  size_t i, num = 0;
  struct word *w = NULL;
  struct word *c = NULL;
  struct word *f = NULL;
  clock_t start;
  string s;
  google::sparse_hash_set<string> sset;
  google::dense_hash_set <string> dset;
  google::sparse_hash_map<string, string> smap;
  google::dense_hash_map <string, string> dmap;

  num = load_word(argc, argv, 20, 20, &w);
  num = load_word(argc, argv, 20, 20, &f);
  printf("num = %ld\n", num);

  // SET
  {
	  printf("google sset\n");
	  start = clock();
	  for (c=w; c; c=c->next) {
		  sset.insert(string(c->text));
	  }
	  printf("insert complete: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	  start = clock();
	  i = 0;
	  for (c=f; c; c=c->next) {
		  assert (sset.find(string(c->text)) != sset.end());
	  }
	  printf("sset: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

  } printf("\n"); {
	  // MAP
	  printf("google smap\n");
	  start = clock();
	  for (c=w; c; c=c->next) {
		  smap[string(c->text)] = string(c->text);
	  }
	  printf("insert complete: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	  start = clock();
	  i = 0;
	  for (c=f; c; c=c->next) {
		s = string(c->text);
		  assert (smap[s] == s);
	  }
	  printf("smap: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
  } printf("\n"); {
	  printf("google dset\n");
	  start = clock();
	  for (c=w; c; c=c->next) {
		  dset.insert(string(c->text));
	  }
	  printf("insert complete: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	  start = clock();
	  i = 0;
	  for (c=f; c; c=c->next) {
		  assert (dset.find(string(c->text)) != dset.end());
	  }
	  printf("dset: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

  } printf("\n"); {
	  // MAP
	  printf("google dmap\n");
	  start = clock();
	  for (c=w; c; c=c->next) {
		  dmap[string(c->text)] = string(c->text);
	  }
	  printf("insert complete: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

	  start = clock();
	  i = 0;
	  for (c=f; c; c=c->next) {
		s = string(c->text);
		  assert (dmap[s] == s);
	  }
	  printf("dmap: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
  }


#if 0
  {
    for (google::sparse_hash_set<int>::const_iterator it = sset.begin();
         it != sset.end(); ++it)
      printf("sset: %d\n", *it);
    for (google::sparse_hash_map<int,int>::const_iterator it = smap.begin();
         it != smap.end(); ++it)
      printf("smap: %d -> %d\n", it->first, it->second);
    for (google::dense_hash_set<int>::const_iterator it = dset.begin();
         it != dset.end(); ++it)
      printf("dset: %d\n", *it);
    for (google::dense_hash_map<int,int>::const_iterator it = dmap.begin();
         it != dmap.end(); ++it)
      printf("dmap: %d -> %d\n", it->first, it->second);
  }
#endif

#if 0
  for (int i = 0; i < 100; i++) {
    CHECK_IFF(sset.find(i) != sset.end(), (i % 10) == 0);
    CHECK_IFF(smap.find(i) != smap.end(), (i % 10) == 0);
    CHECK_IFF(smap.find(i) != smap.end() && smap.find(i)->second == i+1,
              (i % 10) == 0);
    CHECK_IFF(dset.find(i) != dset.end(), (i % 10) == 5);
    CHECK_IFF(dmap.find(i) != dmap.end(), (i % 10) == 5);
    CHECK_IFF(dmap.find(i) != dmap.end() && dmap.find(i)->second == i+1,
              (i % 10) == 5);
  }
  printf("PASS\n");
#endif
  return 0;
}
