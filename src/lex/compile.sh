#!/bin/bash
lex simple.l;  gcc lex.yy.c -o simple -ll
lex number.l;  gcc lex.yy.c -o number -ll
lex parser.l;  gcc lex.yy.c -o parser -ll
