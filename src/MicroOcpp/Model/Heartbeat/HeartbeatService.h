// matth-x/MicroOcpp
// Copyright Matthias Akstaller 2019 - 2024
// MIT License

#ifndef MO_HEARTBEATSERVICE_H
#define MO_HEARTBEATSERVICE_H

#include <MicroOcpp/Core/ConfigurationKeyValue.h>
#include <memory>

namespace MicroOcpp {

class Context;

class HeartbeatService {
private:
    Context& context;

    unsigned long lastHeartbeat;
    std::shared_ptr<ICfg> heartbeatIntervalInt;

public:
    HeartbeatService(Context& context);

    void loop();
};

}

#endif
