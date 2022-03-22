git clean -dxf build
pushd .
cd build
cmake .. -DBUILD_PYTHON=0 -DWITH_EXAMPLES=1
cmake --build . --config Release --target package
popd