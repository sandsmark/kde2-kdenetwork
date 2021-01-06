#ifndef SIMPLELIST_H
#define SIMPLELIST_H

#include "mystring.h"

template <class T>
struct TemplNode
{
   TemplNode(const T& text)
      :m_item(text),m_next(0) {};
   T m_item;
   TemplNode* m_next;
};


template <class T>
class SimpleList
{
   public:
      SimpleList();
      ~SimpleList();
      void append(const T& item);
      void clear();
      int size();
      T* first();
      T* next();
      void removeFirst();
      void remove(T* item);
   protected:
      TemplNode<T>* m_list;
      TemplNode<T>* m_current;
      TemplNode<T>* m_last;
      int m_size;
};

#include "tcpnode.h"
#include "client.h"
#include "mapentry.h"

template class SimpleList<int>;
template class SimpleList<MyString>;
template class SimpleList<Node>;
template class SimpleList<Client>;
template class SimpleList<MapEntry>;

#endif

