// matth-x/MicroOcpp
// Copyright Matthias Akstaller 2019 - 2024
// MIT License

#ifndef MO_METERING_CONNECTOR_H
#define MO_METERING_CONNECTOR_H

#include <functional>
#include <memory>
#include <vector>

#include <MicroOcpp/Model/Metering/MeterValue.h>
#include <MicroOcpp/Model/Metering/MeterStore.h>
#include <MicroOcpp/Model/Transactions/Transaction.h>
#include <MicroOcpp/Core/ConfigurationKeyValue.h>

namespace MicroOcpp {

class Model;
class Operation;
class ITransaction;
class MeterStore;

class MeteringConnector {
private:
    Model& model;
    const int connectorId;
    MeterStore& meterStore;
    
    std::vector<std::unique_ptr<MeterValue>> meterData;
    std::shared_ptr<TransactionMeterData> stopTxnData;

    std::unique_ptr<MeterValueBuilder> sampledDataBuilder;
    std::unique_ptr<MeterValueBuilder> alignedDataBuilder;
    std::unique_ptr<MeterValueBuilder> stopTxnSampledDataBuilder;
    std::unique_ptr<MeterValueBuilder> stopTxnAlignedDataBuilder;
    unsigned long lastSampleTime = 0; //0 means not charging right now
    Timestamp nextAlignedTime;
    std::shared_ptr<ITransaction> transaction;
    bool trackTxRunning = false;
 
    std::vector<std::unique_ptr<SampledValueSampler>> samplers;
    int energySamplerIndex {-1};

    std::shared_ptr<ICfg> meterValueSampleIntervalInt;
    std::shared_ptr<ICfg> meterValueCacheSizeInt;
    std::shared_ptr<ICfg> clockAlignedDataIntervalInt;
    std::shared_ptr<ICfg> meterValuesInTxOnlyBool;
    std::shared_ptr<ICfg> stopTxnDataCapturePeriodicBool;
#if MO_ENABLE_V201
    std::shared_ptr<ICfg> sampledDataTxEndedIntervalInt;
    std::shared_ptr<ICfg> alignedDataTxEndedIntervalInt;
    std::unique_ptr<MeterValueBuilder> txStartDataBuilder;
    unsigned long lastTxEndSampleTime = 0; //0 means not charging right now
    Timestamp nextTxEndAlignedTime;
#endif
public:
    MeteringConnector(Model& model, int connectorId, MeterStore& meterStore);

    std::unique_ptr<Operation> loop();

    void addMeterValueSampler(std::unique_ptr<SampledValueSampler> meterValueSampler);

    std::unique_ptr<SampledValue> readTxEnergyMeter(ReadingContext model);

    std::unique_ptr<Operation> takeTriggeredMeterValues();

    void beginTxMeterData(ITransaction *transaction);

    std::shared_ptr<TransactionMeterData> endTxMeterData(ITransaction *transaction);

    void abortTxMeterData();

    std::shared_ptr<TransactionMeterData> getStopTxMeterData(ITransaction *transaction);

    bool existsSampler(const char *measurand, size_t len);

#if MO_ENABLE_V201
    bool takeTriggeredTransactionEvent();
    std::unique_ptr<MeterValue> takeBeginMeterValue();
#endif

};

} //end namespace MicroOcpp
#endif
