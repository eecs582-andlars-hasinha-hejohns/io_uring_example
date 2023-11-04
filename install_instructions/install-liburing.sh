mkdir liburing
pushd liburing

git clone https://github.com/axboe/liburing.git
pushd liburing
./configure
make
sudo make install

popd
popd
