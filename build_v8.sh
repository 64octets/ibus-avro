export CC=`which clang`
export CXX=`which clang++`
export GYP_DEFINES="clang=1"
cd v8-3.22.14
make dependencies
make native -j 4 library=shared werror=no
cp out/native/lib.target/libv8.so ../libs/
