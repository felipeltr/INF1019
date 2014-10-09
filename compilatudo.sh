CURRDIR=$(pwd)
SHDIR=$(dirname $0)

cd $SHDIR

cd dummies

for file in *.c
do
	bin="${file%.*}"
	echo "Building $bin..."
	gcc -Wall -o $bin $file
done

cd ../sjf
gcc -Wall -o sjf sjf.c ../utils.c
cd ../roundrobin
gcc -Wall -o roundrobin roundrobin.c ../utils.c

cd $CURRDIR