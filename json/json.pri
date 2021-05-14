HEADERS += \
    $$PWD/autolink.h \
    $$PWD/config.h \
    $$PWD/features.h \
    $$PWD/forwards.h \
    $$PWD/json.h \
    $$PWD/json_batchallocator.h \
    $$PWD/json_internalarray.inl \
    $$PWD/json_internalmap.inl \
    $$PWD/json_valueiterator.inl \
    $$PWD/reader.h \
    $$PWD/value.h \
    $$PWD/writer.h

SOURCES += \
    $$PWD/json_reader.cpp \
    $$PWD/json_value.cpp \
    $$PWD/json_writer.cpp

DISTFILES += \
    $$PWD/sconscript
