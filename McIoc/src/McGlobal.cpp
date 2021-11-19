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
#include "McIoc/McGlobal.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QMetaClassInfo>
#include <QMetaObject>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

#include "McIoc/ApplicationContext/impl/McAnnotationApplicationContext.h"
#include "McIoc/BeanDefinition/IMcBeanDefinition.h"

Q_LOGGING_CATEGORY(mcIoc, "mc.ioc")

namespace McPrivate {

using StartUpFuncs = QMap<int, QVector<Mc::StartUpFunction>>;
MC_GLOBAL_STATIC(StartUpFuncs, preRFuncs)
using VFuncs = QMap<int, QVector<Mc::CleanUpFunction>>;
MC_GLOBAL_STATIC(VFuncs, postRFuncs)

int iocStaticInit()
{
    qAddPreRoutine([]() { Mc::callPreRoutine(); });
    qAddPostRoutine([]() {
        VFuncs funcs;
        funcs.swap(*postRFuncs);
        auto keys = funcs.keys();
        for(int i = keys.length() - 1; i >= 0; --i) {
            auto list = funcs.value(keys.value(i));
            for(int j = 0; j < list.length(); ++j) {
                list.at(j)();
            }
        }
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        Q_QGS_postRFuncs::guard.store(QtGlobalStatic::Initialized);
#else
        Q_QGS_postRFuncs::guard.storeRelaxed(QtGlobalStatic::Initialized);
#endif
    });
    return 1;
}
#ifndef MC_MANUAL_ENABLE_IOC
#ifdef Q_CC_GNU
static void __attribute__((constructor)) __iocStaticInit__(void)
{
    iocStaticInit();
}
#elif Q_CC_MSVC
#include <corecrt_startup.h>
#define SECNAME ".CRT$XCY"
#pragma section(SECNAME, long, read)
int __iocStaticInit__()
{
    iocStaticInit();
    return 0;
}
__declspec(allocate(SECNAME)) _PIFV dummy[] = {__iocStaticInit__};
#else
Q_CONSTRUCTOR_FUNCTION(iocStaticInit)
#endif
#endif

MC_STATIC(Mc::RoutinePriority::Max + 10)
auto pId = qRegisterMetaType<QObject *>();
auto sId = qRegisterMetaType<QObjectPtr>();
McMetaTypeId::addQObjectPointerIds(pId, sId);
McMetaTypeId::addSharedPointerId(sId, pId);
qRegisterMetaType<QObjectPtr>(MC_STRINGIFY(QObjectPtr));
qRegisterMetaType<QObjectPtr>(MC_STRINGIFY(QObjectConstPtrRef));
MC_STATIC_END

QString getBeanName(const QMetaObject *metaObj) noexcept
{
    QString beanName;
    auto beanNameIndex = metaObj->indexOfClassInfo(MC_BEANNAME_TAG);
    auto func = [&beanName, metaObj]() {
        beanName = metaObj->className();
        auto str = QLatin1String("::");
        auto index = beanName.lastIndexOf(str);
        if (index != -1) {
            beanName = beanName.mid(index + str.size());
        }
        Q_ASSERT(!beanName.isEmpty());
        auto firstChar = beanName.at(0);
        firstChar = firstChar.toLower();
        beanName.remove(0, 1);
        beanName = firstChar + beanName;
    };
    if(beanNameIndex == -1) {
        func();
    } else {
        auto beanNameClassInfo = metaObj->classInfo(beanNameIndex);
        beanName = beanNameClassInfo.value();
        if (beanName.isEmpty()) {
            func();
        }
    }
    return beanName;
}

} // namespace McPrivate

McCustomEvent::~McCustomEvent() noexcept
{
}

namespace {
QString getStandardPath(QStandardPaths::StandardLocation type)
{
    auto paths = QStandardPaths::standardLocations(type);
    if (paths.isEmpty()) {
        return QString();
    }
    return paths.first();
}
} // namespace

MC_GLOBAL_STATIC_BEGIN(iocGlobalStaticData)
QString applicationDirPath;
QString applicationName;
QHash<QString, std::function<QString()>> pathPlaceholders{
    {QLatin1String("{desktop}"), std::bind(getStandardPath, QStandardPaths::DesktopLocation)},
    {QLatin1String("{documents}"), std::bind(getStandardPath, QStandardPaths::DocumentsLocation)},
    {QLatin1String("{temp}"), std::bind(getStandardPath, QStandardPaths::TempLocation)},
    {QLatin1String("{home}"), std::bind(getStandardPath, QStandardPaths::HomeLocation)},
    {QLatin1String("{appLocalData}"), std::bind(getStandardPath, QStandardPaths::AppLocalDataLocation)},
    {QLatin1String("{data}"), std::bind(getStandardPath, QStandardPaths::AppLocalDataLocation)},
    {QLatin1String("{cache}"), std::bind(getStandardPath, QStandardPaths::CacheLocation)},
    {QLatin1String("{config}"), std::bind(getStandardPath, QStandardPaths::GenericConfigLocation)},
    {QLatin1String("{appData}"), std::bind(getStandardPath, QStandardPaths::AppDataLocation)},
};
MC_GLOBAL_STATIC_END(iocGlobalStaticData)

namespace Mc {

#ifdef MC_MANUAL_ENABLE_IOC
namespace Ioc {
void init() noexcept
{
    McPrivate::iocStaticInit();
}
} // namespace Ioc
#endif

bool waitForExecFunc(const std::function<bool()> &func, qint64 timeout) noexcept 
{
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(100);
    QElapsedTimer stopWatcher;
    bool ret = true;
    
    QObject::connect(&timer, &QTimer::timeout, [&timer, &stopWatcher, &loop, &ret, &func, &timeout](){
        timer.stop();
        if((ret = func())) {
            loop.quit();
            return;
        }
        if(timeout != -1 && stopWatcher.elapsed() > timeout) {
            loop.quit();
            return;
        }
        timer.start();
    });
    
    stopWatcher.start();
    timer.start();
    loop.exec();
    return ret;
}

void registerPathPlaceholder(const QString &placeholder, const std::function<QString()> &func) noexcept
{
    iocGlobalStaticData->pathPlaceholders.insert(placeholder, func);
}

QString toAbsolutePath(const QString &inPath) noexcept
{
    QStringList pathPlhKeys = iocGlobalStaticData->pathPlaceholders.keys();
    auto path = inPath;
    for (const auto &key : pathPlhKeys) {
        const auto &value = iocGlobalStaticData->pathPlaceholders.value(key);
        QString plhPath;
        if (path.contains(key) && !(plhPath = value()).isEmpty()) {
            path.replace(key, plhPath);
        }
    }
    QString dstPath = QDir::toNativeSeparators(path);
    if (QDir::isAbsolutePath(dstPath)) {
        return dstPath;
    }
    QString sepDot = ".";
    QString sepDotDot = "..";
    sepDot += QDir::separator();
    sepDotDot += QDir::separator();
    if (dstPath.startsWith(sepDot) || dstPath.startsWith(sepDotDot)) {
        dstPath = Mc::applicationDirPath() + QDir::separator() + dstPath;
    } else {
        QUrl url(path);
        if (url.isLocalFile()) {
            dstPath = url.toLocalFile();
            dstPath = QDir::toNativeSeparators(dstPath);
            if (!QDir::isAbsolutePath(dstPath)) {
                dstPath = Mc::applicationDirPath() + QDir::separator() + dstPath;
            }
            url = QUrl::fromLocalFile(dstPath);
        }
        dstPath = url.toString();
    }
    dstPath = QDir::cleanPath(dstPath);
    return dstPath;
}

QString applicationDirPath() noexcept
{
    if (iocGlobalStaticData->applicationDirPath.isEmpty()) {
        return QCoreApplication::applicationDirPath();
    }
    return iocGlobalStaticData->applicationDirPath;
}

QDir applicationDir() noexcept
{
    return QDir(applicationDirPath());
}

void setApplicationDirPath(const QString &val) noexcept
{
    iocGlobalStaticData->applicationDirPath = val;
}

QString applicationFilePath() noexcept
{
    if (iocGlobalStaticData->applicationName.isEmpty()) {
        return QCoreApplication::applicationFilePath();
    }
    return applicationDir().absoluteFilePath(iocGlobalStaticData->applicationName);
}

void setApplicationFilePath(const QString &val) noexcept
{
    QFileInfo fileInfo(val);
    iocGlobalStaticData->applicationDirPath = fileInfo.absolutePath();
    iocGlobalStaticData->applicationName = fileInfo.fileName();
}

QList<QString> getAllComponent(IMcApplicationContextConstPtrRef appCtx) noexcept
{
    if (!appCtx) {
        qCritical() << "Please call initContainer to initialize container first";
        return QList<QString>();
    }
    QList<QString> components;
    QHash<QString, IMcBeanDefinitionPtr> beanDefinitions = appCtx->getBeanDefinitions();
    for (auto itr = beanDefinitions.cbegin(); itr != beanDefinitions.cend(); ++itr) {
        auto beanDefinition = itr.value();
        if (!isComponent(beanDefinition->getBeanMetaObject()))
            continue;
        components.append(itr.key());
    }
    return components;
}

QList<QString> getComponents(IMcApplicationContextConstPtrRef appCtx, const QString &componentType) noexcept 
{
    if (!appCtx) {
        qCritical() << "Please call initContainer to initialize container first";
        return QList<QString>();
    }
    QList<QString> components;
    QHash<QString, IMcBeanDefinitionPtr> beanDefinitions = appCtx->getBeanDefinitions();
    for (auto itr = beanDefinitions.cbegin(); itr != beanDefinitions.cend(); ++itr) {
        auto beanDefinition = itr.value();
        if (!isComponentType(beanDefinition->getBeanMetaObject(), componentType))
            continue;
        components.append(itr.key());
    }
    return components;
}

bool isComponent(const QMetaObject *metaObj) noexcept
{
    if(!metaObj) {
        return false;
    }
    int classInfoCount = metaObj->classInfoCount();
    for (int i = 0; i < classInfoCount; ++i) {
        auto classInfo = metaObj->classInfo(i);
        if (qstrcmp(classInfo.name(), MC_COMPONENT_TAG) != 0)
            continue;
        return true;
    }
    return false;
}

bool isComponentType(const QMetaObject *metaObj, const QString &type) noexcept 
{
    if(!metaObj) {
        return false;
    }
    int classInfoCount = metaObj->classInfoCount();
    for (int i = 0; i < classInfoCount; ++i) {
        auto classInfo = metaObj->classInfo(i);
        if (qstrcmp(classInfo.name(), MC_COMPONENT_TAG) != 0)
            continue;
        return classInfo.value() == type;
    }
    return false;
}

bool isContainedTag(const QString &tags, const QString &tag) noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    auto tagList = tags.split(' ', QString::SkipEmptyParts);
#else
    auto tagList = tags.split(' ', Qt::SkipEmptyParts);
#endif
    for (auto t : tagList) {
        if (t == tag) {
            return true;
        }
    }
    return false;
}

QObject *getObject(IMcApplicationContext *appCtx, const QString &beanName) noexcept
{
    auto bean = appCtx->getBeanPointer(beanName);
    if (bean == nullptr) {
        bean = appCtx->getBean(beanName).data();
    }
    if (bean == nullptr) {
        qWarning() << "not exists bean:" << beanName;
    }
    return bean;
}

void addPreRoutine(int priority, const StartUpFunction &func) noexcept
{
    McPrivate::StartUpFuncs *funcs = McPrivate::preRFuncs;
    if(!funcs) {
        return;
    }
    (*funcs)[priority].prepend(func);
}

void callPreRoutine() noexcept
{
    using namespace McPrivate;
    StartUpFuncs funcs;
    funcs.swap(*preRFuncs);
    auto keys = funcs.keys();
    for (int i = keys.length() - 1; i >= 0; --i) {
        auto list = funcs.value(keys.value(i));
        for (int j = 0; j < list.length(); ++j) {
            list.at(j)();
        }
    }
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    Q_QGS_preRFuncs::guard.store(QtGlobalStatic::Initialized);
#else
    Q_QGS_preRFuncs::guard.storeRelaxed(QtGlobalStatic::Initialized);
#endif
}

void cleanPreRoutine() noexcept
{
    McPrivate::preRFuncs->clear();
}

void addPostRoutine(int priority, const CleanUpFunction &func) noexcept
{
    McPrivate::VFuncs *funcs = McPrivate::postRFuncs;
    if(!funcs) {
        return;
    }
    (*funcs)[priority].prepend(func);
}

namespace Ioc {

void connect(const QString &beanName,
             const QString &sender,
             const QString &signal,
             const QString &receiver,
             const QString &slot,
             Qt::ConnectionType type) noexcept
{
    McAnnotationApplicationContext::addConnect(beanName, sender, signal, receiver, slot, type);
}

void connect(const QMetaObject *metaObj,
             const QString &sender,
             const QString &signal,
             const QString &receiver,
             const QString &slot,
             Qt::ConnectionType type) noexcept
{
    Q_ASSERT(metaObj != nullptr);
    QString beanName = McPrivate::getBeanName(metaObj);
    connect(beanName, sender, signal, receiver, slot, type);
}

QMetaObject::Connection connect(IMcApplicationContext *appCtx,
                                const QString &sender,
                                const QString &signal,
                                const QString &receiver,
                                const QString &slot,
                                Qt::ConnectionType type) noexcept
{
    Q_ASSERT(appCtx != nullptr);
    auto *receiverObj = appCtx->getBeanPointer<QObject>(receiver);
    if (receiverObj == nullptr) {
        receiverObj = appCtx->getBean<QObject>(receiver).data();
    }
    if (receiverObj == nullptr) {
        qCritical("not exists bean '%s'", qPrintable(receiver));
        return QMetaObject::Connection();
    }
    return connect(appCtx, sender, signal, receiverObj, slot, type);
}

QMetaObject::Connection connect(IMcApplicationContext *appCtx,
                                const QString &sender,
                                const QString &signal,
                                QObject *receiver,
                                const QString &slot,
                                Qt::ConnectionType type) noexcept
{
    Q_ASSERT(appCtx != nullptr);
    Q_ASSERT(receiver != nullptr);
    auto senderObj = appCtx->getBeanPointer<QObject>(sender);
    if (senderObj == nullptr) {
        senderObj = appCtx->getBean<QObject>(sender).data();
    }
    if (senderObj == nullptr) {
        qCritical("not exists bean '%s'", qPrintable(sender));
        return QMetaObject::Connection();
    }
    QString signalStr = signal;
    if (!signalStr.startsWith(QString::number(QSIGNAL_CODE))) {
        signalStr = QString::number(QSIGNAL_CODE) + signalStr;
    }
    auto slotStr = slot;
    if (!slotStr.startsWith(QString::number(QSLOT_CODE))) {
        slotStr = QString::number(QSLOT_CODE) + slotStr;
    }
    return QObject::connect(senderObj,
                            signalStr.toLocal8Bit(),
                            receiver,
                            slotStr.toLocal8Bit(),
                            type);
}

bool disconnect(IMcApplicationContext *appCtx,
                const QString &sender,
                const QString &signal,
                const QString &receiver,
                const QString &slot) noexcept
{
    Q_ASSERT(appCtx != nullptr);
    auto *receiverObj = appCtx->getBeanPointer<QObject>(receiver);
    if (receiverObj == nullptr) {
        receiverObj = appCtx->getBean<QObject>(receiver).data();
    }
    return disconnect(appCtx, sender, signal, receiverObj, slot);
}

bool disconnect(IMcApplicationContext *appCtx,
                const QString &sender,
                const QString &signal,
                QObject *receiver,
                const QString &slot) noexcept
{
    Q_ASSERT(appCtx != nullptr);
    auto senderObj = appCtx->getBeanPointer<QObject>(sender);
    if (senderObj == nullptr) {
        senderObj = appCtx->getBean<QObject>(sender).data();
    }
    if (senderObj == nullptr) {
        qCritical("not exists bean '%s'", qPrintable(sender));
        return false;
    }
    const char *sig = nullptr;
    const char *slt = nullptr;
    QString signalStr = signal;
    if (!signalStr.isEmpty() && !signalStr.startsWith(QString::number(QSIGNAL_CODE))) {
        signalStr = QString::number(QSIGNAL_CODE) + signalStr;
    }
    if (!signalStr.isEmpty()) {
        sig = signalStr.toLocal8Bit();
    }
    auto slotStr = slot;
    if (!slotStr.isEmpty() && !slotStr.startsWith(QString::number(QSLOT_CODE))) {
        slotStr = QString::number(QSLOT_CODE) + slotStr;
    }
    if (!slotStr.isEmpty()) {
        slt = slotStr.toLocal8Bit();
    }
    return QObject::disconnect(senderObj, sig, receiver, slt);
}
} // namespace Ioc
} // namespace Mc
