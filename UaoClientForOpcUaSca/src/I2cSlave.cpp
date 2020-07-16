

#include <iostream>
#include <I2cSlave.h>
#include <uaclient/uasession.h>
#include <stdexcept>
#include <UaoClientForOpcUaScaArrayTools.h>
#include <UaoClientForOpcUaScaUaoExceptions.h>

namespace UaoClientForOpcUaSca
{


I2cSlave::
I2cSlave
(
    UaSession* session,
    UaNodeId objId
) :
    m_session(session),
    m_objId (objId)
{

}



UaByteString I2cSlave::readValue (
    UaStatus *out_status,
    UaDateTime *sourceTimeStamp,
    UaDateTime *serverTimeStamp)
{

    ServiceSettings   ss;
    UaReadValueIds    nodesToRead;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".value"), m_objId.namespaceIndex() );

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

    UaByteString out;


    UaStatus conversionStatus = (UaVariant(dataValues[0].Value)).toByteString (out);
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


void  I2cSlave::writeValue (
    UaByteString & data,
    UaStatus                                 *out_status)
{
    ServiceSettings   ss;
    UaWriteValues    nodesToWrite;
    UaDataValues      dataValues;
    UaDiagnosticInfos diagnosticInfos;
    UaStatusCodeArray results;

    UaNodeId nodeId ( UaString(m_objId.identifierString()) + UaString(".value"), m_objId.namespaceIndex()  );

    nodesToWrite.create(1);
    nodeId.copyTo( &nodesToWrite[0].NodeId );
    nodesToWrite[0].AttributeId = OpcUa_Attributes_Value;

    UaVariant v ( data );

    v.setByteString( data, false );


    dataValues.create (1);
    v.copyTo( &nodesToWrite[0].Value.Value );

    UaStatus status = m_session->write(
                          ss,
                          nodesToWrite,
                          results,
                          diagnosticInfos
                      );
    if (out_status)
    {
        *out_status = status;
    }
    else
    {
        if (status.isBad())
            throw Exceptions::BadStatusCode("OPC-UA write failed", status.statusCode() );
        if (results[0] != OpcUa_Good)
            throw Exceptions::BadStatusCode ("OPC-UA write failed", results[0] );
    }

}

void I2cSlave::readSlave (
    OpcUa_Byte in_numberOfBytes,UaByteString & out_reply
)
{

    ServiceSettings serviceSettings;
    CallIn callRequest;
    CallOut co;

    callRequest.objectId = m_objId;
    callRequest.methodId = UaNodeId( UaString(m_objId.identifierString()) + UaString(".readSlave"), 2 );


    UaVariant v;


    callRequest.inputArguments.create( 1 );

    v.setByte( in_numberOfBytes );



    v.copyTo( &callRequest.inputArguments[ 0 ] );



    UaStatus status =
        m_session->call(
            serviceSettings,
            callRequest,
            co
        );
    if (status.isBad())
        throw Exceptions::BadStatusCode("In OPC-UA call", status.statusCode());


    v = co.outputArguments[0];

    v.toByteString (out_reply);



}


void I2cSlave::writeSlave (
    const UaByteString &  in_payload
)
{

    ServiceSettings serviceSettings;
    CallIn callRequest;
    CallOut co;

    callRequest.objectId = m_objId;
    callRequest.methodId = UaNodeId( UaString(m_objId.identifierString()) + UaString(".writeSlave"), 2 );


    UaVariant v;


    callRequest.inputArguments.create( 1 );

    v.setByteString( const_cast<UaByteString&>(in_payload), false );



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


