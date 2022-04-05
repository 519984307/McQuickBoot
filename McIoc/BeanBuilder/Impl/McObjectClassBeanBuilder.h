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
#pragma once

#include "McObjectBeanBuilder.h"

MC_FORWARD_DECL_PRIVATE_DATA(McObjectClassBeanBuilder)

class MC_IOC_EXPORT McObjectClassBeanBuilder : public McObjectBeanBuilder
{
public:
    struct ConstructorArg
    {
        int index{-1};
        QByteArray name;
        QVariant value;
    };

    McObjectClassBeanBuilder() noexcept;
    ~McObjectClassBeanBuilder();

    McMetaType metaType() const noexcept;
    void setMetaType(const McMetaType &type) noexcept;
    void setClassName(const QByteArray &className) noexcept;
    void addConstructorArg(int index, const QVariant &val) noexcept;
    void addConstructorArg(const QByteArray &name, const QVariant &val) noexcept;

    bool isPointer() const noexcept override;

protected:
    QVariant create() noexcept override;

private:
    QVariant createByMetaType() noexcept;
    QVariant createByMetaObject() noexcept;

private:
    MC_DECL_PRIVATE(McObjectClassBeanBuilder)
};

MC_DECL_POINTER(McObjectClassBeanBuilder)