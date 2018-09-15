#!/usr/bin/bash

if [ ! $# -eq 2 ]
then
  echo "Usage: $0 [CSV FILE] [PLOT TYPE]"
  exit 0
fi

if [ ! -f $1 ]
then
  echo "不正なファイル名 $1"
  exit 0
fi

FILE_NAME=`date +%Y-%m-%dT%H:%M:%S`"_plot.png"

echo "set terminal png size 2400,1800; \
  set output '$FILE_NAME'; \
  set grid; \
  set xlabel 'θ[rad]'; \
  set ylabel 'ρ'; \
  plot '$1' with $2
  " | gnuplot
