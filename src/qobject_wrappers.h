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


#include "property_qobject.h"

#include <QtWidgets/QWidget>
#include <QtCore/qcoreevent.h>

namespace QtWrapper {

template<class Base> struct Widget : Base {
    template<typename... Args> Widget(Args &&...args) : Base(std::forward<Args>(args)...) {}
    property_wrapper<QRect> geometry{
        [this](const QRect &r){ this->setGeometry(r); },
        [this]() { return this->Base::geometry(); } };

private:
    virtual bool event(QEvent* e) override {
        switch(e->type()) {
            case QEvent::Move:
            case QEvent::Resize:
                geometry.notify();
                break;
            default:
                break;
        }
        return Base::event(e);
    }
};

template<class Base> struct Label : Widget<Base> {
    template<typename... Args> Label(Args &&...args) : Widget<Base>(std::forward<Args>(args)...) {}
    property_qobject<QString> text{ this, "text" };
};

template<class Base> struct LineEdit : Widget<Base> {
    template<typename... Args> LineEdit(Args &&...args) : Widget<Base>(std::forward<Args>(args)...) {}
    property_qobject<QString> text{ this, "text" };
};


template<class Base> struct AbstractSlider : Widget<Base> {
    template<typename... Args> AbstractSlider(Args &&...args) : Widget<Base>(std::forward<Args>(args)...) {}
    property_qobject<int> minimum{ this, "minimum" };
    property_qobject<int> maximum{ this, "maximum" };
    property_qobject<int> value{ this, "value" };
};

}
