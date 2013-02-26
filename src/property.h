/* Copyright (C) 2013 Olivier Goffart <ogoffart@woboq.com>
   http://woboq.com/blog/property-bindings-in-cpp.html

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <functional>
#include <unordered_set>
#include <type_traits>

class property_base
{
  /* Set of properties which are subscribed to this one.
     When this property is changed, subscriptions are refreshed */
  std::unordered_set<property_base *> subscribers;

  /* Set of properties this property is depending on. */
  std::unordered_set<property_base *> dependencies;

public:
  virtual ~property_base()
  { clearSubscribers(); clearDependencies(); }

  // re-evaluate this property
  virtual void evaluate() = 0;

  property_base() = default;
  property_base(const property_base &other) : dependencies(other.dependencies) {
    for (property_base *p : dependencies)
      p->subscribers.insert(this);
  }

protected:
  /* This function is called by the derived class when the property has changed
     The default implementation re-evaluates all the property subscribed to this one. */
  virtual void notify() {
    auto copy = subscribers;
    for (property_base *p : copy) {
      p->evaluate();
    }
  }

  /* Derived class call this function whenever this property is accessed.
     It register the dependencies. */
  void accessed() {
    if (current && current != this) {
      subscribers.insert(current);
      current->dependencies.insert(this);
    }
  }

  void clearSubscribers() {
      for (property_base *p : subscribers)
          p->dependencies.erase(this);
      subscribers.clear();
  }
  void clearDependencies() {
      for (property_base *p : dependencies)
          p->subscribers.erase(this);
      dependencies.clear();
  }

  /* Helper class that is used on the stack to set the current property being evaluated */
  struct evaluation_scope {
    evaluation_scope(property_base *prop) : previous(current) {
      current = prop;
    }
    ~evaluation_scope() { current = previous; }
    property_base *previous;
  };
private:
  friend struct evaluation_scope;
  /* thread_local */ static property_base *current;
};

//FIXME move to .cpp file
property_base *property_base::current = 0;

/** The property class represents a property of type T that can be assigned a value, or a bindings.
    When assigned a bindings, the binding is re-evaluated whenever one of the property used in it
    is changed */
template <typename T>
struct property : property_base {
  typedef std::function<T()> binding_t;

  property() = default;
  property(const T &t) : value(t) {}
  property(const binding_t &b) : binding(b) { evaluate(); }

  void operator=(const T &t) {
      value = t;
      clearDependencies();
      notify();
  }
  void operator=(const binding_t &b) {
      binding = b;
      evaluate();
  }

  //make it possible to initialize directly with lamda without any casts
  template<typename B> property(const B &b, typename std::enable_if<std::is_constructible<binding_t, B>::value, int*>::type = nullptr) : property(binding_t(b)) {}
  template<typename B> typename std::enable_if<std::is_constructible<binding_t, B>::value>::type operator= (const B &b) { *this=binding_t(b); }

  const T &get() const {
    const_cast<property*>(this)->accessed();
    return value;
  }

  //automatic conversions
  const T &operator()() const { return get();  }
  operator const T&() const { return get(); }

  void evaluate() override {
    if (binding) {
      clearDependencies();
      evaluation_scope scope(this);
      value = binding();
    }
    notify();
  }

protected:
  T value;
  binding_t binding;
};

template<typename T>
struct property_hook : property<T> {
  typedef std::function<void()> hook_t;
  typedef typename property<T>::binding_t binding_t;
  void notify() override {
    property<T>::notify();
    hook();
  }
  property_hook(hook_t h) : hook(h) { }
  property_hook(hook_t h, const T &t) : property<T>(t), hook(h) { }
  property_hook(hook_t h, binding_t b) : property<T>(b), hook(h) { }
  using property<T>::operator=;
private:
  hook_t hook;
};

/** property_wrapper do not own the property, but use a getter and a setter */
template <typename T>
struct property_wrapper : property_base {
  typedef std::function<T()> binding_t;
  typedef std::function<void(const T&)> write_hook_t;
  typedef std::function<T()> read_hook_t;
  explicit property_wrapper(write_hook_t w, read_hook_t r) : write_hook(std::move(w)), read_hook(std::move(r)) { }

  T get() const {
    const_cast<property_wrapper*>(this)->accessed();
    return read_hook();
  }
  void operator=(const T &t) {
    write_hook(t);
    notify();
  }

  void operator=(const binding_t &b) {
    binding = b;
    evaluate();
  }

  T operator()() const { return get(); }
  operator T() const { return get(); }

  using property_base::notify;
protected:
  void evaluate() override {
    if (binding) {
      clearDependencies();
      evaluation_scope scope(this);
      write_hook(binding());
    }
    notify();
  }
private:
  const write_hook_t write_hook;
  const read_hook_t read_hook;
  binding_t binding;
};
