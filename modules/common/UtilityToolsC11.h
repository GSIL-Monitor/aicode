#ifndef _UTILITY_C11_
#define _UTILITY_C11_

#include <map>

template <typename T>
class SingletonC11
{
public:
    template <typename... Args>
    static T *CreateInstance(Args&&... args);
    
    static T *GetInstance();

    static void DestroyInstance();


private:
    SingletonC11();
    virtual ~SingletonC11();
    SingletonC11(const SingletonC11&);
    SingletonC11& operator = (const SingletonC11&);

private:
    static T *m_pInstance;
};

template <typename T>
T * SingletonC11<T>::m_pInstance = nullptr;

template<typename T>
template<typename ...Args>
inline T * SingletonC11<T>::CreateInstance(Args && ...args)
{
    if (nullptr == m_pInstance)
    {
        m_pInstance = new T(std::forward<Args>(args)...);
    }

    return m_pInstance;
}


template <typename T>
T * SingletonC11<T>::GetInstance()
{
    return m_pInstance;
}

template <typename T>
void SingletonC11<T>::DestroyInstance()
{
    delete m_pInstance;
    m_pInstance = nullptr;
}


class NonCopyable
{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable & operator = (const NonCopyable&) = delete;
};

template<typename Func>
class Events : NonCopyable
{
public:
    Events() {};
    ~Events() {};

    int Register(const Func &f)
    {
        return Assign(f);
    }

    int Register(Func &&f)
    {
        return Assign(f);
    }

    void UnRegister(int iRid)
    {
        m_FnMap.erase(iRid);
    }

    template<typename... Args>
    void Notify(Args&&... args)
    {
        for (auto it : m_FnMap)
        {
            it.second(std::forward<Args>(args)...);
        }
    }
    
private:
    template<typename F>
    int Assign(F &&f)
    {
        m_FnMap.emplace(++m_Rid, std::forward<F>(f));
        return m_Rid;
    }

    int m_Rid = 0;
    std::map<int, Func> m_FnMap;
};



#endif


