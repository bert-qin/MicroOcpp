// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#ifndef METERVALUES_H
#define METERVALUES_H

#include <ArduinoOcpp/Core/OcppMessage.h>
#include <ArduinoOcpp/Core/OcppTime.h>
#include <ArduinoOcpp/Tasks/Metering/MeterValue.h>

#include <vector>

namespace ArduinoOcpp {
namespace Ocpp16 {

class MeterValues : public OcppMessage {
private:
    std::vector<std::unique_ptr<MeterValue>> meterValue;

    int connectorId = 0;
    int transactionId = -1;

public:
    MeterValues(std::vector<std::unique_ptr<MeterValue>>&& meterValue, int connectorId, int transactionId);

    MeterValues(); //for debugging only. Make this for the server pendant

    ~MeterValues();

    const char* getOcppOperationType();

    std::unique_ptr<DynamicJsonDocument> createReq();

    void processConf(JsonObject payload);

    void processReq(JsonObject payload);

    std::unique_ptr<DynamicJsonDocument> createConf();
};

} //end namespace Ocpp16
} //end namespace ArduinoOcpp
#endif
