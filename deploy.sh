if [ ! -d build ]; then
  mkdir build
fi

cd build
cmake .. && make -j8
./asio_server 13