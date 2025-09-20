#!/bin/bash

./tzc "$1" -o "$1.c"
zig cc -Os "$1.c"
