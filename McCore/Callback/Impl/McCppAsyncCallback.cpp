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
#include "McCppAsyncCallback.h"

MC_INIT(McCppAsyncCallback)
mcRegisterMetaTypeSimple<McCppAsyncCallback>();
MC_INIT_END

McCppAsyncCallback::McCppAsyncCallback(const McCppAsyncCallback &o) noexcept
    : m_callback(o.m_callback)
{
}

McCppAsyncCallback &McCppAsyncCallback::operator=(const McCppAsyncCallback &o) noexcept
{
    McCppAsyncCallback copy(o);
    std::swap(m_callback, copy.m_callback);
    return *this;
}

McCppAsyncCallback::McCppAsyncCallback(McCppAsyncCallback &&o) noexcept
    : m_callback(o.m_callback)
{
    o.m_callback.reset();
}

McCppAsyncCallback &McCppAsyncCallback::operator=(McCppAsyncCallback &&o) noexcept
{
    McCppAsyncCallback copy(std::move(o));
    std::swap(m_callback, copy.m_callback);
    return *this;
}

void McCppAsyncCallback::call(const QVariantList &varList) const noexcept
{
    if (!m_callback.has_value()) {
        return;
    }
    m_callback->call(varList);
}
