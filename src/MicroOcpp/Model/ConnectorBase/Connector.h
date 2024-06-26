// matth-x/MicroOcpp
// Copyright Matthias Akstaller 2019 - 2024
// MIT License

#ifndef MO_CONNECTOR_H
#define MO_CONNECTOR_H

#include <MicroOcpp/Model/ConnectorBase/ChargePointStatus.h>
#include <MicroOcpp/Model/ConnectorBase/ChargePointErrorData.h>
#include <MicroOcpp/Model/ConnectorBase/Notification.h>
#include <MicroOcpp/Model/ConnectorBase/UnlockConnectorResult.h>
#include <MicroOcpp/Core/ConfigurationKeyValue.h>
#include <MicroOcpp/Operations/CiStrings.h>

#include <vector>
#include <functional>
#include <memory>

namespace MicroOcpp {

class Context;
class Model;
class Operation;
class ITransaction;

class Connector {
private:
    Context& context;
    Model& model;
    
    const int connectorId;

    std::shared_ptr<ITransaction> transaction;

    std::shared_ptr<Configuration> availabilityBool;
    char availabilityBoolKey [sizeof(MO_CONFIG_EXT_PREFIX "AVAIL_CONN_xxxx") + 1];
    bool availabilityVolatile = true;

    std::function<bool()> connectorPluggedInput;
    std::function<bool()> evReadyInput;
    std::function<bool()> evseReadyInput;
    std::vector<std::function<ErrorData ()>> errorDataInputs;
    std::vector<bool> trackErrorDataInputs;
    bool isFaulted();
    const char *getErrorCode();

    ChargePointStatus currentStatus = ChargePointStatus_UNDEFINED;
    std::shared_ptr<Configuration> minimumStatusDurationInt; //in seconds
    ChargePointStatus reportedStatus = ChargePointStatus_UNDEFINED;
    unsigned long t_statusTransition = 0;

#if MO_ENABLE_CONNECTOR_LOCK
    std::function<UnlockConnectorResult()> onUnlockConnector;
#endif //MO_ENABLE_CONNECTOR_LOCK

    std::function<bool()> startTxReadyInput; //the StartTx request will be delayed while this Input is false
    std::function<bool()> stopTxReadyInput; //the StopTx request will be delayed while this Input is false
    std::function<bool()> occupiedInput; //instead of Available, go into Preparing / Finishing state

    std::function<void(ITransaction*,TxNotification)> txNotificationOutput;

    std::shared_ptr<Configuration> connectionTimeOutInt; //in seconds
    std::shared_ptr<Configuration> stopTransactionOnInvalidIdBool;
    std::shared_ptr<Configuration> stopTransactionOnEVSideDisconnectBool;
    std::shared_ptr<Configuration> localPreAuthorizeBool;
    std::shared_ptr<Configuration> localAuthorizeOfflineBool;
    std::shared_ptr<Configuration> allowOfflineTxForUnknownIdBool;

    std::shared_ptr<Configuration> silentOfflineTransactionsBool;
    std::shared_ptr<Configuration> authorizationTimeoutInt; //in seconds
    std::shared_ptr<Configuration> freeVendActiveBool;
    std::shared_ptr<Configuration> freeVendIdTagString;
    bool freeVendTrackPlugged = false;

    std::shared_ptr<Configuration> txStartOnPowerPathClosedBool; // this postpones the tx start point to when evReadyInput becomes true

    bool trackLoopExecute = false; //if loop has been executed once
public:
    Connector(Context& context, int connectorId);
    Connector(const Connector&) = delete;
    Connector(Connector&&) = delete;
    Connector& operator=(const Connector&) = delete;

    ~Connector();

    /*
     * beginTransaction begins the transaction process which eventually leads to a StartTransaction
     * request in the normal case.
     * 
     * If the transaction process begins successfully, a Transaction object is returned
     * If no transaction process begins due to this call, nullptr is returned (e.g. memory allocation failed)
     */
    std::shared_ptr<ITransaction> beginTransaction(const char *idTag);
    std::shared_ptr<ITransaction> beginTransaction_authorized(const char *idTag, const char *parentIdTag = nullptr);

    /*
     * End the current transaction process, if existing and not ended yet. This eventually leads to
     * a StopTransaction request, if the transaction process has actually ended due to this call. It
     * is safe to call this function at any time even if no transaction is running
     */
    void endTransaction(const char *idTag = nullptr, const char *reason = nullptr);
    void endTransaction_authorized(const char *idTag = nullptr, const char *reason = nullptr);
    
    std::shared_ptr<ITransaction>& getTransaction();

    //create detached transaction - won't have any side-effects with the transaction handling of this lib
    std::shared_ptr<ITransaction> allocateTransaction(); 

    bool isOperative();
    void setAvailability(bool available);
    void setAvailabilityVolatile(bool available); //set inoperative state but keep only until reboot at most
    void setConnectorPluggedInput(std::function<bool()> connectorPlugged);
    void setEvReadyInput(std::function<bool()> evRequestsEnergy);
    void setEvseReadyInput(std::function<bool()> connectorEnergized);
    void addErrorCodeInput(std::function<const char*()> connectorErrorCode);
    void addErrorDataInput(std::function<ErrorData ()> errorCodeInput);

    void loop();

    ChargePointStatus getStatus();

    bool ocppPermitsCharge();

#if MO_ENABLE_CONNECTOR_LOCK
    void setOnUnlockConnector(std::function<UnlockConnectorResult()> unlockConnector);
    std::function<UnlockConnectorResult()> getOnUnlockConnector();
#endif //MO_ENABLE_CONNECTOR_LOCK

    void setStartTxReadyInput(std::function<bool()> startTxReady);
    void setStopTxReadyInput(std::function<bool()> stopTxReady);
    void setOccupiedInput(std::function<bool()> occupied);

    void setTxNotificationOutput(std::function<void(ITransaction*,TxNotification)> txNotificationOutput);
    void updateTxNotification(TxNotification event);
};

} //end namespace MicroOcpp
#endif
