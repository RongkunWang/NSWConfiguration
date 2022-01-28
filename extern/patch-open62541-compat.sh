#!/usr/bin/env bash

echo "Patching open62541-compat LogIt.cmake to clone from CERN GitLab"

git checkout -- LogIt.cmake

git apply --verbose --ignore-space-change --ignore-whitespace \
    <<'EOF'
diff --git i/LogIt.cmake w/LogIt.cmake
index 6a9387c..f434168 100644
--- i/LogIt.cmake
+++ w/LogIt.cmake
@@ -24,12 +24,12 @@

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

echo "Patching open62541-compat include/uadatavalue.h to enable clang compilation"

git checkout -- include/uadatavalue.h

git apply --verbose --ignore-space-change --ignore-whitespace \
    <<'EOF'
diff --git i/include/uadatavalue.h w/include/uadatavalue.h
index a4ccd2b..01ce57e 100644
--- i/include/uadatavalue.h
+++ w/include/uadatavalue.h
@@ -15,7 +15,7 @@
 #define GCC_VERSION (__GNUC__ * 10000 \
                                + __GNUC_MINOR__ * 100 \
                                + __GNUC_PATCHLEVEL__)
-#if GCC_VERSION > 40800
+#if GCC_VERSION > 40800 || defined(__clang__)
 #include <atomic>
 #else // GCC_VERSION
 #include <stdatomic.h>
EOF
