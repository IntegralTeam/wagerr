// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018 The Wagerr developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "net.h"
#include "betting/bet.h"
#include "rpc/server.h"

#include <univalue.h>

/**
 * Looks up a given map index for a given name. If found then it will return the mapping ID.
 * If its not found then create a new mapping ID and also indicate with a boolean that a new
 * mapping OP_CODE needs to be created and broadcast to the network.
 *
 * @param params The RPC params consisting of an map index name and name.
 * @param fHelp  Help text
 * @return
 */
UniValue getmappingid(const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() < 2))
        throw std::runtime_error(
                "getmappingid\n"
                "\nGet a mapping ID from the specified mapping index.\n"

                "\nResult:\n"
                "[\n"
                "  {\n"
                "    \"mapping index id\": \"xxx\",  (numeric) The mapping index.\n"
                "    \"exists\": \"xxx\", (boolean) mapping id exists\n"
                "    \"mapping-index\": \"xxx\" (string) The index that was searched.\n"
                "  }\n"
                "]\n"

                "\nExamples:\n" +
                HelpExampleCli("getmappingid", "") + HelpExampleRpc("getmappingid", ""));

    CMappingDB dbMapping{};
    mappingIndex_t mappingIndex{};
    std::string mIndex = params[0].get_str();
    std::string name   = params[1].get_str();
    UniValue ret(UniValue::VARR);
    UniValue mapping(UniValue::VOBJ);
    const MappingTypes type = CMapping::FromTypeName(mIndex);

    if (CMapping::ToTypeName(type) != mIndex) {
        throw std::runtime_error("No mapping exist for the mapping index you provided.");
    }

    if (!dbMapping.Read(type, mappingIndex)) {
        throw std::runtime_error("No mapping saved for the mapping type you provided.");
    }

    // Check the map for the string name.
    unsigned int nFirstIndexFree = 0;
    bool mappingFound  = false;
    bool FirstIndexFreeFound = false;
    std::map<uint32_t, CMapping>::iterator it;
    for (it = mappingIndex.begin(); it != mappingIndex.end(); it++) {
        LogPrintf("%s - mapping - it=[%d] nId=[%d] nMType=[%d] nVersion=[%d] [%s]\n", __func__, it->first, it->second.nId, it->second.nMType, it->second.nVersion, it->second.sName);
        if (!mappingFound) {
            if (it->second.sName == name) {
                mapping.push_back(Pair("mapping-id", (uint64_t) it->second.nId));
                mapping.push_back(Pair("exists", true));
                mapping.push_back(Pair("mapping-index", mIndex));
                mappingFound = true;
            }
        }
        // Find the first available free key in the sorted map
        if (!FirstIndexFreeFound){
            if (it->first != nFirstIndexFree) {
                FirstIndexFreeFound = true;
            } else {
                nFirstIndexFree++;
            }
        }
    }

    // If no mapping found then create a new one and add to the given map index.
    if (!mappingFound) {
        CMapping cm{};
        cm.nMType   = type;
        cm.nId      = nFirstIndexFree;
        cm.sName    = name;
        cm.nVersion = 1;

        dbMapping.Save(cm);
        mapping.push_back(Pair("mapping-id",  (uint64_t) nFirstIndexFree));
        mapping.push_back(Pair("exists", false));
        mapping.push_back(Pair("mapping-index", mIndex));
    }

    ret.push_back(mapping);

    return ret;
}

/**
 * Looks up a given map index for a given ID. If found then it will return the mapping name.
 * If its not found return an error message.
 *
 * @param params The RPC params consisting of an map index name and id.
 * @param fHelp  Help text
 * @return
 */
UniValue getmappingname(const UniValue& params, bool fHelp)
{
    if (fHelp || (params.size() < 2))
        throw std::runtime_error(
                "getmappingname\n"
                "\nGet a mapping string name from the specified map index.\n"

                "\nResult:\n"
                "[\n"
                "  {\n"
                "    \"mapping name\": \"xxx\",  (string) The mapping name.\n"
                "    \"exists\": \"xxx\", (boolean) mapping transaction created or not\n"
                "    \"mapping-index\": \"xxx\" (string) The index that was searched.\n"
                "  }\n"
                "]\n"

                "\nExamples:\n" +
                HelpExampleCli("getmappingname", "") + HelpExampleRpc("getmappingname", ""));

    CMappingDB dbMapping{};
    mappingIndex_t mappingIndex{};
    std::string mIndex = params[0].get_str();
    uint32_t id        = std::stoi(params[1].get_str());
    UniValue ret(UniValue::VARR);
    UniValue mapping(UniValue::VOBJ);
    const MappingTypes type = CMapping::FromTypeName(mIndex);

    if (CMapping::ToTypeName(type) != mIndex) {
        throw std::runtime_error("No mapping exist for the mapping index you provided.");
    }

    if (!dbMapping.Read(type, mappingIndex)) {
        throw std::runtime_error("No mapping saved for the mapping type you provided.");
    }

    // Check the map for the mapping ID.
    bool mappingFound = false;
    std::map<uint32_t, CMapping>::iterator it;
    for (it = mappingIndex.begin(); it != mappingIndex.end(); it++) {
        if (it->first == id) {
            mapping.push_back(Pair("mapping-name", it->second.sName));
            mapping.push_back(Pair("exists", true));
            mapping.push_back(Pair("mapping-index", mIndex));
            mappingFound = true;
            break;
        }
    }

    if (!mappingFound) {
        throw std::runtime_error("Currently no mapping name exists for the mapping name you provided.");
    }

    ret.push_back(mapping);

    return ret;
}
