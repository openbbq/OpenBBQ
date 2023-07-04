#pragma once

#include <Print.h>
#include <openbbq/config/Config.h>

template <typename T>
class IControlSignal : public Printable
{
protected:
    typedef T Value;

public:
    virtual ~IControlSignal() = default;
    virtual Value value() const = 0;
    virtual void value(const Value &t) = 0;

    size_t printTo(Print &p) const override
    {
        return p.print(value());
    }

    operator Value() const { return value(); }
    IControlSignal<T> &operator=(const Value &t)
    {
        value(t);
        return *this;
    }
};

template <typename T>
class ControlSignal : public IControlSignal<T>
{
protected:
    typedef T Value;

public:
    ControlSignal(const Value &value) : _value(value) {}
    Value value() const override { return _value; }
    void value(const Value &t) override { _value = t; }

    ControlSignal<Value> &operator=(const T &t)
    {
        value(t);
        return *this;
    }

private:
    Value _value;
};

template <typename TBase>
class WithConnect : public TBase
{
protected:
    typedef typename TBase::Value Value;

public:
    WithConnect(const Value &value) : TBase(value) {}

    void connect(IControlSignal<Value> &source) { return connect(&source); }
    virtual void connect(IControlSignal<Value> *source)
    {
        _source = source;
    }

    Value value() const override
    {
        if (_source)
        {
            return _source->value();
        }
        else
        {
            return TBase::value();
        }
    }

    void value(const Value &t) override
    {
        if (_source)
        {
            _source->value(t);
        }
        else
        {
            TBase::value(t);
        }
    }

private:
    IControlSignal<Value> *_source = nullptr;
};

template <typename TBase>
class WithConfig : public TBase, public ConfigBase
{
protected:
    typedef typename TBase::Value Value;

public:
    WithConfig(const Value &value) : TBase(value) {}

    void unmarshal(JsonVariant json) override
    {
        value(json);
    }

    void marshal(JsonVariant json) override
    {
        // TODO - panic
    }

    void marshal(JsonVariant json, const String &key) override
    {
        json[key] = value();
    }

    Value value() const override { return TBase::value(); }
    void value(const Value &t) override
    {
        changed(true);
        TBase::value(t);
    }
};

template <>
inline void WithConfig<ControlSignal<float>>::marshal(JsonVariant json, const String &key)
{
    json[key] = serialized(String(value(), 4));
}

template <typename TBase>
class WithRange : public TBase
{
protected:
    typedef typename TBase::Value Value;

public:
    WithRange(const Value &value, const Value &low, const Value &high) : TBase(value), _low(low), _high(high) {}

    Value value() const override { return constrain(TBase::value(), _low, _high); }
    void value(const Value &t) override { TBase::value(constrain(t, _low, _high)); }

    WithRange &operator=(const Value &t)
    {
        value(t);
        return *this;
    }

private:
    Value _low;
    Value _high;
};
