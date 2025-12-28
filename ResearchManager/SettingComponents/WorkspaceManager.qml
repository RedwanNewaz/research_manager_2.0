import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Dialogs

Rectangle {
    id: mangeRect
    // width: parent.width - 220
    // height: parent.height - 220

    // anchors.left: sharedFolderBtn.right
    // anchors.right: parent.right
    anchors.fill: parent
    color: "#282828"

    property int selectedRow: -1

    // Confirmation Dialog
    MessageDialog {
        id: deleteConfirmDialog
        title: "Delete Workspace"
        text: "Are you sure you want to delete this workspace?"
        informativeText: "This action cannot be undone. The workspace entry will be removed from the database."
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: {
            if (selectedRow >= 0) {
                wsModel.deleteWorkspace(selectedRow)
                selectedRow = -1
            }
        }
    }

    // Header View
    HorizontalHeaderView {
        id: horizontalHeader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 40
        syncView: tableView
        clip: true

        delegate: Rectangle {
            implicitWidth: 170
            implicitHeight: 40
            color: "#3a3a3a"
            border.color: "#bbb"

            Text {
                text: display
                anchors.centerIn: parent
                color: "white"
                font.bold: true
            }
        }
    }

    // Table View
    TableView {
        id: tableView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: horizontalHeader.bottom
        anchors.bottom: parent.bottom
        clip: true
        model: wsModel

        columnWidthProvider: function(column) {
               return tableView.width / wsModel.columnCount()
        }

        delegate: Rectangle {
            id: cellDelegate
            implicitWidth: 170
            implicitHeight: 60
            border.color: "#bbb"

            property bool isEditing: false

            Text {
                id: cellText
                text: display
                anchors.fill: parent
                anchors.margins: 5
                color: "black"
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                visible: !cellDelegate.isEditing
            }

            TextField {
                id: cellEditor
                anchors.fill: parent
                anchors.margins: 5
                text: display
                visible: cellDelegate.isEditing
                verticalAlignment: Text.AlignVCenter

                onEditingFinished: {
                    wsModel.setData(wsModel.index(row, column), text, Qt.EditRole)
                    cellDelegate.isEditing = false
                }

                onActiveFocusChanged: {
                    if (!activeFocus) {
                        cellDelegate.isEditing = false
                    }
                }

                Component.onCompleted: {
                    if (visible) {
                        forceActiveFocus()
                        selectAll()
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onDoubleTapped: {
                    cellDelegate.isEditing = true
                }
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: {
                    selectedRow = row
                    contextMenu.popup()
                }
            }

            Menu {
                id: contextMenu
                MenuItem {
                    text: "Edit"
                    onTriggered: {
                        cellDelegate.isEditing = true
                    }
                }
                MenuItem {
                    text: "Delete Workspace"
                    onTriggered: {
                        deleteConfirmDialog.open()
                    }
                }
            }
        }
    }
}
