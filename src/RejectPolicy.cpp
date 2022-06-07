#include "RejectPolicy.hpp"
#include <stdlib.h>
#include <iostream>

BaseRejectPolicy::BaseRejectPolicy(){}

BaseRejectPolicy::~BaseRejectPolicy(){}

void BaseRejectPolicy::reject(function<void()>)
{
    return;
}

DiscardPolicy::DiscardPolicy()
{
    cout<<"construct DiscardPolicy\n";
}

DiscardPolicy::~DiscardPolicy(){}

void DiscardPolicy::reject(function<void()>)
{
    std::cout<<"\n *** Discard Policy ***\n"<<std::endl;
    return ;
}

AbortPolicy::AbortPolicy()
{
    cout<<"construct AbortPolicy\n";
}

AbortPolicy::~AbortPolicy(){}

void AbortPolicy::reject(function<void()>)
{
    std::cout<<"\n *** Abort Policy ***\n"<<std::endl;
    abort();
}

CallerRunPolicy::CallerRunPolicy()
{
    cout<<"construct CallerRunPolicy\n";
}

CallerRunPolicy::~CallerRunPolicy(){}

void CallerRunPolicy::reject(function<void()> task)
{
    std::cout<<"\n *** Caller Run Policy ***\n"<<std::endl;
    task();
}


RejectPolicyFactory* RejectPolicyFactory::factoryInstance = nullptr;

RejectPolicyFactory::RejectPolicyFactory(){}

RejectPolicyFactory* RejectPolicyFactory::getInstance()
{
    cout<<"get instance\n";
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
        if(discardPolicy == NULL)
        {
            discardPolicy = new DiscardPolicy();
        }
        rejectPolicy = discardPolicy;
        break;
    case Abort:
        if(abortPolicy == NULL)
        {
            abortPolicy = new AbortPolicy();
        }
        rejectPolicy = abortPolicy;
        break;
    case CallerRun:
        if(callerRunPolicy == NULL)
        {
            callerRunPolicy = new CallerRunPolicy();
        }
        rejectPolicy = callerRunPolicy;
        break;
    default:
        rejectPolicy = new BaseRejectPolicy();
        break;
    }
    return rejectPolicy;
}
