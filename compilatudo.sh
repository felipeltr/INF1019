CURRDIR=$(pwd)
SHDIR=$(dirname $0)

cd $SHDIR

gcc -Wall -o dummy1 dummy1.c
gcc -Wall -o dummy2 dummy2.c
gcc -Wall -o dummy3 dummy3.c

cd sjf
gcc -Wall -o sjf sjf.c ../utils.c
cd ..

cd $CURRDIR