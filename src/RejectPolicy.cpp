#include "RejectPolicy.hpp"
#include <stdlib.h>

BaseRejectPolicy::BaseRejectPolicy(){}

BaseRejectPolicy::~BaseRejectPolicy(){}

void BaseRejectPolicy::reject(function<void()>)
{
    return;
}

DiscardPolicy::DiscardPolicy(){}

DiscardPolicy::~DiscardPolicy(){}

void DiscardPolicy::reject(function<void()>)
{
    return ;
}

AbortPolicy::AbortPolicy(){}

AbortPolicy::~AbortPolicy(){}

void AbortPolicy::reject(function<void()>)
{
    abort();
}

CallerRunPolicy::CallerRunPolicy(){}

CallerRunPolicy::~CallerRunPolicy(){}

void CallerRunPolicy::reject(function<void()> task)
{
    task();
}


RejectPolicyFactory* RejectPolicyFactory::factoryInstance = nullptr;

RejectPolicyFactory::RejectPolicyFactory(){}

RejectPolicyFactory* RejectPolicyFactory::getInstance()
{
    if(factoryInstance == nullptr)
    {
        factoryInstance = new RejectPolicyFactory();
    }
    return factoryInstance;
}

BaseRejectPolicy* RejectPolicyFactory::getRejectPolicy(Policy policy)
{
    switch (policy)
    {
    case Discard:
        rejectPolicy = new DiscardPolicy();
        break;
    case Abort:
        rejectPolicy = new AbortPolicy();
        break;
    case CallerRun:
        rejectPolicy = new CallerRunPolicy();
    default:
        rejectPolicy = new BaseRejectPolicy();
        break;
    }
    return rejectPolicy;
}
