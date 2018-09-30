#ifndef LVFUNCTION_H
#define LVFUNCTION_H


#include "live/elements/lvelementsglobal.h"
#include "live/elements/value.h"

#include "live/meta/functionargs.h"
#include "live/meta/indextuple.h"

#include <functional>
#include <vector>
#include <string>
#include <tuple>
#include <QDebug>

namespace lv{ namespace el{

/**
 * @brief Base class for any mapped script function, method or event.
 */
class LV_ELEMENTS_EXPORT Function{

public:
    typedef void(*FunctionPointer)(const v8::FunctionCallbackInfo<v8::Value>& value);

    // CallInfo class
    // ----------------

    class LV_ELEMENTS_EXPORT CallInfo{

    public:
        CallInfo(v8::FunctionCallbackInfo<v8::Value> const* info) : m_info(info){}

        template<typename T> T* that() const{
            return reinterpret_cast<T*>(internalField());
        }

        template<int index, typename T> T getValue() const{
            return extractValue<T>(m_info, index);
        }

        template<typename T> static T extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index){
            return static_cast<T>(extractValue<Element*>(info, index));
        }

        LocalValue at(size_t index) const;
        size_t length() const;

        void* userData();

        void assignReturnValue(const v8::Local<v8::Value>& value) const;
        v8::Local<v8::Object> thatObject();

        Engine* engine() const;

    private:
        void* internalField() const;

        v8::FunctionCallbackInfo<v8::Value> const* m_info;
    };

    // Parameters class
    // ----------------

    class LV_ELEMENTS_EXPORT Parameters{

        friend class Callable;

    public:
        Parameters(int length);
        Parameters(const std::initializer_list<LocalValue>& init);
        ~Parameters();

        void assign(int index, const LocalValue& ref);
        void assign(int index, const v8::Local<v8::Value>& value);

        LocalValue at(Engine* engine, int index) const;
        int length() const;

        template<int index, typename T> T getValue(Engine* engine) const{
            return extractValue<T>(engine, m_args, index);
        }

        template<typename T> static T extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);

    private:
        DISABLE_COPY(Parameters);

        int m_length;
        v8::Local<v8::Value>* m_args;
    };

public:
    static void assignReturnValue(const el::LocalValue& val, const v8::FunctionCallbackInfo<v8::Value>& info);

    static void ptrImplementation(const v8::FunctionCallbackInfo<v8::Value>& info){
        Function::CallInfo pr(&info);
        Function* f = reinterpret_cast<Function*>(pr.userData());
        Function::assignReturnValue(f->call(pr), info);
    }

    template<class T>
    static void methodPtrImplementation(const v8::FunctionCallbackInfo<v8::Value>& info){
        Function::CallInfo pr(&info);
        Function* f = reinterpret_cast<Function*>(pr.userData());
        Function::assignReturnValue(f->call(pr), info);
    }

    template<typename RT, typename ...Args, int... Indexes>
    static LocalValue callFunction(
            Engine* engine,
            const std::function<RT(Args ...args)>& f,
            meta::IndexTuple<Indexes...>,
            const Function::CallInfo& params)
    {
        return LocalValue(
            engine,
            f(params.template getValue<Indexes,
                 typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...));
    }

    template<typename ...Args, int... Indexes>
    static void callVoidFunction(
            const std::function<void(Args ...args)>& f,
            meta::IndexTuple<Indexes...>,
            const Function::CallInfo& params)
    {
        f(params.template getValue<Indexes,
             typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
    }

    template<typename C, typename RT, typename ...Args, int... Indexes>
    static LocalValue callMethod(
            Engine* engine,
            const std::function<RT(C*, Args ...args)>& f,
            meta::IndexTuple<Indexes...>,
            const Function::CallInfo& params)
    {
        return LocalValue(
            engine,
            f(params.template that<C>(),
              params.template getValue<Indexes,
                  typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...));
    }

    template<typename C, typename RT, typename ...Args, int... Indexes>
    static void callVoidMethod(
            const std::function<RT(C*, Args ...args)>& f,
            meta::IndexTuple<Indexes...>,
            const Function::CallInfo& params)
    {
        f(params.template that<C>(),
          params.template getValue<Indexes,
              typename std::remove_const<typename std::remove_reference<Args>::type>::type>()...);
    }

public:
    size_t totalArguments() const{ return m_totalArguments; }
    Function* less() const { return m_less; }

    LocalValue call(const Function::CallInfo& params){
        if ( params.length() >= totalArguments() )
            return m_unwrapFunction(params);
        else if ( params.length() < totalArguments() && less() )
            return less()->call(params);
        else {
            throw std::exception(); // Argument does not fit the count
        }
    }


    FunctionPointer ptr(){ return m_ptr; }
    void assignLess(Function* f){ m_less = f;}

protected:
    std::function<LocalValue(const Function::CallInfo& params)> m_unwrapFunction;

    size_t          m_totalArguments;
    FunctionPointer m_ptr;
    Function*       m_less;

};

template<> bool Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> Value::Int32 Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> Value::Int64 Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> Value::Number Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> std::string Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> Callable Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> Object Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> Buffer Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> Value Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> LocalValue Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);
template<> Element* Function::CallInfo::extractValue(v8::FunctionCallbackInfo<v8::Value> const* info, int index);


template<typename T> T Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index){
    return static_cast<T>(Function::Parameters::extractValue<Element*>(engine, args, index));
}

template<> bool Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> Value::Int32 Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> Value::Int64 Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> Value::Number Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> std::string Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> Callable Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> Object Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> Buffer Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> Value Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> LocalValue Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);
template<> Element* Function::Parameters::extractValue(Engine* engine, v8::Local<v8::Value> const* args, int index);


template<typename RT, typename ...Args>
class PointerFunction : public Function{

public:
    typedef RT(*FunctionType)(Args...);
    using ReturnType = typename std::remove_const<typename std::remove_reference<RT>::type>::type;

    static_assert(
        std::is_same<ReturnType, bool>::value ||
        std::is_same<ReturnType, Value::Int32>::value ||
        std::is_same<ReturnType, Value::Int64>::value ||
        std::is_same<ReturnType, Value::Number>::value ||
        std::is_same<ReturnType, std::string>::value ||
        std::is_same<ReturnType, Callable>::value ||
        std::is_same<ReturnType, Buffer>::value ||
        std::is_same<ReturnType, Object>::value ||
        std::is_same<ReturnType, LocalValue>::value ||
        std::is_same<ReturnType, Value>::value ||
        std::is_pointer<ReturnType>::value,
        "Return type is not convertible to script value."
    );

public:
    PointerFunction(const std::function<RT(Args...)>& f)
        : m_function(f)
    {
        m_less = nullptr;
        m_ptr = &Function::ptrImplementation;
        m_totalArguments  = sizeof...(Args);
        m_unwrapFunction = [this](const Function::CallInfo& params){
            return Function::callFunction(
                params.engine(), m_function, typename meta::make_indexes<Args...>::type(), params
            );
        };
    }


    RT callArgs(Args... args){
        return m_function(args...);
    }

private:
    std::function<RT(Args ...args)> m_function;
};

template<typename ...Args>
class PointerFunction<void, Args...> : public Function{

public:
    PointerFunction(const std::function<void(Args...)>& f)
        : m_function(f)
    {
        m_less           = nullptr;
        m_ptr            = &Function::ptrImplementation;
        m_totalArguments = sizeof...(Args);
        m_unwrapFunction = [this](const Function::CallInfo& params){
            Function::callVoidFunction(
                m_function, typename meta::make_indexes<Args...>::type(), params
            );
            return LocalValue(params.engine());
        };
    }


    void callArgs(Args... args){
        m_function(args...);
    }

private:
    std::function<void(Args ...args)> m_function;
};

template<typename RT>
class PointerFunction<RT, const Function::CallInfo&> : public Function{

public:
    typedef RT(*FunctionType)(const Function::CallInfo&);
    using ReturnType = typename std::remove_const<typename std::remove_reference<RT>::type>::type;

    static_assert(
        std::is_same<ReturnType, Value::Int32>::value ||
        std::is_same<ReturnType, Value::Int64>::value ||
        std::is_same<ReturnType, Value::Number>::value ||
        std::is_same<ReturnType, std::string>::value ||
        std::is_same<ReturnType, Callable>::value ||
        std::is_same<ReturnType, Buffer>::value ||
        std::is_same<ReturnType, Object>::value ||
        std::is_same<ReturnType, LocalValue>::value ||
        std::is_same<ReturnType, Value>::value ||
        std::is_pointer<ReturnType>::value,
        "Return type is not convertible to script value."
    );

public:
    PointerFunction(const std::function<RT(const Function::CallInfo&)>& f)
        : m_function(f)
    {
        m_less = nullptr;
        m_ptr = &Function::ptrImplementation;
        m_totalArguments  = 1;
        m_unwrapFunction = [this](const Function::CallInfo& params){
            return LocalValue(params.engine(), m_function(params));
        };
    }


    RT callArgs(const Function::CallInfo& p){ return m_function(p); }

private:
    std::function<RT(const Function::CallInfo&)> m_function;
};

template<>
class PointerFunction<void, const Function::CallInfo&> : public Function{

public:
    PointerFunction(const std::function<void(const Function::CallInfo&)>& f)
        : m_function(f)
    {
        m_less           = nullptr;
        m_ptr            = &Function::ptrImplementation;
        m_totalArguments = 1;
        m_unwrapFunction = [this](const Function::CallInfo& params){
            m_function(params);
            return LocalValue(params.engine());
        };
    }


    void callArgs(const Function::CallInfo& p){ m_function(p);}

private:
    std::function<void(const Function::CallInfo&)> m_function;
};


}}// namespace lv, script

#endif // LVFUNCTION_H