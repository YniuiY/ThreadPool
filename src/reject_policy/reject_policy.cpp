#include "reject_policy/reject_policy.hpp"

#include <cstdlib>
#include <iostream>

BaseRejectPolicy::BaseRejectPolicy() = default;

BaseRejectPolicy::~BaseRejectPolicy() = default;

void BaseRejectPolicy::reject(std::function<void()>&&) {}

DiscardPolicy::DiscardPolicy() { std::cout << "construct DiscardPolicy\n"; }

DiscardPolicy::~DiscardPolicy() = default;

void DiscardPolicy::reject(std::function<void()>&&) {
  std::cout << "\n *** Discard Policy ***\n" << std::endl;
}

AbortPolicy::AbortPolicy() { std::cout << "construct AbortPolicy\n"; }

AbortPolicy::~AbortPolicy() = default;

void AbortPolicy::reject(std::function<void()>&&) {
  std::cout << "\n *** Abort Policy ***\n" << std::endl;
  abort();
}

CallerRunPolicy::CallerRunPolicy() { std::cout << "construct CallerRunPolicy\n"; }

CallerRunPolicy::~CallerRunPolicy() = default;

void CallerRunPolicy::reject(std::function<void()>&& task) {
  std::cout << "\n *** Caller Run Policy ***\n" << std::endl;
  task();
}

RejectPolicyFactory::RejectPolicyFactory() = default;

RejectPolicyFactory::~RejectPolicyFactory() = default;

RejectPolicyFactory& RejectPolicyFactory::getInstance() {
  static RejectPolicyFactory instance;
  return instance;
}

BaseRejectPolicy* RejectPolicyFactory::getRejectPolicy(Policy policy) {
  switch (policy) {
    case Discard:
      if (discardPolicy == nullptr) {
        discardPolicy = new DiscardPolicy();
      }
      rejectPolicy = discardPolicy;
      break;
    case Abort:
      if (abortPolicy == nullptr) {
        abortPolicy = new AbortPolicy();
      }
      rejectPolicy = abortPolicy;
      break;
    case CallerRun:
      if (callerRunPolicy == nullptr) {
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
