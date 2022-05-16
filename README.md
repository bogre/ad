# ad
automatic derivation excercises\
I tried ad with autodiff lib\
First of all, focus was on compile time calculations of alpha values\
since, as far as I reached autodiff doesn't support ct calculation.\
For example, we do know all needed data in the time of compilation,\
so alpha values fot T=300 K can and should be calculated during compilation,\
that way we have prepared values for run time use.\
Compile time cache is implemented using compile time populated std::array with
size of No compile time constant, as numer of ch-elements. Level of derivations is cantained in Order
constant\
For derivations, calculations can be calculated in compile time as well, but not\
with autodiff, if I didnt miss something, so I made runtime derivation calculations for T = 300K.\
RunTime cache has been created using ordinary stl hash map, but in real\
use, of course, some more cache friendly datastructure, should be used,\
anyway, the idea and way of thinkig is what is important here.\
Regarding autodiff, I used reverse approach with autodiffs var type, but I tried
forward dual type as well, only for first derivate, but I had to make some
template gymnastic to make it work in compile time using the same alphaelem function, but I resolved that\
using alpha_elemptr explicit instantiation(now not used, but commented out, and present)\
to specify overload for autodiff to choose for runtime calculations.\
Main issue for me here was to acomplish compile time calculations, in order to\
finish that, I used gcem as 3pty lib for compiletime mathematics, as an example.\
Regarding tests, I made simple test, w/o using any particular framework, but
I can do that if needed with my prefered catch2 with cmake integration,for example, or any other.\
C++20 standard is used and code compiled with gcc12 with -fno-bultins as well as with Visual studio 2022.\
Compilation has been preformed with asan and ubsan and after runing it, experienced no issues.\
As a solution, alpha function has signature:\
\
void alpha(decimal_type T, std::span\<const decimal_type> Tc,\
           std::span\<const decimal_type> omega, const std::size_t H,\
           ResultDataSet result)\
where is\
using ResultSet = std::array<decimal_type, No>\
using ResultDataSet = std::span\<ResultSet>\
\
Clang-tidy is used as a static analizer, and it is integrated with cmake as\
a target for static analysis, and one can run it with\
make -C ./build/out check (for linux, of course)\
Tidy complains with "no str() member in iostream", but source construction should\
work with c++20. All other warnings and complains are comming from 3pty.\
\
Finally, you can find screenshot below, with result of executed srk binary for\
task 1 and 2.\
Task3 is still work in progres, and it will be pushed, soon.\
# build
git clone https://github.com/bogre/ad.git\
cd ad

for linux execute commands:\
cmake -B build/out -S .\
make -C ./build/out

for windows:\
cmake -B build/out -S . -G "Visual Studio 17 2022" -A x64
# run
binaries are placed in subdirectory ./build/out/bin\
run executable like\
./build/out/bin/srk\
or\
./build/out/bin/srk test

![results for task 1 and 2](./rc/task1and2.png?raw=true "run on linux")
