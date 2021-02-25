#include "SafeVector.h"


template<class T> SafeVector<T>::SafeVector() {

}

template<class T> SafeVector<T>::~SafeVector() {

}

template<class T> void SafeVector<T>::push_value(T t) {
  pthread_mutex_lock(&mutex_list);
  elems.push_back(t);
  pthread_mutex_unlock(&mutex_list);
}

template<class T> T SafeVector<T>::pop_value() {
  T t;
  pthread_mutex_lock(&mutex_list);
  t = elems.front();
  pthread_mutex_unlock(&mutex_list);
  return t;
}


template<class T> int SafeVector<T>::getSize(){
    return elems.size();
}


