//
// Created by 11048 on 2023/6/6.
//
// ����ļ�����һЩͨ�ù��ߣ����� move, forward, swap �Ⱥ������Լ� pair ��

#ifndef STL_STUDY_UTIL_N_H
#define STL_STUDY_UTIL_N_H

#include <cstddef>
#include "type_traits.h"

namespace mystl
{
    //move
    template<class T>
    typename std::remove_reference<T>::type && move(T&& arg)noexcept
    {
        return static_cast<typename std::remove_reference<T>::type&&>(arg);
    }

    //forward
    template<class T>
    T&& forward(typename std::remove_reference<T>::type& arg)noexcept
    {
        return static_cast<T&&>(arg);
    }

    template <class T>
    T&& forward(typename std::remove_reference<T>::type&& arg) noexcept
    {
        static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
        return static_cast<T&&>(arg);
    }

    //swap
    template<class Tp>
    void swap(Tp& lhs,Tp& rhs)
    {
        auto tmp(mystl::move(lhs));
        lhs=mystl::move(rhs);
        rhs=mystl::move(tmp);
    }

    template<class ForwardIter1,class ForwardIter2>
    ForwardIter2 swap_range(ForwardIter1 first1,ForwardIter1 last1,ForwardIter2 first2)
    {
        for(;first1!=last1;++first1,(void)++first2)
        {
            mystl::swap(*first1,*first2);
        }
        return first2;
    }

    template<class Tp,size_t N>
    void swap(Tp(&a)[N],Tp(&b)[N])
    {
        mystl::swap_range(a,a+N,b);
    }

    // --------------------------------------------------------------------------------------
    // pair

    // �ṹ��ģ�� : pair
    // ����ģ������ֱ��ʾ�������ݵ�����
    // �� first �� second ���ֱ�ȡ����һ�����ݺ͵ڶ�������
    /*
     *��δ��붨����һ������Ĭ��ģ������� pair ���Ĭ�Ϲ��캯�����ù��캯������ Other1 �� Other2 ������Ĭ�Ϲ��캯��ʱ�ſ��á���ʹ�ó�Ա��ʼ���б���ʼ�� first �� second ��Ա���Ӷ�ʵ����һ���ɸ���Ĭ��ģ���������Ĭ�ϳ�ʼ���Ĺ��캯����
     */
    template<class Ty1,class Ty2>
    struct pair
    {
        typedef Ty1 first_type;
        typedef Ty2 second_type;
        first_type first;
        second_type second;

        // default constructiable
        template <class Other1 = Ty1, class Other2 = Ty2,
                typename = typename std::enable_if<
                        std::is_default_constructible<Other1>::value &&
                        std::is_default_constructible<Other2>::value, void>::type>
        constexpr pair()
                : first(), second()
        {
        }

        // implicit constructiable for this type
        template <class U1 = Ty1, class U2 = Ty2,
                typename std::enable_if<
                        std::is_copy_constructible<U1>::value &&
                        std::is_copy_constructible<U2>::value &&
                        std::is_convertible<const U1&, Ty1>::value &&
                        std::is_convertible<const U2&, Ty2>::value, int>::type = 0>
        constexpr pair(const Ty1& a, const Ty2& b)
                : first(a), second(b)
        {
        }

        // explicit constructible for this type
        template <class U1 = Ty1, class U2 = Ty2,
                typename std::enable_if<
                        std::is_copy_constructible<U1>::value &&
                        std::is_copy_constructible<U2>::value &&
                        (!std::is_convertible<const U1&, Ty1>::value ||
                         !std::is_convertible<const U2&, Ty2>::value), int>::type = 0>
        explicit constexpr pair(const Ty1& a, const Ty2& b)
                : first(a), second(b)
        {
        }

        pair(const pair& rhs) = default;
        pair(pair&& rhs) = default;

        // implicit constructiable for other type
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, Other1>::value &&
                        std::is_constructible<Ty2, Other2>::value &&
                        std::is_convertible<Other1&&, Ty1>::value &&
                        std::is_convertible<Other2&&, Ty2>::value, int>::type = 0>
        constexpr pair(Other1&& a, Other2&& b)
                : first(mystl::forward<Other1>(a)),
                  second(mystl::forward<Other2>(b))
        {
        }

        // explicit constructiable for other type
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, Other1>::value &&
                        std::is_constructible<Ty2, Other2>::value &&
                        (!std::is_convertible<Other1, Ty1>::value ||
                         !std::is_convertible<Other2, Ty2>::value), int>::type = 0>
        explicit constexpr pair(Other1&& a, Other2&& b)
                : first(mystl::forward<Other1>(a)),
                  second(mystl::forward<Other2>(b))
        {
        }

        // implicit constructiable for other pair
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, const Other1&>::value &&
                        std::is_constructible<Ty2, const Other2&>::value &&
                        std::is_convertible<const Other1&, Ty1>::value &&
                        std::is_convertible<const Other2&, Ty2>::value, int>::type = 0>
        constexpr pair(const pair<Other1, Other2>& other)
                : first(other.first),
                  second(other.second)
        {
        }

        // explicit constructiable for other pair
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, const Other1&>::value &&
                        std::is_constructible<Ty2, const Other2&>::value &&
                        (!std::is_convertible<const Other1&, Ty1>::value ||
                         !std::is_convertible<const Other2&, Ty2>::value), int>::type = 0>
        explicit constexpr pair(const pair<Other1, Other2>& other)
                : first(other.first),
                  second(other.second)
        {
        }

        // implicit constructiable for other pair
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, Other1>::value &&
                        std::is_constructible<Ty2, Other2>::value &&
                        std::is_convertible<Other1, Ty1>::value &&
                        std::is_convertible<Other2, Ty2>::value, int>::type = 0>
        constexpr pair(pair<Other1, Other2>&& other)
                : first(mystl::forward<Other1>(other.first)),
                  second(mystl::forward<Other2>(other.second))
        {
        }

        // explicit constructiable for other pair
        template <class Other1, class Other2,
                typename std::enable_if<
                        std::is_constructible<Ty1, Other1>::value &&
                        std::is_constructible<Ty2, Other2>::value &&
                        (!std::is_convertible<Other1, Ty1>::value ||
                         !std::is_convertible<Other2, Ty2>::value), int>::type = 0>
        explicit constexpr pair(pair<Other1, Other2>&& other)
                : first(mystl::forward<Other1>(other.first)),
                  second(mystl::forward<Other2>(other.second))
        {
        }

        //copy assign for this pair
        pair& operator=(const pair& rhs)
        {
            if(this!=&rhs)
            {
                first=rhs.first;
                second=rhs.second;
            }
            return *this;
        }

        // move assign for this pair
        pair& operator=(pair&& rhs)noexcept
        {
            if(this!=&rhs)
            {
                first=mystl::move(rhs.first);
                second=mystl::move(rhs.second);
            }
            return *this;
        }

        // copy assign for other pair
        template<class Other1,class Other2>
        pair& operator=(const pair<Other1,Other2>& other)
        {
            first=other.first;
            second=other.second;
            return *this;
        }

        //move assign for other pair
        template<class Other1,class Other2>
        pair &operator=(pair<Other1,Other2>&& other)
        {
            first=mystl::forward<Other1>(other.first);
            second=mystl::forward<Other2>(other.second);
            return *this;
        }

        ~pair()=default;
        void swap(pair& other)
        {
            if(this!=other)
            {
                mystl::swap(first,other.first);
                mystl::swap(second,other.second);
            }
        }
    };


    //���رȽϲ�����
    template<class Ty1,class Ty2>
    bool operator==(const pair<Ty1,Ty2>& lhs,const pair<Ty1,Ty2>& rhs)
    {
        return lhs.first==rhs.first&&lhs.second==rhs.second;
    }

    template<class Ty1,class Ty2>
    bool operator<(const pair<Ty1,Ty2>& lhs,const pair<Ty1,Ty2>& rhs)
    {
        return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
    }

    template<class Ty1,class Ty2>
    bool operator!=(const pair<Ty1,Ty2>& lhs,const pair<Ty1,Ty2>& rhs)
    {
        return !(lhs==rhs);
    }

    template <class Ty1, class Ty2>
    bool operator>(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return rhs < lhs;
    }

    template <class Ty1, class Ty2>
    bool operator<=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class Ty1, class Ty2>
    bool operator>=(const pair<Ty1, Ty2>& lhs, const pair<Ty1, Ty2>& rhs)
    {
        return !(lhs < rhs);
    }

    // ���� mystl �� swap
    template<class Ty1,class Ty2>
    void swap(pair<Ty1,Ty2>& lhs,pair<Ty1,Ty2>& rhs)
    {
        lhs.swap(rhs);
    }

    // ȫ�ֺ��������������ݳ�Ϊһ�� pair
    template<class Ty1,class Ty2>
    pair<Ty1,Ty2> make_pair(Ty1&& first,Ty2 && second)
    {
        return pair<Ty1,Ty2>(mystl::forward<Ty1>(first),mystl::forward<Ty2>(second));
    }
}

#endif //STL_STUDY_UTIL_N_H