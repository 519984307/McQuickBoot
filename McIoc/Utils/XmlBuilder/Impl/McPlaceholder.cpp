/*
 * MIT License
 *
 * Copyright (c) 2021 mrcao20
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "McPlaceholder.h"

#include <QXmlStreamWriter>

namespace Mc::XmlBuilder {
MC_DECL_PRIVATE_DATA(Placeholder)
QString placeholder;
MC_DECL_PRIVATE_DATA_END

Placeholder::Placeholder() noexcept
{
    MC_NEW_PRIVATE_DATA(Placeholder);
}

Placeholder::Placeholder(const QString &val) noexcept
    : Placeholder()
{
    d->placeholder = val;
}

Placeholder::~Placeholder() {}

void Placeholder::setPlaceholder(const QString &val) noexcept
{
    d->placeholder = val;
}

void Placeholder::write(QXmlStreamWriter &writer) const noexcept
{
    writer.writeTextElement(Mc::Constant::Tag::Xml::PLH, d->placeholder);
}
} // namespace Mc::XmlBuilder
