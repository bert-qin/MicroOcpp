// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#include <ArduinoOcpp/Tasks/Metering/MeteringService.h>
#include <ArduinoOcpp/Core/OcppEngine.h>
#include <ArduinoOcpp/SimpleOcppOperationFactory.h>
#include <ArduinoOcpp/Debug.h>

using namespace ArduinoOcpp;

MeteringService::MeteringService(OcppEngine& context, int numConn)
      : context(context) {

    for (int i = 0; i < numConn; i++) {
        connectors.push_back(std::unique_ptr<ConnectorMeterValuesRecorder>(new ConnectorMeterValuesRecorder(context.getOcppModel(), i)));
    }
}

void MeteringService::loop(){

    for (unsigned int i = 0; i < connectors.size(); i++){
        auto meterValuesMsg = connectors[i]->loop();
        if (meterValuesMsg != nullptr) {
            auto meterValues = makeOcppOperation(meterValuesMsg);
            meterValues->setTimeout(std::unique_ptr<Timeout>{new FixedTimeout(120000)});
            context.initiateOperation(std::move(meterValues));
        }
    }
}

void MeteringService::setPowerSampler(int connectorId, PowerSampler ps){
    if (connectorId < 0 || connectorId >= (int) connectors.size()) {
        AO_DBG_ERR("connectorId is out of bounds");
        return;
    }
    connectors[connectorId]->setPowerSampler(ps);
}

void MeteringService::setEnergySampler(int connectorId, EnergySampler es){
    if (connectorId < 0 || connectorId >= (int) connectors.size()) {
        AO_DBG_ERR("connectorId is out of bounds");
        return;
    }
    connectors[connectorId]->setEnergySampler(es);
}

void MeteringService::addMeterValueSampler(int connectorId, std::unique_ptr<SampledValueSampler> meterValueSampler) {
    if (connectorId < 0 || connectorId >= (int) connectors.size()) {
        AO_DBG_ERR("connectorId is out of bounds");
        return;
    }
    connectors[connectorId]->addMeterValueSampler(std::move(meterValueSampler));
}

std::unique_ptr<SampledValue> MeteringService::readTxEnergyMeter(int connectorId, ReadingContext context) {
    if (connectorId < 0 || (size_t) connectorId >= connectors.size()) {
        AO_DBG_ERR("connectorId is out of bounds");
        return nullptr;
    }
    return connectors[connectorId]->readTxEnergyMeter(context);
}

std::unique_ptr<OcppOperation> MeteringService::takeTriggeredMeterValues(int connectorId) {
    if (connectorId < 0 || connectorId >= (int) connectors.size()) {
        AO_DBG_ERR("connectorId out of bounds. Ignore");
        return nullptr;
    }
    auto& connector = connectors.at(connectorId);
    if (connector.get()) {
        auto msg = connector->takeTriggeredMeterValues();
        if (msg) {
            auto meterValues = makeOcppOperation(msg);
            meterValues->setTimeout(std::unique_ptr<Timeout>{new FixedTimeout(120000)});
            return meterValues;
        }
        AO_DBG_DEBUG("Did not take any samples for connectorId %d", connectorId);
        return nullptr;
    }
    AO_DBG_ERR("Could not find connector");
    return nullptr;
}

std::vector<std::unique_ptr<MeterValue>> MeteringService::createStopTxMeterData(int connectorId) {
    if (connectorId < 0 || (size_t) connectorId >= connectors.size()) {
        AO_DBG_ERR("connectorId is out of bounds");
        return std::vector<std::unique_ptr<MeterValue>>();
    }
    auto& connector = connectors[connectorId];

    return connector->createStopTxMeterData();
}
