#pragma once

#include <cstdint>
#include <string>
#include <vector> 
#include <map>
#include <fstream>
#include "Config.h"
#include "src/instances/DataModel.h"
#include "src/instances/Instance.h"

// Place Format (.place)
// Header: ENGINE_NAME (5 bytes) + ENGINE_VERSION (3 bytes)
// Data: Storing the Datamodel in a hierachal pattern

#pragma pack(push, 1)
typedef struct PlaceHeader {
    char engine_header[5];
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
} PlaceHeader;
#pragma pack(pop)

// PlaceHeader header = {
//     ENGINE_HEADER[0], ENGINE_HEADER[1], ENGINE_HEADER[2], ENGINE_HEADER[3], ENGINE_HEADER[4],
//     ENGINE_VERSION_MAJOR,
//     ENGINE_VERSION_MINOR,
//     ENGINE_VERSION_PATCH
// };

struct PlaceSerialisation {
    // API
    static bool SavePlace(const std::string &filePath, DataModel *dataModel); // save from datamodel to filePath
    static bool LoadPlace(const std::string &filePath, DataModel *dataModel); // load to datamodel from filePath

    private:
    // what we need to reconstruct every Instance; if we can put this all into properties it'd be ideal but for now this is fine
    struct InstanceData {
        std::string className; // instance type
        std::string name; // instance name
        std::map<std::string, std::string> properties; // map string -> property; maybe storing value as string is bad, we'll see
        std::vector<InstanceData> children; // children of the instance hierachy
    };

    // Serialization
    static bool SerializeInstance(Instance *inst, InstanceData &data);
    static Instance *DeserializeInstance(const InstanceData &data,
                                        lua_State *L = nullptr);

    // Binary I/O
    static void WriteString(std::ofstream &f, const std::string &str);
    static std::string ReadString(std::ifstream &f);
    static void WriteInstanceTree(std::ofstream &f, const InstanceData &data);
    static bool ReadInstanceTree(std::ifstream &f, InstanceData &data);
    
    // Validation; to ensure correct runtime version for this (.place)
    // TODO: maybe add some kind of migration?
    static bool ValidateHeader(const PlaceHeader &header);
    static PlaceHeader CreateHeader();
};