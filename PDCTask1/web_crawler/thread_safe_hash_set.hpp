/*
* My thread safe hash table.
* Table stores only the results of hashing.
*/
#ifndef SHREAD_SAFE_HASH_SET_H
#define SHREAD_SAFE_HASH_SET_H

#include <vector>
#include <list>
#include <thread>
#include <mutex>

template<typename Key>
class thread_safe_hash_set
{
private:
    class bucket_type {
    private:
        typedef std::size_t bucket_value;
        typedef std::list<bucket_value> bucket_data;
        typedef typename bucket_data::iterator bucket_iterator;

        bucket_data data;
        std::mutex mutex;

        bucket_iterator find_entry_for(bucket_value const &key) {
            bucket_iterator it = data.begin();
            for (; it != data.end(); ++it) {
                if ((*it) == key) {
                    return it;
                }
            }
            return it;
        }

    public:  

        bool insert(bucket_value const &key) {
            std::lock_guard<std::mutex> lock(mutex);
            bucket_iterator const found_entry = find_entry_for(key);
            if (found_entry == data.end()) {
                data.push_back(key);
                return true;
            } else {
                return false;
            }
        }

        bool count(bucket_value const &key) {
            std::lock_guard<std::mutex> lock(mutex);
            bucket_iterator const found_entry = find_entry_for(key);
            if (found_entry == data.end()) {
                return false;
            } else {
                return true;
            }
        }

        void remove(bucket_value const &key) {
            std::lock_guard<std::mutex> lock(mutex);
            bucket_iterator const found_entry = find_entry_for(key);
            if (found_entry != data.end()) {
                data.erase(found_entry);
            }
        }
    };


    std::vector<std::unique_ptr<bucket_type>> buckets;
    std::hash<std::string> hasher;
    
public:
    typedef Key key_type;

    thread_safe_hash_set(
            size_t num_buckets = 100) :
            buckets(num_buckets)
    {
        for( size_t i = 0; i < num_buckets; ++i ) {
            buckets[i].reset(new bucket_type) ;
        }
    }

    thread_safe_hash_set(thread_safe_hash_set const& other) = delete;
    thread_safe_hash_set& operator=(thread_safe_hash_set const& other) = delete;

    void reset(size_t num_buckets)
    {
        buckets.erase(buckets.begin(), buckets.end());
        buckets.resize(num_buckets);
        for( size_t i = 0; i < num_buckets; ++i ) {
            buckets[i].reset(new bucket_type) ;
        }
    }

    bool insert(Key const& key)
    {
        std::size_t const hash_v = hasher(key);
        return buckets[hash_v % buckets.size()]->insert(hash_v);
    }

    bool count(Key const& key)
    {
        std::size_t const hash_v = hasher(key);
        return buckets[hash_v % buckets.size()]->count(hash_v);
    }

    void remove(Key const& key)
    {
        std::size_t const hash_v = hasher(key);
        buckets[hash_v % buckets.size()]->remove(hash_v);
    }
};

#endif