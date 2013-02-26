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

#include "property.h"

#include <QtCore/QObject>
#include <QtCore/qmetaobject.h>

class property_qobject_base : public property_base,  private QObject {

    QObject *obj;
    QMetaProperty prop;

public:
    property_qobject_base(QObject *o, const char *p) : obj(o) {
        const QMetaObject *mo = o->metaObject();
        prop = mo->property(mo->indexOfProperty(p));
        int idx = prop.notifySignalIndex();
        if (idx >= 0)
            bindToSignal(idx);
    }

    void bindToSignal(int signalIndex) {
        QMetaObject::connect(obj, signalIndex, this, staticMetaObject.methodCount());
    }
private:
    virtual int qt_metacall(QMetaObject::Call c, int idx, void** a) {
        idx = QObject::qt_metacall(c, idx, a);
        if (idx < 0)
            return idx;
        notify();
        return idx;
    }

protected:
  //  virtual void notify() = 0;
    QVariant getProperty() { accessed(); return prop.read(obj); }
    bool setProperty(const QVariant &v) { return prop.write(obj, v);  }
};

template <typename T> class property_qobject :
    public property_qobject_base {

public:
    property_qobject(::QObject *o, const char *p) : property_qobject_base(o, p) {}

    typedef std::function<T()> binding_t;

    void operator=(const T &t) {
        setProperty(QVariant::fromValue<T>(t));
        clearDependencies();
        //notify();
        //return *this;
    }
    void operator=(const binding_t &b) {
        binding = b;
        evaluate();
        //return *this;
    }

    T get() {
        return qvariant_cast<T>(getProperty());
    }

    void evaluate()
    {
        T data;
        {
            evaluation_scope scope(this);
            data = binding();
        }
        setProperty(QVariant::fromValue<T>(data));
    }

    T operator->() { return get(); }
    T operator()() { return get(); }
    operator T() { return get(); }

protected:
    binding_t binding;
};

