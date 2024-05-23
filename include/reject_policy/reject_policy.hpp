/**
 * 拒绝策略的定义与实现类。
 * 使用策略模式统一管理。
 * 使用工厂配合单件模式实例化拒绝策略。
 */
#include <functional>

enum Policy { Abort, Discard, CallerRun };

class BaseRejectPolicy {
 private:
 public:
  BaseRejectPolicy();
  virtual ~BaseRejectPolicy();

  virtual void reject(std::function<void()>&&);
};

class AbortPolicy : public BaseRejectPolicy {
 public:
  AbortPolicy();
  ~AbortPolicy() override;

  void reject(std::function<void()>&&) override;
};

class CallerRunPolicy : public BaseRejectPolicy {
 public:
  CallerRunPolicy();
  ~CallerRunPolicy() override;
  void reject(std::function<void()>&&) override;
};

class DiscardPolicy : public BaseRejectPolicy {
 public:
  DiscardPolicy();
  ~DiscardPolicy() override;
  void reject(std::function<void()>&&) override;
};

class RejectPolicyFactory {
 private:
  static RejectPolicyFactory* factoryInstance;
  BaseRejectPolicy* rejectPolicy = nullptr;
  AbortPolicy* abortPolicy = nullptr;
  DiscardPolicy* discardPolicy = nullptr;
  CallerRunPolicy* callerRunPolicy = nullptr;
  RejectPolicyFactory();

 public:
  ~RejectPolicyFactory();
  static RejectPolicyFactory& getInstance();
  BaseRejectPolicy* getRejectPolicy(Policy);
};