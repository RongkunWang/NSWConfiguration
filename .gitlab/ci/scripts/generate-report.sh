#!/bin/sh

usage() {
    cat <<EOF
Usage: $0 <filename>

Parse the output of a compilation into a JSON file compatible with
GitLab CI Code Quality reporting back end (CodeClimate).
EOF

    exit 1
}

if [ ! -f $1 ] || [ ! -n "$1" ]
then
    echo "Invalid file specified"
    usage
fi

inputfile=$1

if [ ! -n "${REPORTS_PATH}" ]
then
    echo "REPORTS_PATH not set, setting to 'reports'"
    REPORTS_PATH=reports
    mkdir -p ${REPORTS_PATH}
fi

outputfile=${inputfile%*.txt}.json
if [ ! -n "${outputfile}" ]
then
    outputfile=${inputfile}.json
fi

set -x

perl -pi -e 's|\\.|\\\\.|g' ${inputfile}
perl -pi -e "s|${PWD}/ci-build/${CI_PROJECT_NAME}/||g" ${inputfile}
cat ${inputfile} | \
    grep -Ev '(clang-diagnostic-error|note:|/extern/)'| \
    grep -E '(error|note|warning):' | \
    sed -E 's/^([^:]+):([^:]+):([^:]+): ([^:]+): (.*) (.[-.a-zA-Z0-9]+.)$/{\n\t"description":"\5",\n\t"severity":"\4",\n\t"fingerprint":"hash",\n\t"location": {\n\t\t"path":"\1",\n\t\t"lines": {\n\t\t\t"begin":\2\n\t\t}\n\t},\n\t"type":"\4",\n\t"check_name": "\6"\t\n},/' \
        > ${outputfile}
