#!/usr/bin/env bash

echo "Patching open62541-compat LogIt.cmake to clone from CERN GitLab"

git checkout -- LogIt.cmake

# patch -p1 \
git apply --ignore-space-change --ignore-whitespace \
    <(cat<<"EOF"
diff --git i/LogIt.cmake w/LogIt.cmake
index 6a9387c..f434168 100644
--- i/LogIt.cmake
+++ w/LogIt.cmake
@@ -25,11 +25,11 @@
 function ( fetch_LogIt )
   SET (LOGIT_VERSION "v0.1.1") #change to master post-merge
-  message(STATUS "fetching LogIt from github. *NOTE* fetching version [${LOGIT_VERSION}]")
+  message(STATUS "fetching LogIt from CERN GitLab. *NOTE* fetching version [${LOGIT_VERSION}]")
   Open62541CompatFetchContent_Declare(
     LogIt
-    GIT_REPOSITORY    https://github.com/quasar-team/LogIt.git
+    GIT_REPOSITORY    ${GIT_URL_ENDPOINT}/quasar-team/LogIt.git
     GIT_TAG           ${LOGIT_VERSION}
     GIT_SHALLOW       "1"
     SOURCE_DIR	      ${PROJECT_BINARY_DIR}/LogIt
     BINARY_DIR	      ${PROJECT_BINARY_DIR}/LogIt
   )
EOF
)
