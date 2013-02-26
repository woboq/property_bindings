/* Copyright (C) 2013 Olivier Goffart <ogoffart@woboq.com>

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

#include <iostream>
#include "property.h"

int calculateArea(int width, int height) {
  return (width * height) * 0.5;
}

struct rectangle {
  property<rectangle*> parent = nullptr;
  property<int> width = 150;
  property<int> height = 75;
  property<int> area = [&]{ return calculateArea(width, height); };

  property<std::string> color = [&]{
    if (parent && area > parent()->area)
      return std::string("blue");
    else
      return std::string("red");
  };
};

int main() {
  rectangle parent;
  rectangle child;
  child.parent = &parent;
  parent.width = 2;
  std::cout << child.color() << std::endl;
  //outputs "red"
}

