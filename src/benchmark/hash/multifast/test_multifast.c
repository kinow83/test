/*
 * example0.c It explains how to use the _settext/_findnext function pair of
 * the ahocorasick library
 * 
 * This file is part of multifast.
 *
    Copyright 2010-2015 Kamiar Kanani <kamiar.kanani@gmail.com>

    multifast is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    multifast is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with multifast.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "ahocorasick.h"
#include "libsample.h"

void print_match (AC_MATCH_t *m);

int main (int argc, char **argv)
{
    unsigned int i;
    AC_TRIE_t *trie;
    AC_PATTERN_t patt;
    AC_TEXT_t chunk;
    AC_MATCH_t match;

	size_t num = 0;
	struct word *w = NULL;
	struct word *c = NULL;
	struct word *f = NULL;
	clock_t start;

	num = load_word(argc, argv, 20, 20, &w);
	num = load_word(argc, argv, 20, 20, &f);
	printf("num = %ld\n", num);
    

    /* Get a new trie */
    trie = ac_trie_create ();

	start = clock();
	printf("multifase 2.0\n");
	for (c=w,i=0; c; c=c->next,i++) {
        /* Fill the pattern data */
        patt.ptext.astring = c->text;
        patt.ptext.length = strlen(patt.ptext.astring);

       /* The replacement pattern is not applicable in this program, so better 
         * to initialize it with 0 */
        patt.rtext.astring = NULL;
        patt.rtext.length = 0;
        
        /* Pattern identifier is optional */
        patt.id.u.number = i + 1;
        patt.id.type = AC_PATTID_TYPE_NUMBER;
        
        /* Add pattern to automata */
        ac_trie_add (trie, &patt, 0);
		
		/* We added pattern with copy option disabled. It means that the 
         * pattern memory must remain valid inside our program until the end of 
         * search. If you are using a temporary buffer for patterns then you 
         * may want to make a copy of it so you can use it later. */
	}
    /* Now the preprocessing stage ends. You must finalize the trie. Remember 
     * that you can not add patterns anymore. */
    ac_trie_finalize (trie);
	printf("multifase 2.0 insert: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

    /* Finalizing the trie is the slowest part of the task. It may take a 
     * longer time for a very large number of patters */
    
    /* Display the trie if you wish */
    // ac_trie_display (trie);
    
	start = clock();
	for (c=f; c; c=c->next) {
		chunk.astring = c->text;
		chunk.length = strlen (chunk.astring);
		/* Set the input text */
		ac_trie_settext (trie, &chunk, 0);
		/* Find matches */
		/*
		while ((match = ac_trie_findnext(trie)).size) {
			print_match (&match);
		}*/
		assert ((match = ac_trie_findnext(trie)).size);
	}
	printf("multifase 2.0 find: %0.5f\n", (float)(clock() - start) / CLOCKS_PER_SEC);


	printf("\n");

    /* You may release the automata after you have done with it. */
    ac_trie_release (trie);
    
    return 0;
}

void print_match (AC_MATCH_t *m)
{
    unsigned int j;
    AC_PATTERN_t *pp;
    
    printf ("@%2lu found: ", m->position);
    
    for (j = 0; j < m->size; j++)
    {
        pp = &m->patterns[j];
        
        printf("#%ld \"%.*s\", ", pp->id.u.number,
            (int)pp->ptext.length, pp->ptext.astring);
        
        /* CAUTION: the AC_PATTERN_t::ptext.astring pointers, point to the 
         * sample patters in our program, since we added patterns with copy 
         * option disabled.
         */        
    }
    
    printf ("\n");
}
