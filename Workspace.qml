/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls
import ResearchManager


Rectangle {
    // width: 1280
    // height: 720
    width: Screen.width
    height: Screen.height -70


    anchors.centerIn: parent

    HomePage{
        id: homepage

        SideMenu{
           id: sideMenuBar
           x: 0
           y: 0
           width: 70
           height: parent.height
           color: "#181818"
        }
    }


}


