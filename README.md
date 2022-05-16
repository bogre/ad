# ad
automatic derivation excercises

# build
git clone https://github.com/bogre/ad.git
cd ad

for linux execute commands:
cmake -B build/out -S .
make -C ./build/out

for windows:
cmake -B build/out -S . -G "Visual Studio 17 2022" -A x64
# run
binaries are placed in subdirectory ./build/out/bin
run executable like
./build/out/bin/srk
or
./build/out/bin/srk test

![results for task 1 and 2](./rc/task1and2.png?raw=true "run on linux")
