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
#include "McObjectPluginBeanBuilder.h"

MC_DECL_PRIVATE_DATA(McObjectPluginBeanBuilder)
QString pluginPath;
MC_DECL_PRIVATE_DATA_END

McObjectPluginBeanBuilder::McObjectPluginBeanBuilder() noexcept
{
    MC_NEW_PRIVATE_DATA(McObjectPluginBeanBuilder);
}

McObjectPluginBeanBuilder::~McObjectPluginBeanBuilder() {}

void McObjectPluginBeanBuilder::setPluginPath(const QString &path) noexcept
{
    d->pluginPath = path;
}

bool McObjectPluginBeanBuilder::isPointer() const noexcept
{
    return true;
}

QVariant McObjectPluginBeanBuilder::create() noexcept
{
    auto obj = Mc::loadPlugin(d->pluginPath);
    if (obj == nullptr) {
        return QVariant();
    }
    const char *className = obj->metaObject()->className();
    setMetaType(McMetaType::fromTypeName(className));
    auto beanStar = obj->qt_metacast(className);
    QVariant beanVar(metaType().pMetaType(), &beanStar);
    return beanVar;
}
