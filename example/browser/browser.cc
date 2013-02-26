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

#include <QtGui/QtGui>
#include <QtWebKit/QtWebKit>

typedef QtWrapper::Widget<QWidget> Widget;
typedef QtWrapper::Label<QLabel> Label;
typedef QtWrapper::LineEdit<QLineEdit> LineEdit;
typedef QtWrapper::Label<QPushButton> PushButton;

struct WebView : QtWrapper::Widget<QWebView>  {
    template<typename... Args> WebView(Args &&...args) : QtWrapper::Widget<QWebView>(std::forward<Args>(args)...) {

        //Because the property don't have a NOTIFY signals :-(
        title.bindToSignal(metaObject()->indexOfSignal("titleChanged(QString)"));
        icon.bindToSignal(metaObject()->indexOfSignal("iconChanged()"));
        url.bindToSignal(metaObject()->indexOfSignal("urlChanged()"));
    }
    property_qobject<QString> title{ this, "title" };
    property_qobject<QIcon> icon{ this, "icon" };
    property_qobject<QString> url { this, "url" };
};


struct BrowserWindow : Widget {
    LineEdit urlBar { this };
    PushButton nextButton { tr("Next") , this };
    PushButton prevButton { tr("Previous") , this };
    WebView webview { this };
    property_qobject<QString> windowTitle { this, "windowTitle" };
    property_qobject<QIcon> windowIcon { this, "windowIcon" };

    ::property<int> margin {10};

    BrowserWindow() {
        urlBar.geometry = [&](){ return QRect(margin, margin, 
                                              prevButton.geometry().left() - 2*margin ,  
                                              urlBar.sizeHint().height()); };
        nextButton.geometry = [&](){ return QRect(QPoint(geometry().width() - 2*margin -  nextButton.sizeHint().width(), margin),
                                                  nextButton.sizeHint()); };
        prevButton.geometry = [&](){ return QRect(QPoint(nextButton.geometry().x() -  prevButton.sizeHint().width() - margin, margin),
                                                  prevButton.sizeHint()); };

        webview.geometry = [&]() {
            int x = urlBar.geometry().bottom() + margin;
            return QRect(margin, x, width() - 2*margin, geometry().height() - x - margin); };

        webview.url =[&](){ return urlBar.text(); };
        //urlBar.text =[&](){ return webview.url(); }; //Well , that's a loop
        windowTitle = [&]() { return webview.title(); };
        windowIcon = [&]() { return webview.icon(); }; }
};

int main(int argc, char **argv)
{
    QApplication app(argc,argv);

    BrowserWindow window;
    window.show();
    
    return app.exec();
}

// g++ -std=c++11 -I../../src -lQtCore -lQtGui -lQtWebKit ./browser.cc
