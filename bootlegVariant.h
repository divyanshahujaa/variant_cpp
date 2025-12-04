#include <cstddef>
#include <typeinfo>
#include <utility>

template  <typename     T, typename... Args>
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

    inline static void copy(size_t old_t, const void * old_v, void * new_v)
    {
        if (old_t == typeid(T).hash_code())
        new (new_v) T(*reinterpret_cast<const T*>(old_v));
        else
        variant_helper<Args...>::copy(old_t, old_v, new_v);
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

    inline static void copy(size_t old_t, const void * old_v, void * new_v)
    {
        if (old_t == typeid(T).hash_code())
        new (new_v) T(*reinterpret_cast<const T*>(old_v));
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

    variant<T, Args...>() : type_id(invalid_type()) { }

    variant(variant<T, Args...>&& old) : type_id(old.type_id), data(old.data)
    {
        old.type_id = invalid_type();
    }

    variant(const variant<T, Args...>& old) : type_id(old.type_id)
    {
        // We need to do a "deep" copy, that is - invoke the copy constructor of the contained type.
        helper::copy(old.type_id, &old.data, &data);
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

    variant<T, Args...>& operator= (variant<T, Args...>&& old){
        data = old.data;
        old.type_id = invalid_type();
        return *this;
    }

    variant<T, Args...>& operator= (variant<T, Args...> old)
    {
        // Note the call-by-value above
        std::swap(data, old.data);
        std::swap(type_id, old.type_id);

        return *this;
    }

    template<typename Tn>
    bool is() {
        return (type_id == typeid(Tn).hash_code());
    }

    bool valid() {
       return (type_id != invalid_type());
    }
    
    ~variant() { helper::destroy(type_id, &data); }
};
