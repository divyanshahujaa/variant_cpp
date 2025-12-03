#include <cstddef>
#include <typeinfo>
#include <utility>

template  <typename T, typename... Args>
struct variant_helper{
    static const size_t size = sizeof(T)>variant_helper<Args...>::size ? sizeof(T) : variant_helper<Args...>::size;

    inline static void destroy(size_t id, void* data){
        if(id==typeid(T).hash_code()){
            reinterpret_cast<T*>(data)->~T();
        }
        else{
            variant_helper<Args...>::destroy(id, data);
        }
    }
};

template  <typename T>
struct variant_helper<T>{
    static const size_t size = sizeof(T);

    inline static void destroy(size_t id, void* data){
        if(id==typeid(T).hash_code()){
            reinterpret_cast<T*>(data)->~T();
        }
    }
};


template<size_t size>
struct raw_data{
    char data[size];
    // type type_id;
};


template<typename T, typename... Args>
struct variant{
    static inline size_t invalid_type() {
        return typeid(void).hash_code();
    }

    typedef variant_helper<T, Args...> helper;
    size_t type_id;
    raw_data<helper::size> data;

    variant() : type_id(invalid_type()) { }

    variant(variant<T, Args...>&& old) : type_id(old.type_id), data(old.data)
    {
        old.type_id = invalid_type();
    }

    template<typename F, typename... Ts>
    void set(Ts&&... args)
    {
        // We must first destroy what is currently contained within
        helper::destroy(type_id, &data);

        new (&data) F(std::forward<Ts>(args)...);
        type_id = typeid(F).hash_code();
    }

    template<typename F>
    F& get()
    {
        if (type_id == typeid(F).hash_code())
        return *reinterpret_cast<F*>(&data);
        else
        throw std::bad_cast();
    }
};

