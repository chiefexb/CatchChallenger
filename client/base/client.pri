include(../tiled/tiled.pri)

LIBS += -lvlc
QT       += script opengl widgets qml quick

win32:CONFIG   += console

SOURCES += $$PWD/Api_client_virtual.cpp \
    $$PWD/DatapackChecksum.cpp \
    $$PWD/Api_protocol.cpp \
    $$PWD/Bot/FakeBot.cpp \
    $$PWD/render/TileLayerItem.cpp \
    $$PWD/render/ObjectGroupItem.cpp \
    $$PWD/render/MapVisualiserPlayer.cpp \
    $$PWD/render/MapVisualiser.cpp \
    $$PWD/render/MapVisualiser-map.cpp \
    $$PWD/render/MapObjectItem.cpp \
    $$PWD/render/MapItem.cpp \
    $$PWD/Map_client.cpp \
    $$PWD/interface/BaseWindow.cpp \
    $$PWD/interface/BaseWindowCharacter.cpp \
    $$PWD/interface/BaseWindowCity.cpp \
    $$PWD/interface/BaseWindowClan.cpp \
    $$PWD/interface/BaseWindowFactory.cpp \
    $$PWD/interface/BaseWindowLoad.cpp \
    $$PWD/interface/BaseWindowMarket.cpp \
    $$PWD/interface/BaseWindowOptions.cpp \
    $$PWD/interface/BaseWindowShop.cpp \
    $$PWD/interface/BaseWindowWarehouse.cpp \
    $$PWD/interface/MapControllerMP.cpp \
    $$PWD/interface/ListEntryEnvolued.cpp \
    $$PWD/../crafting/interface/MapControllerCrafting.cpp \
    $$PWD/../crafting/interface/QmlInterface/CraftingAnimation.cpp \
    $$PWD/../fight/interface/QmlInterface/QmlMonsterGeneralInformations.cpp \
    $$PWD/../fight/interface/QmlInterface/EvolutionControl.cpp \
    $$PWD/QmlInterface/AnimationControl.cpp \
    $$PWD/interface/MapController.cpp \
    $$PWD/interface/DatapackClientLoader.cpp \
    $$PWD/../crafting/interface/DatapackClientLoaderCrafting.cpp \
    $$PWD/../fight/interface/DatapackClientLoaderFight.cpp \
    $$PWD/../crafting/interface/BaseWindowCrafting.cpp \
    $$PWD/Options.cpp \
    $$PWD/../fight/interface/BaseWindowFight.cpp \
    $$PWD/interface/Chat.cpp \
    $$PWD/../fight/interface/ClientFightEngine.cpp \
    $$PWD/../fight/interface/MapVisualiserPlayerWithFight.cpp \
    $$PWD/interface/WithAnotherPlayer.cpp \
    $$PWD/render/MapVisualiserThread.cpp \
    $$PWD/render/MapVisualiserOrder.cpp \
    $$PWD/interface/QuestJS.cpp \
    $$PWD/interface/GetPrice.cpp \
    $$PWD/LanguagesSelect.cpp \
    $$PWD/interface/NewProfile.cpp \
    $$PWD/interface/NewGame.cpp \
    $$PWD/InternetUpdater.cpp \
    $$PWD/ExtraSocket.cpp \
    $$PWD/LocalListener.cpp \
    $$PWD/Audio.cpp \
    $$PWD/interface/MapDoor.cpp \
    $$PWD/interface/TriggerAnimation.cpp \
    $$PWD/interface/TemporaryTile.cpp \
    $$PWD/render/PreparedLayer.cpp \
    $$PWD/render/MapMark.cpp \
    $$PWD/interface/PathFinding.cpp

HEADERS  += $$PWD/ClientStructures.h \
    $$PWD/DatapackChecksum.h \
    $$PWD/Api_client_virtual.h \
    $$PWD/Api_protocol.h \
    $$PWD/Bot/FakeBot.h \
    $$PWD/render/TileLayerItem.h \
    $$PWD/render/ObjectGroupItem.h \
    $$PWD/render/MapVisualiserPlayer.h \
    $$PWD/render/MapVisualiser.h \
    $$PWD/render/MapObjectItem.h \
    $$PWD/render/MapItem.h \
    $$PWD/Map_client.h \
    $$PWD/interface/BaseWindow.h \
    $$PWD/interface/MapControllerMP.h \
    $$PWD/interface/ListEntryEnvolued.h \
    $$PWD/interface/MapController.h \
    $$PWD/interface/DatapackClientLoader.h \
    $$PWD/../crafting/interface/QmlInterface/CraftingAnimation.h \
    $$PWD/../fight/interface/QmlInterface/QmlMonsterGeneralInformations.h \
    $$PWD/../fight/interface/QmlInterface/EvolutionControl.h \
    $$PWD/QmlInterface/AnimationControl.h \
    $$PWD/ClientVariable.h \
    $$PWD/Options.h \
    $$PWD/interface/Chat.h \
    $$PWD/../fight/interface/ClientFightEngine.h \
    $$PWD/../fight/interface/MapVisualiserPlayerWithFight.h \
    $$PWD/interface/WithAnotherPlayer.h \
    $$PWD/render/MapVisualiserThread.h \
    $$PWD/render/MapVisualiserOrder.h \
    $$PWD/interface/QuestJS.h \
    $$PWD/interface/GetPrice.h \
    $$PWD/LanguagesSelect.h \
    $$PWD/interface/NewProfile.h \
    $$PWD/interface/NewGame.h \
    $$PWD/InternetUpdater.h \
    $$PWD/ExtraSocket.h \
    $$PWD/LocalListener.h \
    $$PWD/DisplayStructures.h \
    $$PWD/Audio.h \
    $$PWD/interface/MapDoor.h \
    $$PWD/interface/TriggerAnimation.h \
    $$PWD/interface/TemporaryTile.h \
    $$PWD/render/PreparedLayer.h \
    $$PWD/render/MapMark.h \
    $$PWD/interface/PathFinding.h

FORMS    += $$PWD/interface/BaseWindow.ui \
    $$PWD/interface/Chat.ui \
    $$PWD/interface/WithAnotherPlayer.ui \
    $$PWD/interface/GetPrice.ui \
    $$PWD/LanguagesSelect.ui \
    $$PWD/interface/NewProfile.ui \
    $$PWD/interface/NewGame.ui

win32:RC_FILE += $$PWD/resources/resources-windows.rc
ICON = $$PWD/resources/client.icns
macx:INCLUDEPATH += /usr/local/include/
macx:LIBS += -L/usr/local/lib/

RESOURCES += $$PWD/resources/client-resources.qrc \
    $$PWD/../crafting/resources/client-resources-plant.qrc \
    $$PWD/../fight/resources/client-resources-fight.qrc

TRANSLATIONS    = $$PWD/resources/languages/en/translation.ts \
    $$PWD/languages/fr/translation.ts
