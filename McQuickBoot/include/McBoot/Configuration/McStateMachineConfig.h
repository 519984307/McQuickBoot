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

#include "../McBootGlobal.h"

MC_FORWARD_DECL_CLASS(QScxmlStateMachine);

MC_FORWARD_DECL_PRIVATE_DATA(McStateMachineConfig);

class McStateMachineConfig : public QObject
{
    Q_OBJECT
    MC_COMPONENT("stateMachineConfig")
    MC_CONFIGURATION_PROPERTIES("boot.application.stateMachine")
    Q_PRIVATE_PROPERTY(d, QStringList paths MEMBER paths)
public:
    Q_INVOKABLE explicit McStateMachineConfig(QObject *parent = nullptr) noexcept;
    ~McStateMachineConfig() override;

    QScxmlStateMachinePtr stateMachine() const noexcept;

private:
    Q_INVOKABLE
    MC_ALL_FINISHED
    void allFinished() noexcept;

private:
    MC_DECL_PRIVATE(McStateMachineConfig)
};

MC_DECL_METATYPE(McStateMachineConfig)
