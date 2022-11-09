#!/usr/bin/env bash

echo "Patching UaoClientForOpcUaSca to use find_package variables for protobuf"

git checkout -- CMakeLists.txt

git apply --verbose --ignore-space-change --ignore-whitespace \
    <<'EOF'
diff --git i/CMakeLists.txt w/CMakeLists.txt
index 84444d1..0d83d7e 100644
--- i/CMakeLists.txt
+++ w/CMakeLists.txt
@@ -11,9 +11,12 @@ if (BUILD_STANDALONE)
     include( ${BUILD_CONFIG} )
   endif(BUILD_CONFIG)
   include_directories( include )
-  include_directories( ${LOGIT_INCLUDE_DIR} )
-  include_directories( $ENV{PROTOBUF_HEADERS} )
-  add_library(UaoClientForOpcUaSca STATIC ${SOURCES} $ENV{PROTOBUF_DIRECTORIES}/libprotobuf.a  )
+  string(REPLACE ".so-lpthread" ".a" Protobuf_STATIC_LIBRARIES ${Protobuf_LIBRARIES} )
+  add_library(UaoClientForOpcUaSca STATIC ${SOURCES} )
+  target_include_directories(UaoClientForOpcUaSca SYSTEM PRIVATE ${Protobuf_INCLUDE_DIRS} )
+  target_include_directories(UaoClientForOpcUaSca SYSTEM PRIVATE ${LOGIT_INCLUDE_DIR} )
+  target_compile_options(UaoClientForOpcUaSca PRIVATE -fvisibility=hidden )
+  target_link_libraries(UaoClientForOpcUaSca PRIVATE ${Protobuf_LIBRARIES} )
   install(TARGETS UaoClientForOpcUaSca DESTINATION lib)
   install(DIRECTORY include/ DESTINATION include)
 
EOF

echo "Patching UaoClientForOpcUaSca/include/BitBangProtocol.pb.h to use find_package variables for protobuf"

git checkout -- include/BitBangProtocol.pb.h

git apply --verbose --ignore-space-change --ignore-whitespace \
    <<'EOF'
diff --git i/include/BitBangProtocol.pb.h w/include/BitBangProtocol.pb.h
index d85fe8a..b6b46dd 100644
--- i/include/BitBangProtocol.pb.h
+++ w/include/BitBangProtocol.pb.h
@@ -27,6 +27,8 @@
 #include <google/protobuf/unknown_field_set.h>
 // @@protoc_insertion_point(includes)

+#include <BitBangRequests.h>
+
 namespace ScaPb {

 // Internal implementation detail -- do not call these.
@@ -57,10 +59,6 @@ inline bool BitBangRequest_RequestType_Parse(
   return ::google::protobuf::internal::ParseNamedEnum<BitBangRequest_RequestType>(
     BitBangRequest_RequestType_descriptor(), name, value);
 }
-enum BitBangRequest_Direction {
-  BitBangRequest_Direction_INPUT = 0,
-  BitBangRequest_Direction_OUTPUT = 1
-};
 bool BitBangRequest_Direction_IsValid(int value);
 const BitBangRequest_Direction BitBangRequest_Direction_Direction_MIN = BitBangRequest_Direction_INPUT;
 const BitBangRequest_Direction BitBangRequest_Direction_Direction_MAX = BitBangRequest_Direction_OUTPUT;
EOF

echo "Patching UaoClientForOpcUaSca/include/IoBatch.h to use find_package variables for protobuf"

git checkout -- include/IoBatch.h

git apply --verbose --ignore-space-change --ignore-whitespace \
    <<'EOF'
diff --git i/include/IoBatch.h w/include/IoBatch.h
index 20ee5e6..1c84981 100644
--- i/include/IoBatch.h
+++ w/include/IoBatch.h
@@ -8,9 +8,15 @@
 #pragma once
 
 #include <uaclient/uaclientsdk.h>
-#include <BitBangProtocol.pb.h>
+#include <BitBangRequests.h>
+#include <map>
 #include <memory>
 
+namespace ScaPb {
+class BitBangRequest;
+class BitBangRequests;
+}
+
 namespace UaoClientForOpcUaSca
 {
 
@@ -34,11 +40,11 @@ public:
     void addSetPinsDirections( std::map<uint32_t, direction> pinDirection, uint32_t delay = 0 );
     std::vector<OpcUa_UInt32> dispatch();
 
-    int getCurrentBatchSize() { return m_group.requests_size(); }
-    void clearBatch() { m_group.Clear(); }
+    int getCurrentBatchSize();
+    void clearBatch();
 
 private:
-    ScaPb::BitBangRequests m_group;
+    std::unique_ptr<ScaPb::BitBangRequests> m_group;
     UaSession  * m_session;
     UaNodeId     m_objId;
 
EOF

echo "Patching UaoClientForOpcUaSca/src/IoBatch.cpp to use find_package variables for protobuf"

git checkout -- src/IoBatch.cpp

git apply --verbose --ignore-space-change --ignore-whitespace \
    <<'EOF'
diff --git i/src/IoBatch.cpp w/src/IoBatch.cpp
index 9353ac9..2ad67bf 100644
--- i/src/IoBatch.cpp
+++ w/src/IoBatch.cpp
@@ -10,6 +10,7 @@
 #include <stdexcept>
 #include <UaoClientForOpcUaScaArrayTools.h>
 #include <UaoClientForOpcUaScaUaoExceptions.h>
+#include <BitBangProtocol.pb.h>
 #include <GpioBitBanger.h>
 #include <iostream>
 #include <LogIt.h>
@@ -23,6 +24,7 @@ IoBatch::direction const IoBatch::OUTPUT = ScaPb::BitBangRequest_Direction_OUTPU
 IoBatch::direction const IoBatch::INPUT = ScaPb::BitBangRequest_Direction_INPUT;
 
 IoBatch::IoBatch( UaSession* session, UaNodeId objId ) :
+    m_group{std::make_unique<ScaPb::BitBangRequests>()},
     m_session(session),
     m_objId (objId)
 {
@@ -42,7 +44,7 @@ void IoBatch::addSetPins( std::map<uint32_t, bool> pinValue, uint32_t delay )
     if ( pinValue.empty() )
         throw std::runtime_error("No pins and no values were passed");
 
-    ScaPb::BitBangRequest* request = m_group.add_requests();
+    ScaPb::BitBangRequest* request = m_group->add_requests();
 
 	request->set_type( ScaPb::BitBangRequest_RequestType_WRITE );
 
@@ -65,7 +67,7 @@ void IoBatch::addGetPins( uint32_t delay )
 
     LOG(Log::DBG) << "Adding a get-pins request";
 
-    ScaPb::BitBangRequest* request = m_group.add_requests();
+    ScaPb::BitBangRequest* request = m_group->add_requests();
 
 	request->set_type( ScaPb::BitBangRequest_RequestType_READ );
 
@@ -81,7 +83,7 @@ void IoBatch::addSetPinsDirections( std::map<uint32_t, direction> pinDirection,
     if ( pinDirection.empty() )
         throw std::runtime_error("No pins and no directions were passed");
 
-    ScaPb::BitBangRequest* request = m_group.add_requests();
+    ScaPb::BitBangRequest* request = m_group->add_requests();
 
 	request->set_type( ScaPb::BitBangRequest_RequestType_SET_DIRECTION );
 
@@ -104,14 +106,14 @@ std::vector<OpcUa_UInt32> IoBatch::dispatch()
 
     LOG(Log::DBG) << "Dispatching batch of GPIO requests";
 
-    LOG(Log::DBG) << "Bytes to dispatch: " << m_group.ByteSize();
+    LOG(Log::DBG) << "Bytes to dispatch: " << m_group->ByteSize();
 
     UaByteString requestsMessageSerialized;
 
-    size_t size = m_group.ByteSize();
+    size_t size = m_group->ByteSize();
     std::unique_ptr<unsigned char[]> serialized(new unsigned char[size]);
 
-    m_group.SerializeToArray( &serialized[0], static_cast<int>(size) );
+    m_group->SerializeToArray( &serialized[0], static_cast<int>(size) );
     
     requestsMessageSerialized.setByteString( static_cast<int>(size), &serialized[0] );
 
@@ -127,6 +129,16 @@ std::vector<OpcUa_UInt32> IoBatch::dispatch()
 
 }
 
+int IoBatch::getCurrentBatchSize()
+{
+   return m_group->requests_size();
+}
+
+void IoBatch::clearBatch()
+{
+    m_group->Clear();
+}
+
 std::vector<bool> repliesToPinBits( const std::vector<uint32_t>& interestingReplies, uint32_t pinOfInterest )
 {
 
EOF

echo "Creating UaoClientForOpcUaSca/include/BitBangRequests.h to use find_package variables for protobuf"

rm -rf include/BitBangRequests.h

cat <<'EOF'> include/BitBangRequests.h
#pragma once

namespace ScaPb {
enum BitBangRequest_Direction {
  BitBangRequest_Direction_INPUT = 0,
  BitBangRequest_Direction_OUTPUT = 1
};
}
EOF
