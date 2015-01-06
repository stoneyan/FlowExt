set -v
rm -f /cores/core.*
g++ -g -o parser lex.cpp grammar.cpp semantic.cpp semantic_test.cpp -stdlib=libstdc++
./parser > tmp.log

