#pragma once

#include <functional>

//////////////////////////////////////////////////////////////////////////
// Scoped RAII class.
//////////////////////////////////////////////////////////////////////////


// Call deleter on pointer on exit scope
template<typename T, typename F>
class scoped_ptr
{
public:
   scoped_ptr(T* ptr, F action) : ptr_(ptr), action_(std::move(action)) {}
   ~scoped_ptr() { release(); }
   void release() 
   { 
      if (ptr_)
         action_(ptr_); 
      ptr_ = nullptr;
   }
   // automatic conversion to underlying pointer
   operator T*() { return ptr_; }
   T* operator ->() { return ptr_; }

private:
   T * ptr_ = nullptr;
   F action_;
};

template<typename T, typename Fun>
auto make_scoped_ptr(T* ptr, Fun&& fun) { return scoped_ptr<T, Fun>(ptr, std::forward<Fun>(fun)); }
