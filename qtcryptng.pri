QT += core

CONFIG += c++11

HEADERS += $$PWD/include/config.h \
    $$PWD/include/crypto.h \
    $$PWD/include/openssl_symbols.h \
    $$PWD/include/qtng_temp.h \
    $$PWD/qtsslng.h \
    $$PWD/include/md.h \
    $$PWD/include/random.h \
    $$PWD/include/cipher.h \
    $$PWD/include/pkey.h \
    $$PWD/include/crypto_p.h \
    $$PWD/include/certificate.h \
    $$PWD/include/qasn1element.h

SOURCES += $$PWD/src/crypto.cpp \
    $$PWD/src/openssl_symbols.cpp \
    $$PWD/src/random.cpp \
    $$PWD/src/md.cpp \
    $$PWD/src/pkey.cpp \
    $$PWD/src/cipher.cpp \
    $$PWD/src/certificate.cpp \
    $$PWD/src/qasn1element.cpp

