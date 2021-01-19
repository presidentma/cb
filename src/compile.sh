#!/bin/bash
gcc -g main.c include/cJSON.c cb.c cb_store.c hashset.c cb_curses.c cb_utils.c -o cb -lncursesw && ./cb -l
