QT.ioc.VERSION = 5.15.0
QT.ioc.name = McIoc
QT.ioc.module = McIoc
QT.ioc.libs = $$QT_MODULE_LIB_BASE
QT.ioc.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/McIoc
QT.ioc.bins = $$QT_MODULE_BIN_BASE
QT.ioc.libexecs = $$QT_MODULE_LIBEXEC_BASE
QT.ioc.imports = $$QT_MODULE_IMPORT_BASE
QT.ioc.depends = core xml
QT.ioc.DEFINES = MC_IOC_LIB
QT_MODULES += ioc