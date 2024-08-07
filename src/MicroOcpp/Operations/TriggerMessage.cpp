// matth-x/MicroOcpp
// Copyright Matthias Akstaller 2019 - 2024
// MIT License

#include <MicroOcpp/Operations/TriggerMessage.h>
#include <MicroOcpp/Model/ConnectorBase/Connector.h>
#include <MicroOcpp/Model/Metering/MeteringService.h>
#include <MicroOcpp/Operations/StatusNotification.h>
#include <MicroOcpp/Model/Model.h>
#include <MicroOcpp/Core/Context.h>
#include <MicroOcpp/Core/Request.h>
#include <MicroOcpp/Debug.h>

using MicroOcpp::Ocpp16::TriggerMessage;

TriggerMessage::TriggerMessage(Context& context) : context(context) {

}

const char* TriggerMessage::getOperationType(){
    return "TriggerMessage";
}

void TriggerMessage::processReq(JsonObject payload) {

    const char *requestedMessage = payload["requestedMessage"] | "Invalid";
    const int connectorId = 
    #if MO_ENABLE_V201
            context.getVersion().major == 2 ?
                payload["evse"]["id"] | -1 :
#endif //MO_ENABLE_V201
                payload["connectorId"] | -1;

    MO_DBG_INFO("Execute for message type %s, connectorId = %i", requestedMessage, connectorId);

    statusMessage = "Rejected";

    if (!strcmp(requestedMessage, "MeterValues")) {
        if (auto mService = context.getModel().getMeteringService()) {
            if (connectorId < 0) {
                auto nConnectors = mService->getNumConnectors();
                for (decltype(nConnectors) cId = 0; cId < nConnectors; cId++) {
                    auto request = mService->takeTriggeredMeterValues(cId);
                    if(request){
                        context.getRequestQueue().sendRequestPreBoot(std::move(request));
                        statusMessage = "Accepted";
                    }
                }
            } else if (connectorId < mService->getNumConnectors()) {
                context.getRequestQueue().sendRequestPreBoot(mService->takeTriggeredMeterValues(connectorId));
                statusMessage = "Accepted";
            } else {
                //hehongyang, OCTT test case 055 need reject
                //errorCode = "PropertyConstraintViolation";
            }
        }
    }
#if MO_ENABLE_V201
    else if (!strcmp(requestedMessage, "TransactionEvent")) {
        if (auto mService = context.getModel().getMeteringService()) {
            if (connectorId < 0) {
                auto nConnectors = mService->getNumConnectors();
                for (decltype(nConnectors) cId = 0; cId < nConnectors; cId++) {
                    if(mService->takeTriggeredTransactionEvent(cId)){
                        statusMessage = "Accepted";
                    }
                }
            } else if (connectorId < mService->getNumConnectors()) {
                if(mService->takeTriggeredTransactionEvent(connectorId)){
                    statusMessage = "Accepted";
                }
            } else {
                //do nothing
            }
        }
    }
#endif
    else if (!strcmp(requestedMessage, "StatusNotification")) {
        unsigned int cIdRangeBegin = 0, cIdRangeEnd = 0;
        if (connectorId < 0) {
            cIdRangeEnd = context.getModel().getNumConnectors();
        } else if ((unsigned int) connectorId < context.getModel().getNumConnectors()) {
            cIdRangeBegin = connectorId;
            cIdRangeEnd = connectorId + 1;
        } else {
            errorCode = "PropertyConstraintViolation";
        }

        for (auto i = cIdRangeBegin; i < cIdRangeEnd; i++) {
            auto connector = context.getModel().getConnector(i);
            auto statusNotification = 
#if MO_ENABLE_V201
            context.getVersion().major == 2 ?
                makeRequest(
                    new Ocpp201::StatusNotification(i, connector->getStatus(), context.getModel().getClock().now())) :
#endif //MO_ENABLE_V201
                makeRequest(
                    new Ocpp16::StatusNotification(i, connector->getStatus(), context.getModel().getClock().now()));
            statusNotification->setTimeout(60000);

            context.getRequestQueue().sendRequestPreBoot(std::move(statusNotification));
            statusMessage = "Accepted";
        }
    } else {
        auto msg = context.getOperationRegistry().deserializeOperation(requestedMessage);
        if (msg) {
            context.getRequestQueue().sendRequestPreBoot(std::move(msg));
            statusMessage = "Accepted";
        } else {
            statusMessage = "NotImplemented";
        }
    }
}

std::unique_ptr<DynamicJsonDocument> TriggerMessage::createConf(){
    auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(1)));
    JsonObject payload = doc->to<JsonObject>();
    payload["status"] = statusMessage;
    return doc;
}
