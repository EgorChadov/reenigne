#include "alfe/main.h"

#ifndef INCLUDED_HASH_TABLE_H
#define INCLUDED_HASH_TABLE_H

#include "alfe/tuple.h"

// HashTable is not quite a value type, since changing an element in one table
// will affect copies of the same table. Adding an element may cause it to
// become a deep copy, if more storage space was needed.
//
// Note that the default-constructed Key should not be used for real entries.

template<class Key, class Value> class HashTableEntry
{
public:
    HashTableEntry() : _key(Key()), _value(Value()) { }
    HashTableEntry(const Key& key, const Value& value)
      : _key(key), _value(value) { }
    const Key& key() const { return _key; }
    const Value& value() const { return _value; }
    Key& key() { return _key; }
    Value& value() { return _value; }
private:
    Key _key;
    Value _value;
};

template<class Key, class Value> class HashTableBody
  : public Array<HashTableEntry<Key, Value>>::AppendableBaseBody
{
public:
    virtual void justSetSize(int size) const = 0;
    void preDestroy() const { justSetSize(this->_allocated); }
};

template<class Key, class Value> class HashTable
  : private AppendableArray<HashTableEntry<Key, Value>,
    HashTableBody<Key, Value>>
{
    typedef HashTableEntry<Key, Value> Entry;
public:
    bool hasKey(const Key& key) const
    {
        auto e = lookup(key);
        if (e != 0)
            return e->key() == key;
        return false;
    }
    Value& operator[](const Key& key)
    {
        auto e = lookup(key);
        if (e != 0 && e->key() == key)
            return e->value();
        if (count() >= this->allocated()*3/4) {
            int n = this->allocated()*2;
            if (n == 0)
                n = 1;
            HashTable other;
            other.allocate(n);
            n = other.allocated();
            other.expand(n);
            other.body()->_size = 0;
            for (auto i : *this)
                other[i.key()] = i.value();
            *this = other;
            e = lookup(key);
        }
        ++this->body()->_size;
        e->key() = key;
        return e->value();
    }
    Value operator[](const Key& key) const
    {
        auto e = lookup(key);
        if (e != 0)
            return e->value();
        return Value();
    }
    void add(const Key& key, const Value& value) { (*this)[key] = value; }
    int count() const { return this->body() == 0 ? 0 : this->body()->_size; }
    class Iterator
    {
    public:
        const Entry& operator*() const { return *_entry; }
        const Entry* operator->() const { return _entry; }
        bool operator==(const Iterator& other) const
        {
            return _entry == other._entry;
        }
        bool operator!=(const Iterator& other) const
        {
            return !operator==(other);
        }
        void operator++()
        {
            do {
                ++_entry;
                if ((*this) == _table.end())
                    return;
            } while (_entry->key() == Key());
        }
    private:
        Iterator(const Entry* entry, const HashTable& table)
          : _entry(entry), _table(table) { }
        const Entry* _entry;
        const HashTable _table;

        friend class HashTable;
    };
    Iterator begin() const
    {
        if (this->allocated() == 0)
            return Iterator(0, *this);
        Iterator i(data(0), *this);
        if (i->key() == Key())
            ++i;
        return i;
    }
    Iterator end() const
    {
        if (this->allocated() == 0)
            return Iterator(0, *this);
        return Iterator(data(this->allocated()), *this);
    }
    template<class V1> bool operator==(HashTable<Key, V1> other) const
    {
        if (count() != other.count())
            return false;
        for (auto i : *this) {
            Key k = i.key();
            if (!other.hasKey(k))
                return false;
            if (i.value() != other[k])
                return false;
        }
        return true;
    }
    template<class K1, class V1> bool operator==(HashTable<K1, V1> other) const
    {
        if (count() != other.count())
            return false;
        for (auto i : *this) {
            Key k = i.key();
            if (!other.hasKey(k))
                return false;
            if (i.value() != other[k])
                return false;
        }
        for (auto i : other) {
            K1 k = i.key();
            if (!hasKey(k))
                return false;
            if (i.value() != (*this)[k])
                return false;
        }
        return true;
    }
private:
    int row(const Key& key) const { return ::hash(key) % this->allocated(); }
    Entry* lookup(const Key& key)
    {
        if (this->allocated() == 0)
            return 0;
        int r = row(key);
        for (int i = 0; i < this->allocated(); ++i) {
            // We have a decent hash function so linear probing should work
            // fine.
            r = (r + 1)%this->allocated();
            Entry* e = data(r);
            if (e->key() == key || e->key() == Key())
                return e;
        }
        return 0;
    }
    const Entry* lookup(const Key& key) const
    {
        if (this->allocated() == 0)
            return 0;
        int r = row(key);
        for (int i = 0; i < this->allocated(); ++i) {
            r = (r + 1)%this->allocated();
            const Entry* e = data(r);
            if (e->key() == key || e->key() == Key())
                return e;
        }
        return 0;
    }
    Entry* data(int row)
    {
        return &static_cast<AppendableArray<Entry,
            HashTableBody<Key, Value>>&>(*this)[row];
    }
    const Entry* data(int row) const
    {
        return &static_cast<const AppendableArray<Entry,
            HashTableBody<Key, Value>>&>(*this)[row];
    }
};

#endif // INCLUDED_HASH_TABLE_H
