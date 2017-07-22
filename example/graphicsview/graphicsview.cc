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

#include "property.h"
#include "qobject_wrappers.h"

#include <QtWidgets/QtWidgets>

typedef QtWrapper::Widget<QWidget> Widget;
typedef QtWrapper::AbstractSlider<QSlider> Slider;
typedef QtWrapper::Widget<QGraphicsView> GraphicsView;
typedef QtWrapper::Label<QLabel> Label;
typedef QtWrapper::LineEdit<QLineEdit> LineEdit;

struct GraphicsRectObject : QGraphicsWidget {
  // bind the QObject properties.
  property_qobject<QRectF> geometry { this, "geometry" };
  property_qobject<qreal> opacity { this, "opacity" };
  property_qobject<qreal> rotation { this, "rotation" };

  // add a color property, with a hook to update when it changes
  property_hook<QColor> color { [this]{ this->update(); } };
private:
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) override {
    painter->setBrush(color());
    painter->drawRect(boundingRect());
  }
};


struct MyWindow : Widget {
  LineEdit colorEdit {this};

  Slider rotationSlider {Qt::Horizontal, this};
  Slider opacitySlider {Qt::Horizontal, this};

  QGraphicsScene scene;
  GraphicsView view {&scene, this};
  GraphicsRectObject rectangle;

  ::property<int> margin {10};

  MyWindow() {
    // Layout the items.  Not really as good as real layouts, but it demonstrates bindings
    colorEdit.geometry = [&]{ return QRect(margin, margin,
                                             geometry().width() - 2*margin,
                                             colorEdit.sizeHint().height()); };
    rotationSlider.geometry = [&]{ return QRect(margin,
                                                  colorEdit.geometry().bottom() + margin,
                                                  geometry().width() - 2*margin,
                                                  rotationSlider.sizeHint().height()); };
    opacitySlider.geometry = [&]{ return QRect(margin,
                                                 rotationSlider.geometry().bottom() + margin,
                                                 geometry().width() - 2*margin,
                                                 opacitySlider.sizeHint().height()); };
    view.geometry = [&]{
        int x = opacitySlider.geometry().bottom() + margin;
        return QRect(margin, x, width() - 2*margin, geometry().height() - x - margin); 
    };

    // Some proper default value
    colorEdit.text = QString("blue");
    rotationSlider.minimum = -180;
    rotationSlider.maximum = 180;
    opacitySlider.minimum = 0;
    opacitySlider.maximum = 100;
    opacitySlider.value = 100;

    scene.addItem(&rectangle);

    // now the 'cool' bindings
    rectangle.color = [&]{ return QColor(colorEdit.text);  };
    rectangle.opacity = [&]{ return qreal(opacitySlider.value/100.); };
    rectangle.rotation = [&]{ return rotationSlider.value(); };
  }
};

int main(int argc, char **argv)
{
    QApplication app(argc,argv);
    MyWindow window;
    window.show();
    return app.exec();
}

// c++ -std=c++11 -I../../src -lQt5Core -lQt5Gui -lQt5Widgets -fPIC -I/usr/include/qt ./graphicsview.cc
