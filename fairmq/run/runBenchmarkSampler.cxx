/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/

#include "runFairMQDevice.h"
#include "FairMQBenchmarkSampler.h"

namespace bpo = boost::program_options;

void addCustomOptions(bpo::options_description& options)
{
    options.add_options()
        ("out-channel", bpo::value<std::string>()->default_value("data-out"), "Name of the output channel")
        ("msg-size", bpo::value<int>()->default_value(1000), "Message size in bytes")
        ("num-msgs", bpo::value<int>()->default_value(0),    "Number of messages to send")
        ("msg-rate", bpo::value<int>()->default_value(0), "Msg rate limit in maximum number of messages per second");
}

FairMQDevice* getDevice(const FairMQProgOptions& /*config*/)
{
    return new FairMQBenchmarkSampler();
}
