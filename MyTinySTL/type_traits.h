// 这个头文件用于提取类型信息
// use standard header for type_traits
#ifndef STL_STUDY_TYPE_TRAITS_H
#define STL_STUDY_TYPE_TRAITS_H
#include <type_traits>
namespace mystl
{
    template<class T,T v>
    struct m_integral_constant
    {
        static constexpr T value=v;
    };
    template<bool b>
    using m_bool_constant=m_integral_constant<bool,b>;

    typedef m_bool_constant<true> m_true_type;
    typedef m_bool_constant<false> m_false_type;


    template<class T1,class T2>
    struct pair;
    template<class T1>
    struct is_pair:mystl::m_false_type {};
    template<class T1,class T2>
    struct is_pair<mystl::pair<T1,T2>>:mystl::m_true_type {};

}

#endif //STL_STUDY_TYPE_TRAITS_H
