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

    // Drag-to-reorder state: the row currently being dragged, or -1
    property int draggedRow: -1
    readonly property int rowHeight: 60

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

        rowHeightProvider: function(row) {
               return mangeRect.rowHeight
        }

        delegate: Rectangle {
            id: cellDelegate
            implicitWidth: 170
            implicitHeight: mangeRect.rowHeight

            property bool isEditing: false
            readonly property bool beingDragged: mangeRect.draggedRow === row

            // Lift the row being dragged so it reads as "picked up"
            color: beingDragged ? "#cfe3ff" : "white"
            z: beingDragged ? 2 : 0
            border.width: beingDragged ? 2 : 1
            border.color: beingDragged ? "#3b82f6" : "#bbb"

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

            // Drag a row up or down to change the workspace order.
            // The rows swap live under the pointer and the new order is saved
            // to the database immediately.
            DragHandler {
                id: rowDragHandler
                target: null                    // reorder the model, don't move the cell
                enabled: !cellDelegate.isEditing
                acceptedButtons: Qt.LeftButton
                xAxis.enabled: false
                yAxis.enabled: true
                cursorShape: active ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                onActiveChanged: {
                    mangeRect.draggedRow = active ? row : -1
                }

                onCentroidChanged: {
                    if (!active || mangeRect.draggedRow < 0)
                        return

                    var scenePos = centroid.scenePosition
                    var local = tableView.mapFromItem(null, scenePos.x, scenePos.y)
                    var target = Math.floor((local.y + tableView.contentY) / mangeRect.rowHeight)

                    target = Math.max(0, Math.min(tableView.rows - 1, target))

                    if (target !== mangeRect.draggedRow) {
                        if (wsModel.moveWorkspace(mangeRect.draggedRow, target))
                            mangeRect.draggedRow = target
                    }
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
