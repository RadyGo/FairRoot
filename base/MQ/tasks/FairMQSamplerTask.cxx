/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             *
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
/**
 * FairMQSamplerTask.cxx
 *
 * @since 2012-11-22
 * @author D. Klein, A. Rybalchenko
 */

#include "FairMQSamplerTask.h"

using namespace std;

FairMQSamplerTask::FairMQSamplerTask()
    : FairTask("Abstract base task used for loading a branch from a root file into memory")
    , fBranch()
    , fInput(nullptr)
    , fPayload()
    , fEventIndex(0)
    , fEvtHeader(nullptr)
    , fTransportFactory(nullptr)
{
}

FairMQSamplerTask::FairMQSamplerTask(const Text_t* name, int iVerbose)
    : FairTask(name, iVerbose)
    , fBranch()
    , fInput(nullptr)
    , fPayload()
    , fEventIndex(0)
    , fEvtHeader(nullptr)
    , fTransportFactory(nullptr)
{
}

FairMQSamplerTask::~FairMQSamplerTask()
{
    delete fInput;
}

InitStatus FairMQSamplerTask::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    fEvtHeader = static_cast<FairEventHeader*>(ioman->GetObject("EventHeader."));
    fInput = static_cast<TClonesArray*>(ioman->GetObject(fBranch.c_str()));

    return kSUCCESS;
}

void FairMQSamplerTask::Exec(Option_t* /*opt*/)
{
}

void FairMQSamplerTask::SetBranch(string branch)
{
    fBranch = branch;
}

void FairMQSamplerTask::SetEventIndex(Long64_t eventIndex)
{
    fEventIndex = eventIndex;
}

void FairMQSamplerTask::GetPayload(std::unique_ptr<FairMQMessage>& msg)
{
    msg = std::move(fPayload);
}

void FairMQSamplerTask::SetTransport(FairMQTransportFactory* factory)
{
    fTransportFactory = factory;
}
