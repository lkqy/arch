#pragma once
namespace knitter {

template <typename T>
class Singleton {
public:
    static T& get() {
        static T instance;
        return instance;
    }

private:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

}  // end of namespace knitter