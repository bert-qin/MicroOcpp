// matth-x/MicroOcpp
// Copyright Matthias Akstaller 2019 - 2024
// MIT License

#ifndef CONFIGURATIONKEYVALUE_H
#define CONFIGURATIONKEYVALUE_H

#include <ArduinoJson.h>
#include <memory>

#define MO_CONFIG_MAX_VALSTRSIZE 128

#ifndef MO_CONFIG_EXT_PREFIX
#define MO_CONFIG_EXT_PREFIX "Cst_"
#endif

#ifndef MO_CONFIG_TYPECHECK
#define MO_CONFIG_TYPECHECK 1 //enable this for debugging
#endif

namespace MicroOcpp {

using revision_t = uint16_t;

enum class TConfig : uint8_t {
    Int,
    Bool,
    String
};

template<class T>
TConfig convertType();

class ICfg{
public:
    virtual ~ICfg()=default;
    virtual int getInt()=0;
    virtual void setInt(int)=0;
    virtual bool getBool()=0;
    virtual void setBool(bool)=0;
    virtual const char *getString()=0;
    virtual bool setString(const char*)=0;
    virtual revision_t getValueRevision()=0;
};

class Configuration : public ICfg{
protected:
    revision_t value_revision = 0; //write access counter; used to check if this config has been changed
private:
    bool rebootRequired = false;
    bool readOnly = false;
public:
    virtual ~Configuration();

    virtual bool setKey(const char *key) = 0;
    virtual const char *getKey() = 0;

    virtual void setInt(int);
    virtual void setBool(bool);
    virtual bool setString(const char*);

    virtual int getInt();
    virtual bool getBool();
    virtual const char *getString() override; //always returns c-string (empty if undefined)

    virtual TConfig getType() = 0;

    virtual revision_t getValueRevision() override;

    void setRebootRequired();
    bool isRebootRequired();

    void setReadOnly();
    bool isReadOnly();
};

/*
 * Default implementations of the Configuration interface.
 *
 * How to use custom implementations: for each OCPP config, pass a config instance to the OCPP lib
 * before its initialization stage. Then the library won't create new config objects but 
 */
std::unique_ptr<Configuration> makeConfiguration(TConfig type, const char *key);

const char *serializeTConfig(TConfig type);
bool deserializeTConfig(const char *serialized, TConfig& out);

} //end namespace MicroOcpp

#endif
