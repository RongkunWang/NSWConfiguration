

#include <iostream>
#include <XilinxFpga.h>
#include <uaclient/uasession.h>
#include <stdexcept>
#include <UaoClientForOpcUaScaArrayTools.h>
#include <UaoClientForOpcUaScaUaoExceptions.h>

namespace UaoClientForOpcUaSca
{


XilinxFpga::
XilinxFpga
(
    UaSession* session,
    UaNodeId objId
) :
    m_session(session),
    m_objId (objId)
{

}



OpcUa_UInt32 XilinxFpga::readIdcodeNumeric (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".idcodeNumeric"), m_objId.namespaceIndex() );

    nodesToRead.create(1);
    nodeId.copyTo( &nodesToRead[0].NodeId );
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;

    dataValues.create (1);

    UaStatus status = m_session->read(
                          ss,
                          0 /*max age*/,
                          OpcUa_TimestampsToReturn_Both,
                          nodesToRead,
                          dataValues,
                          diagnosticInfos
                      );
    if (status.isBad())
        throw Exceptions::BadStatusCode("OPC-UA read failed", status.statusCode());
    if (out_status)
        *out_status = dataValues[0].StatusCode;
    else
    {
        if (! UaStatus(dataValues[0].StatusCode).isGood())
            throw Exceptions::BadStatusCode("OPC-UA read: variable status is not good", dataValues[0].StatusCode );
    }

    OpcUa_UInt32 out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toUInt32 (out);
    if (! conversionStatus.isGood())
    {
        throw std::runtime_error(std::string("OPC-UA read: read succeeded but conversion to native type failed (was it NULL value?): ") + UaStatus(dataValues[0].StatusCode).toString().toUtf8() );
    }


    if (sourceTimeStamp)
        *sourceTimeStamp = dataValues[0].SourceTimestamp;
    if (serverTimeStamp)
        *serverTimeStamp = dataValues[0].ServerTimestamp;

    return out;
}

UaString XilinxFpga::readIdcodeString (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".idcodeString"), m_objId.namespaceIndex() );

    nodesToRead.create(1);
    nodeId.copyTo( &nodesToRead[0].NodeId );
    nodesToRead[0].AttributeId = OpcUa_Attributes_Value;

    dataValues.create (1);

    UaStatus status = m_session->read(
                          ss,
                          0 /*max age*/,
                          OpcUa_TimestampsToReturn_Both,
                          nodesToRead,
                          dataValues,
                          diagnosticInfos
                      );
    if (status.isBad())
        throw Exceptions::BadStatusCode("OPC-UA read failed", status.statusCode());
    if (out_status)
        *out_status = dataValues[0].StatusCode;
    else
    {
        if (! UaStatus(dataValues[0].StatusCode).isGood())
            throw Exceptions::BadStatusCode("OPC-UA read: variable status is not good", dataValues[0].StatusCode );
    }

    UaString out;


    out = UaVariant(dataValues[0].Value).toString();


    if (sourceTimeStamp)
        *sourceTimeStamp = dataValues[0].SourceTimestamp;
    if (serverTimeStamp)
        *serverTimeStamp = dataValues[0].ServerTimestamp;

    return out;
}


void XilinxFpga::program (
    const UaByteString &  in_bitstring
)
{

    ServiceSettings serviceSettings;
    CallIn callRequest;
    CallOut co;

    callRequest.objectId = m_objId;
    callRequest.methodId = UaNodeId( UaString(m_objId.identifierString()) + UaString(".program"), 2 );


    UaVariant v;


    callRequest.inputArguments.create( 1 );

    v.setByteString( const_cast<UaByteString&>(in_bitstring), false );



    v.copyTo( &callRequest.inputArguments[ 0 ] );



    UaStatus status =
        m_session->call(
            serviceSettings,
            callRequest,
            co
        );
    if (status.isBad())
        throw Exceptions::BadStatusCode("In OPC-UA call", status.statusCode());




}



}


