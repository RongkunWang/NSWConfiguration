rm -Rf build
mkdir build
cd build
git clone https://github.com/quasar-team/open62541-compat.git -b pmaster 
cd open62541-compat
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DSTANDALONE_BUILD=ON -DOPEN62541-COMPAT_BUILD_CONFIG_FILE=boost_standard_install_cc7.cmake -DSKIP_TESTS=ON ../
make
echo "open62541-compat finished. Not sure if it was successful ;-) "
cd ../../
cmake -DCMAKE_BUILD_TYPE=Debug ../ 
make




