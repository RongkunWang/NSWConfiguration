#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <module-name>"
    exit 64
fi

LOWERCASE_NAME=test_$(echo $1 | tr '[:upper:]' '[:lower:]')

if [ -f "${LOWERCASE_NAME}.cpp" ]; then
    echo "${LOWERCASE_NAME}.cpp already exists, exiting..."
    exit 1
fi

cat <<EOF >> ${LOWERCASE_NAME}.cpp
#define BOOST_TEST_MODULE ${1}_tests
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
EOF

echo "Test module created in ${LOWERCASE_NAME}.cpp!"