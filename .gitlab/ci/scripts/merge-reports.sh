#!/bin/sh

if [ ! -n "${REPORTS_PATH}" ]
then
    echo "REPORTS_PATH not set, setting to 'reports'"
    REPORTS_PATH=reports
fi

## gl-code-quality-report.json
outputfile=${REPORTS_PATH}/gl-code-quality-report.json

cat ${REPORTS_PATH}/*.json > ${outputfile}

## insert braces
echo ']' >> ${outputfile}
sed -i '0,/^/s//\[\n/' ${outputfile}

## remove trailing ','
perl -0777 -pi -e 's/,(\s*\n?\s*[}\]])/\1/g' ${outputfile}
