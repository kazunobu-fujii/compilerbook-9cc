##!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 '5+20-4;'
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 5 "-5+10;"
try 0 "-(3+5)+8;"
try 0 "-(2*5+1)+11;"
try 0 "-(1+6/3*2)+5;"
try 1 "1==1;"
try 1 "-1==-1;"
try 0 "1==0;"
try 1 "1!=-1;"
try 0 "1!=1;"
try 1 "-1<=-1;"
try 1 "-1<=0;"
try 0 "1<=-1;"
try 1 "1>=1;"
try 1 "1>=-1;"
try 0 "-1>=0;"
try 1 "-1<1;"
try 0 "-1<-1;"
try 0 "1<-1;"
try 1 "1>-1;"
try 0 "1>1;"
try 0 "-1>1;"
try 0 "-3*2+6*(1==1);"
try 3 "3*(4==2*2);"
try 1 "3*2==6*1;"
try 0 "3*(2==6)*1;"
try 14 "a = 3;b = 5 * 6 - 8;a + b / 2;"

echo OK
