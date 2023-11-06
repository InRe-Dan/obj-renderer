#! /bin/sh
# invoke with bash flamegraph.sh
timeout -s USR1 -k 5s 60s perf record -g './build/Renderer'
perf script > out.perf
perl ./stackcollapse-perf.pl out.perf > out.folded
perl ./flamegraph.pl out.folded > perf.svg
firefox perf.svg