/*
    The MIT License (MIT)

    Copyright (c) 2016 Rafael Fassi Lobão - rafael@fassi.com.br

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/************************
* Alpha version: 1.0.002
*************************/

#ifndef MULTIINDEX
#define MULTIINDEX

#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <cassert>

template <typename C>
class MultiIndex
{
    typedef std::vector<C> ClsT;
public:

    template <typename T>
    class Iterator
    {
    public:
        virtual ~Iterator() {}
        virtual void next() = 0;
        virtual bool is_begin() = 0;
        virtual bool is_end() = 0;
        virtual const T& key() = 0;
        virtual C& value() = 0;
        virtual size_t idx() = 0;
    };

    class IndexBase
    {
    public:
        virtual ~IndexBase() {}
        virtual void reserve(size_t size) = 0;
        virtual void setValue(size_t idx, C *cl) = 0;
        virtual void remove(size_t idx)  = 0;
    };

    template <typename T>
    class Index : public IndexBase
    {
    public:
        Index(ClsT &_cls) : cls(_cls) {}
        virtual C* findFirst(const T &) = 0;
        virtual std::shared_ptr<Iterator<T>> begin() = 0;
        virtual std::shared_ptr<Iterator<T>> rbegin() = 0;
        virtual std::shared_ptr<Iterator<T>> find(const T &) = 0;
        virtual bool contensField(T C::* _filed) { (void)_filed; return false; }
        virtual bool contensField(void *_fileds) { (void)_fileds; return false; }
    protected:
        ClsT &cls;
    };

private:
    template <typename T, typename U>
    class IteratorForward : public Iterator<T>
    {
        typedef U DataT;
        typedef typename DataT::iterator ItT;

        ClsT &cls;
        DataT &data;
        ItT it;
    public:
        IteratorForward(ClsT &_cls, U &_data, ItT _it) : cls(_cls), data(_data), it(_it) {}
        ~IteratorForward() {}

        virtual void next() { ++it; }
        virtual bool is_begin() { return (it == data.begin()); }
        virtual bool is_end() { return (it == data.end()); }
        virtual const T& key() { return it->first; }
        virtual C& value() { return cls.at(it->second); }
        virtual size_t idx() { return it->second; }
    };

    template <typename T, typename U>
    class IteratorBackward : public Iterator<T>
    {
        typedef U DataT;
        typedef typename DataT::reverse_iterator ItT;

        ClsT &cls;
        DataT &data;
        ItT it;
    public:
        IteratorBackward(ClsT &_cls, DataT &_data, ItT _it) : cls(_cls), data(_data), it(_it) {}
        ~IteratorBackward() {}

        virtual void next() { ++it; }
        virtual bool is_begin() { return (it == data.rbegin()); }
        virtual bool is_end() { return (it == data.rend()); }
        virtual const T& key() { return it->first; }
        virtual C& value() { return cls.at(it->second); }
        virtual size_t idx() { return it->second; }
    };

    template <typename T, typename U>
    class IndexSingle : public Index<T>
    {
    public:
        IndexSingle(ClsT &_cls, T C::* _filed) : Index<T>(_cls), filed(_filed) {}

        virtual void setValue(size_t idx, C *cl)
        {
                data.insert(std::pair<T, size_t>(cl->*this->filed, idx));
        }

        virtual C* findFirst(const T &val)
        {
            typename U::const_iterator it = data.find(val);
            if(it != data.end())
                return &this->cls.at(it->second);
            else
                return 0;
        }

        virtual void remove(size_t idx)
        {
            typename U::iterator it = data.begin();
            while (it != data.end())
            {
                if (it->second < idx)
                {
                    ++it;
                    continue;
                }
                else if(it->second != idx)
                {
                    --it->second;
                    ++it;
                    continue;
                }
                else
                {
                    it = data.erase(it);
                }
            }
        }

        virtual std::shared_ptr<Iterator<T>> begin()
        {
            return std::make_shared<IteratorForward<T, U>>(this->cls, data, data.begin());
        }

        virtual std::shared_ptr<Iterator<T>> find(const T &val)
        {
            return std::make_shared<IteratorForward<T, U>>(this->cls, data, data.find(val));
        }

        virtual std::vector<C*> list(const T &val)
        {
            std::vector<C*> vec;
            typename U::const_iterator it;
            for (it = data.find(val); it != data.end(); ++it)
            {
                if (it->first == val)
                    vec.push_back(&this->cls.at(it->second));
                else
                    break;
            }
            return vec;
        }

        virtual bool contensField(T C::* _filed)
        {
            return (filed == _filed);
        }

    protected:
        T C::* filed;
        U data;
    };

    template <typename T, typename U>
    class IndexSingleMap : public IndexSingle<T, U>
    {
    public:
        IndexSingleMap(ClsT &_cls, T C::* _filed) : IndexSingle<T, U>(_cls, _filed) {}

        virtual void reserve(size_t size)
        {
            (void)size;
        }

        virtual std::shared_ptr<Iterator<T>> rbegin()
        {
            return std::make_shared<IteratorBackward<T, U>>(this->cls, this->data, this->data.rbegin());
        }
    };


    template <typename T, typename U>
    class IndexSingleHash : public IndexSingle<T, U>
    {
    public:
        IndexSingleHash(ClsT &_cls, T C::* _filed) : IndexSingle<T, U>(_cls, _filed) {}

        virtual void reserve(size_t size)
        {
            this->data.reserve(size);
        }

        virtual std::shared_ptr<Iterator<T>> rbegin()
        {
            assert(false); // rbegin is not allowed for the hash type;
            return std::shared_ptr<Iterator<T>>();
        }
    };

    template <typename T, typename F, typename U>
    class IndexComposite : public Index<T>
    {
        template<std::size_t> struct StaticCount{};
    public:
        IndexComposite(ClsT &_cls, F _fileds) : Index<T>(_cls), fields(_fileds) {}

        template <std::size_t I>
        void setTupleValue(C *c, F &f, T &v, StaticCount<I>)
        {
            setTupleValue(c, f, v, StaticCount<I - 1>());
            std::get<I>(v) = c->*std::get<I>(f);
        }

        void setTupleValue(C *c, F &f, T &v, StaticCount<0>)
        {
            std::get<0>(v) = c->*std::get<0>(f);
        }

        virtual void reserve(size_t size)
        {
            (void)size;
        }

        virtual void setValue(size_t idx, C *cl)
        {
            T tpVal;
            setTupleValue(cl, fields, tpVal, StaticCount<std::tuple_size<T>::value - 1>());
            data.insert(std::pair<T, size_t>(tpVal, idx));
        }

        virtual std::shared_ptr<Iterator<T>> begin()
        {
            return std::make_shared<IteratorForward<T, U> >(this->cls, data, data.begin());
        }

        virtual std::shared_ptr<Iterator<T>> rbegin()
        {
            return std::make_shared<IteratorBackward<T, U> >(this->cls, data, data.rbegin());
        }

        virtual C* findFirst(const T &val)
        {
            typename U::const_iterator it = data.find(val);
            if (it != data.end())
                return &this->cls.at(it->second);
            else
                return 0;
        }

        virtual void remove(size_t idx)
        {
            typename U::iterator it = data.begin();
            while (it != data.end())
            {
                if (it->second < idx)
                {
                    ++it;
                    continue;
                }
                else if(it->second != idx)
                {
                    --it->second;
                    ++it;
                    continue;
                }
                else
                {
                    it = data.erase(it);
                }
            }
        }

        virtual std::shared_ptr<Iterator<T>> find(const T &val)
        {
            return std::make_shared<IteratorForward<T, U>>(this->cls, data, data.find(val));
        }

        virtual bool contensField(void *_fileds)
        {
            F *tpFields = static_cast<F*>(_fileds);
            return (*tpFields == fields);
        }

    private:
        F fields;
        U data;
    };


    typedef std::vector<IndexBase*> IdxsT;

public:
    MultiIndex() {}
    ~MultiIndex()
    {
        for (typename IdxsT::iterator it = m_idxs.begin(); it != m_idxs.end(); ++it)
            delete *it;
    }

    void addData(const C &cl)
    {
        C *pCl = const_cast<C*>(&cl);
        for (size_t i = 0; i < m_idxs.size(); ++i)
        {
            IndexBase *p = m_idxs[i];
            p->setValue(m_cls.size(), pCl);
        }
        m_cls.push_back(*pCl);
    }

    template<class T>
    Index<T> *addIndex_Ordered_Unique(T C::*field)
    {
        typedef std::map<T, size_t> ContT;
        typedef IndexSingleMap<T, ContT> IdxT;

        IdxT *index = new IdxT(m_cls, field);
        m_idxs.push_back(index);
        return static_cast<Index<T> *>(index);
    }

    template<typename... T>
    Index<std::tuple<T...> > *addIndex_Ordered_Unique(T C::*... fields)
    {
        typedef std::tuple<T...> TupleT;
        typedef std::tuple<T C::*...> TupleF;
        typedef std::map<TupleT, size_t> ContT;
        typedef IndexComposite<TupleT, TupleF, ContT> IdxT;

        TupleF tpFields = TupleF(fields...);
        IdxT *index = new IdxT(m_cls, tpFields);
        m_idxs.push_back(index);
        return static_cast<Index<TupleT> *>(index);
    }

    template<class T>
    Index<T> *addIndex_Ordered_NonUnique(T C::*field)
    {
        typedef std::multimap<T, size_t> ContT;
        typedef IndexSingleMap<T, ContT> IdxT;

        IdxT *index = new IdxT(m_cls, field);
        m_idxs.push_back(index);
        return static_cast<Index<T> *>(index);
    }

    template<typename... T>
    Index<std::tuple<T...> > *addIndex_Ordered_NonUnique(T C::*... fields)
    {
        typedef std::tuple<T...> TupleT;
        typedef std::tuple<T C::*...> TupleF;
        typedef std::multimap<TupleT, size_t> ContT;
        typedef IndexComposite<TupleT, TupleF, ContT> IdxT;

        TupleF tpFields = TupleF(fields...);
        IdxT *index = new IdxT(m_cls, tpFields);
        m_idxs.push_back(index);
        return static_cast<Index<TupleT> *>(index);
    }

    template<class T>
    Index<T> *addIndex_Hashed_Unique(T C::*field)
    {
        typedef std::unordered_map<T, size_t> ContT;
        typedef IndexSingleHash<T, ContT> IdxT;

        IdxT *index = new IdxT(m_cls, field);
        m_idxs.push_back(index);
        return static_cast<Index<T> *>(index);
    }

    template<class T>
    Index<T> *addIndex_Hashed_NonUnique(T C::*field)
    {
        typedef std::unordered_multimap<T, size_t> ContT;
        typedef IndexSingleHash<T, ContT> IdxT;

        IdxT *index = new IdxT(m_cls, field);
        m_idxs.push_back(index);
        return static_cast<Index<T> *>(index);
    }

    template<class T>
    Index<T> *getIndex(T C::*field)
    {
        typedef Index<T> IdxT;
        for (size_t i = 0; i < m_idxs.size(); ++i)
        {
            IdxT *idx = static_cast<IdxT*>(m_idxs[i]);
            if (idx->contensField(field))
                return idx;
        }
        return 0;
    }

    template<typename... T>
    Index<std::tuple<T...> > *getIndex(T C::*... fields)
    {
        typedef std::tuple<T...> TupleT;
        typedef std::tuple<T C::*...> TupleF;
        typedef Index<TupleT> IdxT;

        TupleF tpFields = TupleF(fields...);
        for (size_t i = 0; i < m_idxs.size(); ++i)
        {
            IdxT *idx = static_cast<IdxT*>(m_idxs[i]);
            if (idx->contensField(&tpFields))
                return idx;
        }
        return 0;
    }

    void reserve(size_t size)
    {
        m_cls.reserve(size);
        for (typename IdxsT::iterator it = m_idxs.begin(); it != m_idxs.end(); ++it)
            (*it)->reserve(size);
    }

    template<typename T>
    void remove(std::shared_ptr<Iterator<T>> i)
    {
        if(!i->is_end())
        {
            m_cls.erase(m_cls.begin() + i->idx());
            for (typename IdxsT::iterator it = m_idxs.begin(); it != m_idxs.end(); ++it)
                (*it)->remove(i->idx());
        }
    }

private:
    IdxsT m_idxs;
    ClsT m_cls;
};


#endif // MULTIINDEX
