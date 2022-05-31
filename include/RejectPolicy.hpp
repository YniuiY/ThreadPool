/**
 * 拒绝策略的定义与实现类。
 * 使用策略模式统一管理。
 * 使用工厂配合单件模式实例化拒绝策略。
 */
#include <functional>
using namespace std;

enum Policy
{
    Abort,
    Discard,
    CallerRun
};

class BaseRejectPolicy
{
private:

public:
    BaseRejectPolicy();
    virtual ~BaseRejectPolicy();

    virtual void reject(function<void()>);
};

class AbortPolicy : public BaseRejectPolicy
{
public:
    AbortPolicy();
    ~AbortPolicy();

    void reject(function<void()>) override;
};

class CallerRunPolicy : public BaseRejectPolicy
{
public:
    CallerRunPolicy();
    ~CallerRunPolicy();
    void reject(function<void()>) override;
};

class DiscardPolicy : public BaseRejectPolicy
{
public:
    DiscardPolicy();
    ~DiscardPolicy();
    void reject(function<void()>) override;
};

class RejectPolicyFactory
{
private:
    static RejectPolicyFactory* factoryInstance;
    BaseRejectPolicy* rejectPolicy;
    RejectPolicyFactory();
public:
    ~RejectPolicyFactory();
    RejectPolicyFactory* getInstance();
    BaseRejectPolicy* getRejectPolicy(Policy);
};