#!/bin/bash
./DocumentProcess -H 9003 -I localhost -P 9000 -C test -S scd-files/DOCUMENT.SCD -N 100 > doc_log &
