mkdir build
cd build
cmake .. 
make
cd ..

mkdir final
cp build/build/* final/
cp configs/* final/
