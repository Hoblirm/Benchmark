#ifndef ADDRESSED_MAP_H
#define ADDRESSED_MAP_H

#include <iostream>
using namespace std;

template <class T>
class addressed_map {

  private:
    T* mList;
    int mSize;
    int mCapacity;
    int mMinKey;
    bool mMinKeyAssigned;
    void readdress_list(int key) {
      while (key < mMinKey) {
        T* tmpList = new T[mCapacity*2];
        for (int i=0; i < mCapacity; i++) {
          tmpList[i+mCapacity] = mList[i];
        }

        delete [] mList;

        mList = tmpList;
        mMinKey -= mCapacity;
        mCapacity *= 2;
      }
    }

    void grow_list(int key) {
      while ( (key - mMinKey) >= mCapacity ) {
        T* tmpList = new T[mCapacity*2];
        for (int i=0; i < mCapacity; i++) {
          tmpList[i] = mList[i];
        }
        delete [] mList;
        mList = tmpList;
        mCapacity *= 2;
      }
    }
  public:

    addressed_map() {
      mSize = 0;
      mMinKeyAssigned = false;
      mCapacity = 32;
      mList = new T[mCapacity];
    }

    ~addressed_map() {
      delete [] mList;
    }

    void insert(int key, T& obj) {
      if (!mMinKeyAssigned) {
        mMinKey = key; 
        mMinKeyAssigned = true;
      }
      if (key < mMinKey) {
        readdress_list(key);
      } else if ( (key - mMinKey) >= mCapacity) {
        grow_list(key);
      }

      mList[key - mMinKey] = obj;
      mSize++;
    }

    T* find(int key) {
      if ( (key < mMinKey) || ((key - mMinKey) >= mCapacity) ) {
        return end();
      } else {

        T* ptr = mList + (key - mMinKey);
        if ((*ptr) == T()){
          return end();
        } else {
          return ptr;
        }
      }
    }

    T* end() {
      return mList + mCapacity;
    }

    size_t size() { return mSize;}
    size_t capacity() { return mCapacity;}

};

#endif /* ADDRESSED_MAP_H */
