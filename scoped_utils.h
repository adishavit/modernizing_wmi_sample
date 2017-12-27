#pragma once

#include <functional>

//////////////////////////////////////////////////////////////////////////
// Simple, std::function (hence, inefficient) based scoped RAII classes.
//////////////////////////////////////////////////////////////////////////


// Call deleter on pointer on exit scope
template<typename T, typename R = void>
class scoped_ptr
{
   T* ptr_ = nullptr;
   std::function<R(T*)> deleter_;
public:
   scoped_ptr() = default;
   scoped_ptr(T* ptr, std::function<R(T*)>&& deleter) : ptr_(ptr), deleter_(deleter) {}
   ~scoped_ptr() { release(); }
   void release() 
   { 
      if (ptr_ && deleter_)
         deleter_(ptr_); 
      deleter_ = {};
   }
   operator T*() { return ptr_; }
   T* operator ->() { return ptr_; }
};

template<typename T, typename Fun>
auto make_scoped_ptr(T* ptr, Fun&& fun) { return scoped_ptr<T, decltype(fun(ptr))>(ptr, fun); }


//////////////////////////////////////////////////////////////////////////

// Invoke callable on exit scope
template<typename R=void>
class scoped_invoke
{
   std::function<R()> fun_;
public:
   scoped_invoke(std::function<R()>&& fun) : fun_(fun) {}
   ~scoped_invoke() { fun_(); }
};


template<typename Fun>
auto make_scoped_invoker(Fun&& fun) { return scoped_invoke<decltype(fun())>(fun); }


//////////////////////////////////////////////////////////////////////////