#include "RejectPolicy.hpp"
#include <cstdlib>
#include <iostream>

BaseRejectPolicy::BaseRejectPolicy()= default;

BaseRejectPolicy::~BaseRejectPolicy()= default;

void BaseRejectPolicy::reject(function<void()>&&)
{

}

DiscardPolicy::DiscardPolicy()
{
    cout<<"construct DiscardPolicy\n";
}

DiscardPolicy::~DiscardPolicy()= default;

void DiscardPolicy::reject(function<void()>&&)
{
    std::cout<<"\n *** Discard Policy ***\n"<<std::endl;
}

AbortPolicy::AbortPolicy()
{
    cout<<"construct AbortPolicy\n";
}

AbortPolicy::~AbortPolicy()= default;

void AbortPolicy::reject(function<void()>&&)
{
    std::cout<<"\n *** Abort Policy ***\n"<<std::endl;
    abort();
}

CallerRunPolicy::CallerRunPolicy()
{
    cout<<"construct CallerRunPolicy\n";
}

CallerRunPolicy::~CallerRunPolicy()= default;

void CallerRunPolicy::reject(function<void()>&& task)
{
    std::cout<<"\n *** Caller Run Policy ***\n"<<std::endl;
    task();
}


RejectPolicyFactory* RejectPolicyFactory::factoryInstance = nullptr;

RejectPolicyFactory::RejectPolicyFactory()= default;

RejectPolicyFactory::~RejectPolicyFactory() = default;

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
        if(discardPolicy == nullptr)
        {
            discardPolicy = new DiscardPolicy();
        }
        rejectPolicy = discardPolicy;
        break;
    case Abort:
        if(abortPolicy == nullptr)
        {
            abortPolicy = new AbortPolicy();
        }
        rejectPolicy = abortPolicy;
        break;
    case CallerRun:
        if(callerRunPolicy == nullptr)
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
