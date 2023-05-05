include(settings/settings.pro)

TEMPLATE = subdirs

SUBDIRS += \
    utils \
    evaluation \
    aipenserver \
    mainframe \
    plugins

aipenserver.depends = utils

plugins.depends = utils

evaluation.depends = utils

mainframe.depends = utils \
                    evaluation \
                    aipenserver

