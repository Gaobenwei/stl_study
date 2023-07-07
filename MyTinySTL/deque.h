//
// Created by 11048 on 2023/6/12.
//
// ���ͷ�ļ�������һ��ģ���� deque
// deque: ˫�˶���
/*
 *   //����Ԫ��
        template<class ...Args>
        iterator insert_aux(iterator pos,Args&& ...args);
        void fill_insert(iterator pos,size_type n,const value_type& value);
        template<class IIter>
        void copy_insert(iterator pos,IIter first,IIter last,size_type n);

        template<class IIter>
        void insert_dispatch(iterator pos,IIter first,IIter last,input_iterator_tag);

        template<class FIter>
        void insert_dispatch(iterator pos,FIter first,FIter last,forward_iterator_tag);

        //reallocate ��map�ռ䲻��ʱ����������map�ռ�
        void require_capacity(size_type n, bool front); // nΪ��Ҫ��������С
        void reallocate_map_at_front(size_type need); // ��map��ǰ���������ÿռ�
        void reallocate_map_at_back(size_type need); // ��map��β���������ÿռ�
        void reallocate_map(size_type need, bool front); // ��������map�ռ�
���Ϻ������߼�������
 */
// notes:
//
// �쳣��֤��
// mystl::deque<T> ��������쳣��֤�����ֺ������쳣��֤���������µȺ�����ǿ�쳣��ȫ��֤��
//   * emplace_front
//   * emplace_back
//   * emplace
//   * push_front
//   * push_back
//   * insert
#ifndef STL_STUDY_DEQUE_H
#define STL_STUDY_DEQUE_H

#include <initializer_list>
#include "iterator.h"
#include "memory.h"
#include "util.h"
#include "exceptdef.h"

namespace mystl
{
#ifdef max
    #pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
    #pragma message("#undefing marco min")
#undef min
#endif // min

    // deque map ��ʼ���Ĵ�С
#ifndef DEQUE_MAP_INIT_SIZE
#define DEQUE_MAP_INIT_SIZE 8
#endif

    template<class T>
    struct deque_buf_size
    {
        static constexpr size_t value=sizeof (T)<256?4096/sizeof (T):16;
    };

    // deque �ĵ��������
    template<class T,class Ref,class Ptr>
    struct deque_iterator:public iterator<random_access_iterator_tag,T>
    {
        typedef deque_iterator<T,T&,T*>         iterator;
        typedef deque_iterator<T,const T&,const T*>         const_iterator;
        typedef deque_iterator              self;


        typedef T            value_type;
        typedef Ptr          pointer;
        typedef Ref          reference;
        typedef size_t       size_type;
        typedef ptrdiff_t    difference_type;
        typedef T*           value_pointer;
        typedef T**         map_pointer;

        static const size_type buffer_size=deque_buf_size<T>::value;

        // ������������Ա����
        value_pointer cur; //ָ�����ڻ������ĵ�ǰԪ��
        value_pointer first;//ָ�����ڻ�������ͷ��
        value_pointer last;// ָ�����ڻ�������β��
        map_pointer  node;  // ���������ڽڵ�

        // ���졢���ơ��ƶ�����
        deque_iterator() noexcept
                :cur(nullptr), first(nullptr), last(nullptr), node(nullptr) {}

        deque_iterator(value_pointer v, map_pointer n):cur(v),first(*n),last(*n+buffer_size),node(n){}

        deque_iterator(const iterator & rhs):cur(rhs.cur),first(rhs.first),last(rhs.last), node(rhs.node){}

        deque_iterator(iterator && rhs)noexcept:cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node)
        {
            rhs.cur = nullptr;
            rhs.first = nullptr;
            rhs.last = nullptr;
            rhs.node = nullptr;
        }

        deque_iterator(const const_iterator &rhs):cur(rhs.cur), first(rhs.first), last(rhs.last), node(rhs.node){}

        self operator=(const iterator &rhs)
        {
            if(this!=&rhs)
            {
                cur = rhs.cur;
                first = rhs.first;
                last = rhs.last;
                node = rhs.node;
            }
            return *this;
        }

        // ת����һ��������
        void set_node(map_pointer new_node)
        {
            node=new_node;
            first=new_node;
            last=first+buffer_size;
        }

        // ���������
        reference operator*()const{return *cur;}
        pointer operator->()const{return cur;}

        difference_type operator-(const self& x)const
        {
            return static_cast<difference_type>(buffer_size)*(node-x.node)+(cur-first)-(x.cur-x.first);
        }

        self & operator++()
        {
            ++cur;
            if(cur==last)
            {
                // ������ﻺ��
                set_node(node+1);
                cur=first;
            }
            return *this;
        }

        self operator++(int )
        {
            self tmp=*this;
            ++*this;
            return tmp;
        }

        self & operator--()
        {
            if(cur==first)
            {
                // ������ﻺ������ͷ
                set_node(node-1);
                cur=last;
            }
            --cur;
            return *this;
        }

        self operator--(int)
        {
            self tmp=*this;
            --*this;
            return tmp;
        }

        self &operator+=(difference_type n)
        {
            const auto offset=n+(cur-first);
            if(offset>=0 && offset<static_cast<difference_type>(buffer_size))
            {
                // ���ڵ�ǰ������
                cur+=n;
            } else
            {
                //��Ҫ��ת������������
                const auto node_offset=offset>0?offset / static_cast<difference_type>(buffer_size):-static_cast<difference_type>((-offset - 1) / buffer_size) - 1;
                set_node(node+node_offset);
                cur=first+(offset-node_offset*static_cast<difference_type>(buffer_size));
            }
            return *this;
        }

        self operator+(difference_type n)const
        {
            self tmp=*this;
            return tmp+=n;
        }

        self & operator-=(difference_type n)
        {
            return *this+=-n;
        }

        self operator-(difference_type n)const
        {
            self tmp=*this;
            return tmp-=n;
        }

        reference operator[](difference_type n)const{return *(*this+n);}

        // ���رȽϲ�����
        bool operator==(const self& rhs)const{return cur==rhs.cur;}
        bool operator<(const self& rhs)const
        {
            return node==rhs.node?(cur<rhs.cur):(node<rhs.node);
        }
        bool operator!=(const self& rhs)const{return !(*this==rhs);}
        bool operator> (const self& rhs) const { return rhs < *this; }
        bool operator<=(const self& rhs) const { return !(rhs < *this); }
        bool operator>=(const self& rhs) const { return !(*this < rhs); }

    };

    // ģ���� deque
    // ģ�����������������
    template<class T>
    class deque
    {
public:
        // deque ��Ƕ���ͱ���
        typedef mystl::allocator<T> allocator_type;
        typedef mystl::allocator<T> data_allocator;
        typedef mystl::allocator<T*> map_allocator;

        typedef typename allocator_type::value_type      value_type;
        typedef typename allocator_type::pointer         pointer;
        typedef typename allocator_type::const_pointer   const_pointer;
        typedef typename allocator_type::reference       reference;
        typedef typename allocator_type::const_reference const_reference;
        typedef typename allocator_type::size_type       size_type;
        typedef typename allocator_type::difference_type difference_type;

        typedef pointer* map_pointer;
        typedef const_pointer* const_map_pointer;

        typedef deque_iterator<T,T&,T*>         iterator;
        typedef deque_iterator<T,const T&,const T*>         const_iterator;
        typedef mystl::reverse_iterator<iterator> reverse_iterator;
        typedef mystl::reverse_iterator<const_iterator> const_reverse_iterator;

        allocator_type get_allocator()const{return allocator_type();}

        static const size_type buffer_size=deque_buf_size<T>::value;

private:
        //�������ĸ����ݳ�Ա������һ��deque
        iterator  begin_; // ���ֵ�һ���ڵ�
        iterator  end_;   // �������һ���ڵ�
        map_pointer map_; // ָ��map��map�ǿ������ռ䣬ÿ��Ԫ�ض���һ��ָ�룬ָ��һ��������
        size_type map_size_; // map���ж���ָ��
    public:
        // ���졢���ơ��ƶ�����������
        deque() { fill_init(0, value_type()); }

        explicit deque(size_type n) { fill_init(n, value_type()); }

        deque(size_type n, const value_type& value) { fill_init(n, value); }

        template<class IIter,typename std::enable_if<
                mystl::is_input_iterator<IIter>::value, int>::type = 0>
        deque(IIter first, IIter last)
        {
            copy_init(first, last, iterator_category(first));
        }

        deque(std::initializer_list<T> ilist)
        {
            copy_init(ilist.begin(), ilist.end(), mystl::forward_iterator_tag());
        }

        deque(const deque& rhs)
        {
            copy_init(rhs.begin(), rhs.end(), mystl::forward_iterator_tag());
        }

        deque(deque&& rhs) noexcept
                :begin_(mystl::move(rhs.begin_)),
                 end_(mystl::move(rhs.end_)),
                 map_(rhs.map_),
                 map_size_(rhs.map_size_)
        {
            rhs.map_=nullptr;
            rhs.map_size_=0;
        }

        deque& operator=(const deque& rhs)
        {
            if(this!=&rhs)
            {
                const auto len=size();
                if(len>=rhs.size())
                {
                    erase(mystl::copy(rhs.begin(),rhs.end(),begin_),end_);
                } else
                {
                    const_iterator mid=rhs.begin()+static_cast<difference_type>(len);
                    mystl::copy(rhs.begin(),mid,begin_);
                    insert(end_,mid,rhs.end());
                }
            }
            return *this;
        }

        deque& operator=(deque&& rhs) noexcept
        {
            clear();
            begin_=mystl::move(rhs.begin_);
            end_=mystl::move(rhs.end_);
            map_=rhs.map_;
            map_size_=rhs.map_size_;
            rhs.map_=nullptr;
            rhs.map_size_=0;
            return *this;
        }

        deque& operator=(std::initializer_list<T> ilist)
        {
            deque tmp(ilist.begin(),ilist.end());
            swap(tmp);
            return *this;
        }

        ~deque()
        {
            if(map_!=nullptr)
            {
                clear();
                data_allocator::deallocate(*begin_.node,buffer_size);
                *begin_.node=nullptr;
                map_allocator::deallocate(map_,map_size_);
                map_=nullptr;
                map_size_=0;
            }
        }

    public:
        // ��������ز���
        iterator               begin()        { return begin_; }
        const_iterator         begin()  const { return begin_; }
        iterator               end()          { return end_; }
        const_iterator         end()    const { return end_; }

        reverse_iterator       rbegin()       { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
        reverse_iterator       rend()         { return reverse_iterator(begin()); }
        const_reverse_iterator rend()   const { return const_reverse_iterator(begin()); }

        const_iterator         cbegin() const { return begin(); }
        const_iterator         cend()   const { return end(); }
        const_reverse_iterator crbegin()const { return rbegin(); }
        const_reverse_iterator crend()  const { return rend(); }

        // ������ز���
        bool      empty()    const { return begin_ == end_; }
        size_type size()     const { return end_ - begin_; }
        size_type max_size() const { return static_cast<size_type>(-1); }
        void     resize(size_type new_size) { resize(new_size, value_type()); }
        void     resize(size_type new_size, const value_type& value);
        void    shrink_to_fit()noexcept;

        // ����Ԫ����ز���
        reference       operator[](size_type n)       {
            MYSTL_DEBUG(n<size());
            return begin_[n]; }

        const_reference operator[](size_type n) const {
            MYSTL_DEBUG(n<size());
            return begin_[n];
        }

        reference       at(size_type n)
        {
            THROW_OUT_OF_RANGE_IF(!(n<size()),"deque<T>::at() subscript out of range");
            return (*this)[n];
        }

        const_reference at(size_type n) const
        {
            THROW_OUT_OF_RANGE_IF(!(n<size()),"deque<T>::at() subscript out of range");
            return (*this)[n];
        }

        reference front()
        {
            MYSTL_DEBUG(!empty());
            return *begin();
        }

        const_reference front()const
        {
            MYSTL_DEBUG(!empty());
            return *begin();
        }

        reference back()
        {
            MYSTL_DEBUG(!empty());
            return *(end()-1);
        }

        const_reference back()const
        {
            MYSTL_DEBUG(!empty());
            return *(end()-1);
        }

        // �޸�������ز���

        // assign
        void assign(size_type n, const value_type& value)
        {
            fill_assign(n,value);
        }

        template<class IIter,typename std::enable_if<
                mystl::is_input_iterator<IIter>::value,int>::type=0>
        void assign(IIter first,IIter last)
        {
            copy_assign(first,last,mystl::iterator_category(first));
        }

        void assign(std::initializer_list<T> ilist)
        {
            copy_assign(ilist.begin(),ilist.end(),mystl::forward_iterator_tag());
        }

        // emplace_front ��ͷ���͵ع���Ԫ��
        template<class ...Args>
        void emplace_front(Args&& ...args);

        // emplace_back ��β���͵ع���Ԫ��
        template<class ...Args>
        void emplace_back(Args&& ...args);

        // emplace ��ָ��λ�þ͵ع���Ԫ��
        template<class ...Args>
        iterator emplace(const_iterator pos,Args&& ...args);

        // push_front ��ͷ������Ԫ��
        void push_front(const value_type& value);
        void push_front(value_type&& value)
        {
            emplace_front(mystl::move(value));
        }

        // push_back ��β������Ԫ��
        void push_back(const value_type& value);
        void push_back(value_type&& value)
        {
            emplace_back(mystl::move(value));
        }

        // pop_front ɾ��ͷ��Ԫ��
        void pop_front();

        // pop_back ɾ��β��Ԫ��
        void pop_back();

        // insert ��ָ��λ�ò���Ԫ��
        iterator insert(iterator pos,const value_type& value);
        iterator insert(iterator pos,value_type&& value);
        void    insert(iterator pos,size_type n,const value_type& value);

        template<class IIter,typename std::enable_if<
                mystl::is_input_iterator<IIter>::value,int>::type=0>
        void insert(iterator pos,IIter first,IIter last)
        {
            insert_dispatch(pos,first,last,mystl::iterator_category(first));
        }

        //erase ɾ��ָ��λ�õ�Ԫ��
        iterator erase(iterator pos);
        iterator erase(iterator first,iterator last);

        //clear �������Ԫ��
        void clear() noexcept;

        //swap ��������deque
        void swap(deque& rhs) noexcept;

    private:
        //helpful function
        // ����һ��map��ʹ��ӵ��n���ڵ�
        map_pointer create_map(size_type size);
        void        create_buffer(map_pointer nstart, map_pointer nfinish);
        void        destroy_buffer(map_pointer nstart, map_pointer nfinish);

        // �����ķ�ʽ�����ڵ�
        void fill_init(size_type n, const value_type& value);
        void map_init(size_type n);

        template<class IIter>
        void copy_init(IIter first, IIter last, mystl::input_iterator_tag);

        template<class FIter>
        void copy_init(FIter first, FIter last, mystl::forward_iterator_tag);

        // �����ķ�ʽ��ֵ
        void fill_assign(size_type n, const value_type& value);
        // �Ը��Ƶķ�ʽ��ֵ
        template<class IIter>
        void copy_assign(IIter first, IIter last, mystl::input_iterator_tag);

        template<class FIter>
        void copy_assign(FIter first, FIter last, mystl::forward_iterator_tag);

        //����Ԫ��
        template<class ...Args>
        iterator insert_aux(iterator pos,Args&& ...args);
        void fill_insert(iterator pos,size_type n,const value_type& value);
        template<class IIter>
        void copy_insert(iterator pos,IIter first,IIter last,size_type n);

        template<class IIter>
        void insert_dispatch(iterator pos,IIter first,IIter last,input_iterator_tag);

        template<class FIter>
        void insert_dispatch(iterator pos,FIter first,FIter last,forward_iterator_tag);

        //reallocate ��map�ռ䲻��ʱ����������map�ռ�
        void require_capacity(size_type n, bool front); // nΪ��Ҫ��������С
        void reallocate_map_at_front(size_type need); // ��map��ǰ���������ÿռ�
        void reallocate_map_at_back(size_type need); // ��map��β���������ÿռ�
        void reallocate_map(size_type need, bool front); // ��������map�ռ�

    };

    /*******************************************************/

    //����������С
    template<class T>
    void deque<T>::resize(size_type new_size, const value_type& value)
    {
        const auto len = size();
        if(new_size < len)
            erase(begin()+new_size,end());
        else
            insert(end(),new_size-len,value);
    }

    //��С��������
    template<class T>
    void deque<T>::shrink_to_fit()noexcept
    {
        //��������ͷ��������
        for(auto cur=map_;cur<begin_.node;++cur)//�ͷ�ǰ�˶���Ļ�����
        {
            data_allocator ::deallocate(*cur,buffer_size());
            *cur = nullptr;
        }
        for(auto cur=end_.node +1;cur<=map_+map_size_;++cur)//�ͷ�β�˶���Ļ�����
        {
            data_allocator ::deallocate(*cur,buffer_size());
            *cur = nullptr;
        }
    }

    //��ͷ���͵ع���Ԫ��
    template<class T>
    template<class ...Args>
    void deque<T>::emplace_front(Args &&...args) {
        if(begin_.cur!=begin_.first)//ͷ�����������пռ�
        {
            data_allocator::construct(begin_.cur-1,mystl::forward<Args>(args)...);//��ͷ���������͵ع���Ԫ��
            --begin_.cur;//����ͷ��������
        }
        else//ͷ��������û�пռ�
        {
            require_capacity(1,true);//��������map�ռ�,����true��ʾ��ͷ���������ã�����1��ʾ��Ҫ��������С
            try
            {
                --begin_;//����ͷ��������
                data_allocator::construct(begin_.cur,mystl::forward<Args>(args)...);//��ͷ���������͵ع���Ԫ��
            }
            catch (...)
            {
                ++begin_;//����ʧ�ܣ�����ͷ��������
                //reallocate_map_at_front(1);//��������map�ռ�
                throw;
            }
        }
    }

    //��β���͵ع���Ԫ��
    template<class T>
    template<class ...Args>
    void deque<T>::emplace_back(Args &&...args)
    {
        if(end_.cur!=end_.last-1)//β�����������пռ�
        {
            data_allocator::construct(end_.cur,mystl::forward<Args>(args)...);//��β���������͵ع���Ԫ��
            ++end_;//����β��������
        }
        else//β��������û�пռ�
        {
            require_capacity(1,false);//��������map�ռ�,����false��ʾ��β���������ã�����1��ʾ��Ҫ��������С
            data_allocator ::construct(end_.cur,mystl::forward<Args>(args)...);//��β���������͵ع���Ԫ��
            ++end_;
        }
    }

    //��ָ��λ�þ͵ع���Ԫ��
    template<class T>
    template<class ...Args>
    typename deque<T>::iterator deque<T>::emplace(const_iterator pos,Args&&...args)
    {
        if(pos.cur==begin_.cur)//��ͷ���͵ع���Ԫ��
        {
            emplace_front(mystl::forward<Args>(args)...);
            return begin_;
        }
        else if(pos.cur==end_.cur)//��β���͵ع���Ԫ��
        {
            emplace_back(mystl::forward<Args>(args)...);
            auto tmp=end_;
            --tmp;
            return tmp;
        }
        else//���м�͵ع���Ԫ��
        {
            return insert_aux(pos,mystl::forward<Args>(args)...);
        }
    }

    //��ͷ������Ԫ��
    template<class T>
    void deque<T>::push_front(const value_type& value)
    {
        if(begin_.cur!=begin_.first)
        {
            data_allocator::construct(begin_.cur-1,value);//��ͷ���������͵ع���Ԫ��
            --begin_.cur;
        }
        else
        {
            require_capacity(1,true);//��������map�ռ�.�������壺true��ʾ��ͷ���������ã�1��ʾ��Ҫ��������С
            try
            {
                --begin_;
                data_allocator::construct(begin_.cur,value);
            }
            catch (...)
            {
                ++begin_;
                //reallocate_map_at_front(1);
                throw;
            }
        }
    }

    //��β������Ԫ��
    template<class T>
    void deque<T>::push_back(const value_type& value)
    {
        //
        if(end_.cur!=end_.last-1)
        {
            data_allocator::construct(end_.cur,value);//��β���������͵ع���Ԫ��
            ++end_;
        }
        else
        {
            require_capacity(1,false);//��������map�ռ�.�������壺false��ʾ��β���������ã�1��ʾ��Ҫ��������С
            try {
                data_allocator::construct(end_.cur, value);
                ++end_;
            }
            catch (...)
            {
                //reallocate_map_at_back(1);
                throw;
            }
        }
    }

    //����ͷ��Ԫ��
    template<class T>
    void deque<T>::pop_front()
    {
        MYSTL_DEBUG(!empty());
        if(begin_.cur!=begin_.last-1)//ͷ������������Ԫ��
        {
            data_allocator::destroy(begin_.cur);//��ͷ���������͵�����Ԫ��
            ++begin_.cur;
        }
        else//ͷ��������û��Ԫ��
        {
            data_allocator::destroy(begin_.cur);//��ͷ���������͵�����Ԫ��
            ++begin_;
            destroy_buffer(begin_.node-1,begin_.node-1);//����ǰ�˶���Ļ�����
        }
    }

    //����β��Ԫ��
    template<class T>
    void deque<T>::pop_back()
    {
        MYSTL_DEBUG(!empty());
        if(end_.cur!=end_.first)//β������������Ԫ��
        {
            --end_.cur;
            data_allocator::destroy(end_.cur);//��β���������͵�����Ԫ��
        }
        else//β��������û��Ԫ��
        {
            --end_;
            data_allocator::destroy(end_.cur);//��β���������͵�����Ԫ��
            destroy_buffer(end_.node+1,end_.node+1);//����β�˶���Ļ�����
        }
    }

    //��ָ��λ�ò���Ԫ��
    template<class T>
    typename deque<T>::iterator deque<T>::insert(iterator position,const value_type& value)
    {
        if(position.cur==begin_.cur)//��ͷ������Ԫ��
        {
            push_front(value);
            return begin_;
        }
        else if(position.cur==end_.cur)//��β������Ԫ��
        {
            push_back(value);
            auto tmp=end_;
            --tmp;
            return tmp;
        }
        else//���м����Ԫ��
        {
            return insert_aux(position,value);
        }
    }

    template<class T>
    typename deque<T>::iterator deque<T>::insert(iterator position,value_type&& value)
    {
        if(position.cur==begin_.cur)// ��ͷ������Ԫ��
        {
            emplace_front(mystl::move(value));
            return begin_;
        }
        else if(position.cur==end_.cur)// ��β������Ԫ��
        {
            emplace_back(mystl::move(value));
            auto tmp=end_;
            --tmp;
            return tmp;
        }
        else// ���м����Ԫ��
        {
            return insert_aux(position,mystl::move(value));
        }
    }

    //��ָ��λ�ò���n��Ԫ��
    template<class T>
    void deque<T>::insert(iterator position,size_type n,const value_type &value)
    {
        if(position.cur==begin_.cur)//��ͷ������Ԫ��
        {
            require_capacity(n,true);//��������map�ռ�.�������壺true��ʾ��ͷ���������ã�n��ʾ��Ҫ��������С
            auto new_begin=begin_-n;
            mystl::uninitialized_fill_n(new_begin,n,value);
            begin_=new_begin;
        }
        else if(position.cur==end_.cur)//��β������Ԫ��
        {
            require_capacity(n,false);//��������map�ռ�.�������壺false��ʾ��β���������ã�n��ʾ��Ҫ��������С
            auto new_end=end_+n;
            mystl::uninitialized_fill_n(end_,n,value);
            end_=new_end;
        }
        else//���м����Ԫ��
        {
            fill_insert(position,n,value);
        }
    }

    //ɾ��positionλ�õ�Ԫ��
    template<class T>
    typename deque<T>::iterator deque<T>::erase(iterator position)
    {
        MYSTL_DEBUG(position!=end());//position������β�������
        auto next=position;//ɾ��Ԫ�غ����һ��Ԫ��
        ++next;
        const size_type elems_before=position-begin_;//position֮ǰ��Ԫ�ظ���
        if(elems_before<(size()/2)) //���position֮ǰ��Ԫ�ظ���С��size()/2
        {
            mystl::copy_backward(begin_,position,next);//��position֮ǰ��Ԫ������ƶ�һ��λ��
            pop_front();//����ͷ��Ԫ��
        }
        else//���position֮ǰ��Ԫ�ظ������ڵ���size()/2
        {
            mystl::copy(next,end_,position);//��position֮���Ԫ����ǰ�ƶ�һ��λ��
            pop_back();//����β��Ԫ��
        }
        return begin_+elems_before;
    }

    //ɾ��[first,last)�����Ԫ��
    template<class T>
    typename deque<T>::iterator
    deque<T>::erase(iterator first,iterator last)
    {
        if(first==begin_ && last==end_)// ɾ������deque
        {
            clear();
            return end_;
        }
        else
        {
            const size_type n=last-first;//Ҫɾ����Ԫ�ظ���
            const size_type elems_before=first-begin_;//first֮ǰ��Ԫ�ظ���
            if(elems_before<(size()-n)/2)//���first֮ǰ��Ԫ�ظ���С��size()-n��һ��
            {
                mystl::copy_backward(begin_,first,last);//��first֮ǰ��Ԫ������ƶ�n��λ��
                auto new_begin=begin_+n;
                //destroy_buffer(begin_.node,new_begin.node);//����ǰ�˶���Ļ�����
                data_allocator ::destory(begin_.cur,new_begin.cur);//����ǰ�˶����Ԫ��
                begin_=new_begin;
            }
            else//���first֮ǰ��Ԫ�ظ������ڵ���size()-n��һ��
            {
                mystl::copy(last,end_,first);//��last֮���Ԫ����ǰ�ƶ�n��λ��
                auto new_end=end_-n;
                //destroy_buffer(new_end.node+1,end_.node+1);//����β�˶���Ļ�����
                data_allocator ::destory(new_end.cur,end_.cur);//����β�˶����Ԫ��
                end_=new_end;
            }
            return begin_+elems_before;
        }
    }

    //���
    template<class T>
    void deque<T>::clear()noexcept
    {
        //clear �ᱣ��һ��������
        for(map_pointer node=begin_.node+1;node<end_.node;++node)
        {
            //destroy_buffer(node,node+1);//���ٻ�����
            data_allocator ::destory(*node,*node+buffer_size);//����Ԫ��
        }
        if(begin_.node!=end_.node)//���deque�����������ϵĻ�����
        {
            //destroy_buffer(begin_.node+1,end_.node+1);//���ٻ�����
            data_allocator ::destory(begin_.cur,begin_.last);//����Ԫ��
            data_allocator ::destory(end_.first,end_.cur);//����Ԫ��
        }
        else//���deque��ֻ��һ��������
        {
            data_allocator ::destory(begin_.cur,end_.cur);//����Ԫ��
        }
        shrink_to_fit();//��Сmap�Ĵ�С
        end_=begin_;//����״̬
    }

    //��������deque
    template<class T>
    void deque<T>::swap(deque& rhs)noexcept
    {
        if(this!=&rhs)
        {
            mystl::swap(begin_,rhs.begin_);
            mystl::swap(end_,rhs.end_);
            mystl::swap(map_,rhs.map_);
            mystl::swap(map_size_,rhs.map_size_);
        }
    }

    /*********************************************/
    //��������

    //create_map �������������ź�deque��map�ռ�
    template<class T>
    typename deque<T>::map_pointer deque<T>::create_map(size_type size)
    {
        map_pointer mp=nullptr;
        mp=map_allocator::allocate(size);//����map�ռ�
        for(size_type i=0;i<size;++i)
        {
            *(mp+i)= nullptr;//���ýڵ�ռ�
        }
        return mp;
    };

    //creatr_buffer �������������ź�deque�Ļ�����
    template<class T>
    void deque<T>:: create_buffer(map_pointer nstart, map_pointer nfinish)
    {
        map_pointer cur;
        try
        {
            for(cur=nstart;cur<=nfinish;++cur)
            {
                *cur=data_allocator::allocate(buffer_size);//���û�����
            }
        }
        catch(...)
        {
            while (cur!=nstart)//������ù����г����쳣���ͷ��Ѿ����õĻ�����
            {
                --cur;  //����
                data_allocator::deallocate(*cur,buffer_size);//�ͷŻ�����
                *cur= nullptr;//ָ���
            }
            throw;//�����׳��쳣
        }
    }

    //destroy_buffer �����ͷ�deque�Ļ�����
    template<class T>
    void deque<T>::destroy_buffer(map_pointer nstart, map_pointer nfinish)
    {
        for(map_pointer node=nstart;node<=nfinish;++node)
        {
            data_allocator::deallocate(*node,buffer_size);//�ͷŻ�����
            *node= nullptr;//ָ���
        }
    }

    //map_init ������ʼ��map
    template<class T>
    void deque<T>::map_init(size_type nElem)
    {
        //������Ҫ�Ľڵ�����ÿ���ڵ����һ����������
        const size_type nNode=nElem/buffer_size+1;
        map_size_=mystl::max(static_cast<size_type>(DEQUE_MAP_INIT_SIZE), nNode + 2);//map�Ĵ�С����Ϊ8
        try
        {
            map_=create_map(map_size_);//����map�ռ�
        }
        catch(...)
        {
            map_= nullptr;//ָ���
            map_size_=0;//map��СΪ0
            throw;//�����׳��쳣
        }
        //��nstart��nfinishָ��map��ӵ�е�ȫ���ڵ������������
        map_pointer nstart=map_+(map_size_-nNode)/2;
        map_pointer nfinish=nstart+nNode-1;
        try
        {
            create_buffer(nstart,nfinish);//����������
        }
        catch(...)
        {
            map_allocator::deallocate(map_,map_size_);//�ͷ�map�ռ�
            map_= nullptr;//ָ���
            map_size_=0;//map��СΪ0
            throw;//�����׳��쳣
        }
        begin_.set_node(nstart);//�趨deque����ʼ������
        end_.set_node(nfinish);//�趨deque�Ľ���������
        begin_.cur=begin_.first;//�趨deque����ʼԪ��
        end_.cur=end_.first+nElem%buffer_size;//�趨deque�Ľ���Ԫ��
    }

    //fill_init �������deque
    template<class T>
    void deque<T>::fill_init(size_type n, const value_type& value)
    {
        map_init(n);//��ʼ��map
        if(n!=0)
        {
            for(auto cur=begin_.node;cur<end_.node;++cur)
            {
                mystl::uninitialized_fill(*cur,*cur+buffer_size,value);//���ÿ��������
            }
            mystl::uninitialized_fill(end_.first,end_.cur,value);//������һ��������
        }
    }

    //copy_init ��������deque
    template<class T>
    template<class IIter>
    void deque<T>::copy_init(IIter first, IIter last, mystl::input_iterator_tag)
    {
        const size_type n = mystl::distance(first, last);//������Ҫ������Ԫ�ظ���
        map_init(n);//��ʼ��map
        for(;first!=last;++first)
        {
            emplace_back(*first);//��Ԫ��һ�������뵽deque��
        }
    }

    template<class T>
    template<class FIter>
    void deque<T>::copy_init(FIter first,FIter last,forward_iterator_tag)
    {
        const size_type n=mystl::distance(first,last);//������Ҫ������Ԫ�ظ���
        map_init(n);//��ʼ��map
        for(auto cur=begin_.node;cur<end_.node;++cur)
        {
            auto next=first;
            mystl::advance(next,buffer_size);//����ÿ����������Ҫ������Ԫ�ظ���
            mystl::uninitialized_copy(first,next,*cur);//����Ԫ��
            first=next;//����first
        }
        mystl::uninitialized_copy(first,last,end_.first);//�������һ����������Ԫ��
    }

    //fill_assign �������deque
    template<class T>
    void deque<T>::fill_assign(size_type n, const value_type& value)
    {
        if(n>size())
        {
            mystl::fill(begin(),end(),value);//�������Ԫ��
            insert(end(),n-size(),value);//������Ԫ��
        }
        else
        {
            erase(begin()+n,end());//��������Ԫ��
            mystl::fill(begin(),end(),value);//�������Ԫ��
        }
    }

    //copy_assign ��������deque
    template<class T>
    template<class IIter>
    void deque<T>::copy_assign(IIter first,IIter last,input_iterator_tag)
    {
        auto first1=begin();
        auto last1=end();
        for(;first!=last && first1!= last1;++first,++first1)
        {
            *first1=*first;//����Ԫ��
        }
        if(first1!=last1)
        {
            erase(first1,last1);//��������Ԫ��
        }
        else
        {
            insert_dispatch(end(),first,last,input_iterator_tag());//������Ԫ��
        }
    }

    template<class T>
    template<class FIter>
    void deque<T>::copy_assign(FIter first,FIter last,forward_iterator_tag)
    {
        const size_type len=mystl::distance(first,last);
        if(len>size())
        {
            auto mid=first;
            mystl::advance(mid,size());//�ҵ��м�λ��
            mystl::copy(first,mid,begin());//����ǰ���
            insert_dispatch(end(),mid,last,forward_iterator_tag());//�������
        }
        else
        {
            erase(mystl::copy(first,last,begin()),end());//��������������Ԫ��
        }
    }

    //insert_aux ���� ������ָ��λ�ò���Ԫ��
    template<class T>
    template<class ...Args>
    typename deque<T>::iterator
    deque<T>::insert_aux(iterator position,Args&&...args)
    {
        const size_type elems_before=position-begin();//�����֮ǰ��Ԫ�ظ���
        value_type value_copy=value_type(mystl::forward<Args>(args)...);//��Ԫ�ص�����
        if(elems_before< size()/2)//��������֮ǰ��Ԫ�ظ����Ƚ���
        {
            emplace_front(front());//����ǰ�˲������һԪ��ֵͬ��Ԫ��
            auto front1=begin();
            ++front1;
            auto front2=front1;
            ++front2;
            position=begin()+elems_before;
            auto pos1=position;
            ++pos1;
            mystl::copy(front2,pos1,front1);//�������֮ǰ��Ԫ����ǰ�ƶ�һ��λ��
        }
        else//��������֮���Ԫ�ظ����Ƚ���
        {
            emplace_back(back());//����β�˲��������Ԫ��ֵͬ��Ԫ��
            auto back1=end();
            --back1;
            auto back2=back1;
            --back2;
            position=begin()+elems_before;
            mystl::copy_backward(position,back2,back1);//�������֮���Ԫ������ƶ�һ��λ��
        }
        *position=value_copy;//�ڲ�������趨��ֵ
        return position;
    }

    //fill_insert ���� ������ָ��λ�ò���n��Ԫ��
    template<class T>
    void deque<T>::fill_insert(iterator position,size_type n,const value_type& value)
    {
        const size_type elems_before=position-begin_;//�����֮ǰ��Ԫ�ظ���
        const size_type length=size();//deque�ĳ���
        auto value_copy=value;//����һ����ֵ
        if(elems_before<(length/2))
        {
            //��������֮ǰ��Ԫ�ظ����Ƚ���
            require_capacity(n, true);//����Ƿ���Ҫ����,����Ϊtrue��ʾ��ǰ������
            //ԭ���ĵ���������ʧЧ����Ҫ���¶�λ
            auto old_begin=begin_;//��¼ԭʼ��begin
            auto new_begin=begin_-n;//�����µ���ʼ��
            position=begin_+elems_before;//���¶�λ�����
            try
            {
                if(elems_before>=n)
                {
                    //�����֮ǰ��Ԫ�ظ������ڵ�������Ԫ�ظ���
                    auto begin_n=begin_+n;
                    mystl::uninitialized_copy(begin_,begin_n,new_begin);//�������֮ǰ��Ԫ�ؿ������µ���ʼ��
                    begin_=new_begin;//�趨deque����ʼ��
                    mystl::copy(begin_n,position,old_begin);//�������֮���Ԫ�ؿ�����ԭʼ����ʼ��
                    mystl::fill(position,begin_n,value_copy);//�������Ԫ��
                }
                else
                {
                    //�����֮ǰ��Ԫ�ظ���С������Ԫ�ظ���
                    mystl::uninitialized_fill(
                            mystl::uninitialized_copy(begin_, position, new_begin), begin_, value_copy);//�������Ԫ��
                    begin_ = new_begin;//�趨deque����ʼ��
                    mystl::fill(old_begin, position, value_copy);//���ԭʼ��ʼ�㵽������Ԫ��
                }
            }
            catch (...)
            {
                if (new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        else
        {
            require_capacity(n, false);
            // ԭ���ĵ��������ܻ�ʧЧ
            auto old_end = end_;
            auto new_end = end_ + n;
            const size_type elems_after = length - elems_before;
            position = end_ - elems_after;
            try
            {
                if (elems_after > n)
                {
                    auto end_n = end_ - n;
                    mystl::uninitialized_copy(end_n, end_, end_);
                    end_ = new_end;
                    mystl::copy_backward(position, end_n, old_end);
                    mystl::fill(position, position + n, value_copy);
                }
                else
                {
                    mystl::uninitialized_fill(end_, position + n, value_copy);
                    mystl::uninitialized_copy(position, end_, position + n);
                    end_ = new_end;
                    mystl::fill(position, old_end, value_copy);
                }
            }
            catch (...)
            {
                if(new_end.node != end_.node)
                    destroy_buffer(end_.node + 1, new_end.node);
                throw;
            }
        }
    }

    //copy_insert ���� ������ָ��λ�ò���[first,last)�����Ԫ��
    template<class T>
    template <class FIter>
    void deque<T>::
    copy_insert(iterator position, FIter first, FIter last, size_type n)
    {
        const size_type elems_before = position - begin_;
        auto len = size();
        if (elems_before < (len / 2))
        {
            require_capacity(n, true);
            // ԭ���ĵ��������ܻ�ʧЧ
            auto old_begin = begin_;
            auto new_begin = begin_ - n;
            position = begin_ + elems_before;
            try
            {
                if (elems_before >= n)
                {
                    auto begin_n = begin_ + n;
                    mystl::uninitialized_copy(begin_, begin_n, new_begin);
                    begin_ = new_begin;
                    mystl::copy(begin_n, position, old_begin);
                    mystl::copy(first, last, position - n);
                }
                else
                {
                    auto mid = first;
                    mystl::advance(mid, n - elems_before);
                    mystl::uninitialized_copy(first, mid,
                                              mystl::uninitialized_copy(begin_, position, new_begin));
                    begin_ = new_begin;
                    mystl::copy(mid, last, old_begin);
                }
            }
            catch (...)
            {
                if(new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        else
        {
            require_capacity(n, false);
            // ԭ���ĵ��������ܻ�ʧЧ
            auto old_end = end_;
            auto new_end = end_ + n;
            const auto elems_after = len - elems_before;
            position = end_ - elems_after;
            try
            {
                if (elems_after > n)
                {
                    auto end_n = end_ - n;
                    mystl::uninitialized_copy(end_n, end_, end_);
                    end_ = new_end;
                    mystl::copy_backward(position, end_n, old_end);
                    mystl::copy(first, last, position);
                }
                else
                {
                    auto mid = first;
                    mystl::advance(mid, elems_after);
                    mystl::uninitialized_copy(position, end_,
                                              mystl::uninitialized_copy(mid, last, end_));
                    end_ = new_end;
                    mystl::copy(first, mid, position);
                }
            }
            catch (...)
            {
                if(new_end.node != end_.node)
                    destroy_buffer(end_.node + 1, new_end.node);
                throw;
            }
        }
    }

    // insert_dispatch ����
    template <class T>
    template <class IIter>
    void deque<T>::
    insert_dispatch(iterator position, IIter first, IIter last, input_iterator_tag)
    {
        if (last <= first)  return;
        const size_type n = mystl::distance(first, last);
        const size_type elems_before = position - begin_;
        if (elems_before < (size() / 2))
        {
            require_capacity(n, true);
        }
        else
        {
            require_capacity(n, false);
        }
        position = begin_ + elems_before;
        auto cur = --last;//curָ�����һ��Ԫ��
        for (size_type i = 0; i < n; ++i, --cur)//�Ӻ���ǰ����
        {                                //��ֹ�����ʱ�������ʧЧ
            insert(position, *cur);//����insert����
        }
    }

    template <class T>
    template <class FIter>
    void deque<T>::
    insert_dispatch(iterator position, FIter first, FIter last, forward_iterator_tag)
    {
        if (last <= first)  return;
        const size_type n = mystl::distance(first, last);
        if (position.cur == begin_.cur) //��ͷ������
        {
            require_capacity(n, true);
            auto new_begin = begin_ - n;
            try
            {
                mystl::uninitialized_copy(first, last, new_begin);
                begin_ = new_begin;
            }
            catch (...)
            {
                if(new_begin.node != begin_.node)
                    destroy_buffer(new_begin.node, begin_.node - 1);
                throw;
            }
        }
        else if (position.cur == end_.cur)
        {
            require_capacity(n, false);
            auto new_end = end_ + n;
            try
            {
                mystl::uninitialized_copy(first, last, end_);
                end_ = new_end;
            }
            catch (...)
            {
                if(new_end.node != end_.node)
                    destroy_buffer(end_.node + 1, new_end.node);
                throw;
            }
        }
        else
        {
            copy_insert(position, first, last, n);
        }
    }

    // require_capacity ���� �����ж��Ƿ���Ҫ����
    template <class T>
    void deque<T>::require_capacity(size_type n, bool front)
    {
        if (front && (static_cast<size_type>(begin_.cur - begin_.first) < n))
        {   //��ͷ������
            const size_type need_buffer = (n - (begin_.cur - begin_.first)) / buffer_size + 1;
            if (need_buffer > static_cast<size_type>(begin_.node - map_))
            {
                reallocate_map_at_front(need_buffer);
                return;
            }
            create_buffer(begin_.node - need_buffer, begin_.node - 1);
        }
        else if (!front && (static_cast<size_type>(end_.last - end_.cur - 1) < n))
        {
            const size_type need_buffer = (n - (end_.last - end_.cur - 1)) / buffer_size + 1;
            if (need_buffer > static_cast<size_type>((map_ + map_size_) - end_.node - 1))
            {
                reallocate_map_at_back(need_buffer);
                return;
            }
            create_buffer(end_.node + 1, end_.node + need_buffer);
        }
    }

    // reallocate_map_at_front ���� ������ͷ������

    template <class T>
    void deque<T>::reallocate_map_at_front(size_type need_buffer)
    {
        const size_type new_map_size = mystl::max(map_size_ << 1,
                                                  map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
        map_pointer new_map = create_map(new_map_size);
        const size_type old_buffer = end_.node - begin_.node + 1;
        const size_type new_buffer = old_buffer + need_buffer;

        // ���µ� map �е�ָ��ָ��ԭ���� buffer���������µ� buffer
        auto begin = new_map + (new_map_size - new_buffer) / 2;
        auto mid = begin + need_buffer;
        auto end = mid + old_buffer;
        create_buffer(begin, mid - 1);
        for (auto begin1 = mid, begin2 = begin_.node; begin1 != end; ++begin1, ++begin2)
            *begin1 = *begin2;

        // ��������
        map_allocator::deallocate(map_, map_size_);
        map_ = new_map;
        map_size_ = new_map_size;
        begin_ = iterator(*mid + (begin_.cur - begin_.first), mid);
        end_ = iterator(*(end - 1) + (end_.cur - end_.first), end - 1);
    }

    // reallocate_map_at_back ���� ������β������

    template <class T>
    void deque<T>::reallocate_map_at_back(size_type need_buffer)
    {
        const size_type new_map_size = mystl::max(map_size_ << 1,
                                                  map_size_ + need_buffer + DEQUE_MAP_INIT_SIZE);
        map_pointer new_map = create_map(new_map_size);
        const size_type old_buffer = end_.node - begin_.node + 1;
        const size_type new_buffer = old_buffer + need_buffer;

        // ���µ� map �е�ָ��ָ��ԭ���� buffer���������µ� buffer
        auto begin = new_map + ((new_map_size - new_buffer) / 2);
        auto mid = begin + old_buffer;
        auto end = mid + need_buffer;
        for (auto begin1 = begin, begin2 = begin_.node; begin1 != mid; ++begin1, ++begin2)
            *begin1 = *begin2;
        create_buffer(mid, end - 1);

        // ��������
        map_allocator::deallocate(map_, map_size_);
        map_ = new_map;
        map_size_ = new_map_size;
        begin_ = iterator(*begin + (begin_.cur - begin_.first), begin);
        end_ = iterator(*(mid - 1) + (end_.cur - end_.first), mid - 1);
    }

    // ���رȽϲ�����
    template <class T>
    bool operator==(const deque<T>& lhs, const deque<T>& rhs)
    {
        return lhs.size() == rhs.size() &&
               mystl::equal(lhs.begin(), lhs.end(), rhs.begin());
    }

    template <class T>
    bool operator<(const deque<T>& lhs, const deque<T>& rhs)
    {
        return mystl::lexicographical_compare(lhs.begin(), lhs.end(),
                                              rhs.begin(), rhs.end());
    }

    template <class T>
    bool operator!=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class T>
    bool operator>(const deque<T>& lhs, const deque<T>& rhs)
    {
        return rhs < lhs;
    }

    template <class T>
    bool operator<=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class T>
    bool operator>=(const deque<T>& lhs, const deque<T>& rhs)
    {
        return !(lhs < rhs);
    }

    // ���� mystl �� swap
    template <class T>
    void swap(deque<T>& lhs, deque<T>& rhs)
    {
        lhs.swap(rhs);
    }

}
#endif //STL_STUDY_DEQUE_H