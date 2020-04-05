#pragma once

#include <QVariant>

#include "../McBootGlobal.h"

class IMcControllerContainer {
public:
    virtual ~IMcControllerContainer() = default;
    
    virtual QVariant invoke(const QString &uri, const QVariant &body) noexcept = 0;
};

MC_DECL_POINTER(IMcControllerContainer)
