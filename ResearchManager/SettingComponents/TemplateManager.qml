import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Rectangle {
    id: templateManagerRoot
    // width: parent ? parent.width - 220 : 800
    // height: parent ? parent.height - 220 : 600
    anchors.fill: parent
    color: "#282828"

    property bool templatesLoaded: false
    property int selectedItemId: -1
    property string selectedItemText: ""
    property int selectedItemCategoryId: 0

    // Connection to handle operation results
    Connections {
        target: templateModel
        function onOperationCompleted(success, message) {
            statusLabel.text = message
            statusLabel.color = success ? "#27ae60" : "#e74c3c"
            if (success) {
                loadTemplateItems()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        // Header with title and template management buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: "Template Manager"
                font.pixelSize: 24
                font.bold: true
                color: "white"
                Layout.fillWidth: true
            }

            Button {
                id: createTemplateBtn
                text: "New Template"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 36
                
                background: Rectangle {
                    color: createTemplateBtn.pressed ? "#1a8a1a" : (createTemplateBtn.hovered ? "#27ae60" : "#2ecc71")
                    radius: 3
                }
                
                contentItem: Text {
                    text: createTemplateBtn.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: createTemplateDialog.open()
            }

            Button {
                id: deleteTemplateBtn
                text: "Delete Template"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 36
                enabled: templateComboBox.currentText.length > 0
                
                background: Rectangle {
                    color: !deleteTemplateBtn.enabled ? "#555" : 
                           (deleteTemplateBtn.pressed ? "#a93226" : (deleteTemplateBtn.hovered ? "#c0392b" : "#e74c3c"))
                    radius: 3
                }
                
                contentItem: Text {
                    text: deleteTemplateBtn.text
                    color: deleteTemplateBtn.enabled ? "white" : "#888"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: deleteTemplateDialog.open()
            }
        }

        // Template Selection Section
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            color: "#3a3a3a"
            radius: 5

            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                Label {
                    text: "Select Template:"
                    color: "white"
                    font.pixelSize: 14
                    Layout.preferredWidth: 100
                }

                ComboBox {
                    id: templateComboBox
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    
                    model: templateModel ? templateModel.templateNames : []
                    
                    background: Rectangle {
                        color: "#4a4a4a"
                        border.color: templateComboBox.activeFocus ? "#0d7377" : "#666"
                        border.width: 2
                        radius: 3
                    }

                    contentItem: Text {
                        leftPadding: 10
                        text: templateComboBox.displayText
                        font.pixelSize: 14
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                    }

                    popup: Popup {
                        y: templateComboBox.height
                        width: templateComboBox.width
                        implicitHeight: contentItem.implicitHeight > 300 ? 300 : contentItem.implicitHeight
                        padding: 1

                        contentItem: ListView {
                            clip: true
                            implicitHeight: contentHeight
                            model: templateComboBox.popup.visible ? templateComboBox.delegateModel : null
                            currentIndex: templateComboBox.highlightedIndex
                            ScrollIndicator.vertical: ScrollIndicator { }
                        }

                        background: Rectangle {
                            color: "#3a3a3a"
                            border.color: "#666"
                            radius: 2
                        }
                    }

                    delegate: ItemDelegate {
                        width: templateComboBox.width
                        contentItem: Text {
                            text: modelData
                            color: highlighted ? "white" : "#ddd"
                            font.pixelSize: 14
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                        }
                        highlighted: templateComboBox.highlightedIndex === index
                        
                        background: Rectangle {
                            color: highlighted ? "#0d7377" : (hovered ? "#4a4a4a" : "#3a3a3a")
                        }
                    }

                    onCurrentTextChanged: {
                        if (currentText && currentText.length > 0) {
                            templateModel.currentTemplate = currentText
                            loadTemplateItems()
                        }
                    }
                }

                Button {
                    id: refreshBtn
                    text: "↻"
                    font.pixelSize: 18
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    
                    background: Rectangle {
                        color: refreshBtn.pressed ? "#0a5a5d" : (refreshBtn.hovered ? "#0d7377" : "#4a4a4a")
                        radius: 3
                    }
                    
                    contentItem: Text {
                        text: refreshBtn.text
                        color: "white"
                        font.pixelSize: 18
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: loadTemplatesFromDb()
                    
                    ToolTip.visible: hovered
                    ToolTip.text: "Refresh templates"
                }

                Button {
                    id: renameTemplateBtn
                    text: "Rename"
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 40
                    enabled: templateComboBox.currentText.length > 0
                    
                    background: Rectangle {
                        color: !renameTemplateBtn.enabled ? "#555" : 
                               (renameTemplateBtn.pressed ? "#1a6a6d" : (renameTemplateBtn.hovered ? "#0d7377" : "#4a4a4a"))
                        radius: 3
                    }
                    
                    contentItem: Text {
                        text: renameTemplateBtn.text
                        color: renameTemplateBtn.enabled ? "white" : "#888"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        renameTemplateField.text = templateComboBox.currentText
                        renameTemplateDialog.open()
                    }
                }
            }
        }

        // Item management buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            visible: templateComboBox.currentText.length > 0

            Button {
                id: addItemBtn
                text: "+ Add Item"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 32
                
                background: Rectangle {
                    color: addItemBtn.pressed ? "#1a8a1a" : (addItemBtn.hovered ? "#27ae60" : "#2ecc71")
                    radius: 3
                }
                
                contentItem: Text {
                    text: addItemBtn.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    addItemTextField.text = ""
                    addItemCategoryField.text = "0"
                    addItemDialog.open()
                }
            }

            Button {
                id: editItemBtn
                text: "Edit Item"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 32
                enabled: selectedItemId >= 0
                
                background: Rectangle {
                    color: !editItemBtn.enabled ? "#555" : 
                           (editItemBtn.pressed ? "#d68910" : (editItemBtn.hovered ? "#f39c12" : "#f1c40f"))
                    radius: 3
                }
                
                contentItem: Text {
                    text: editItemBtn.text
                    color: editItemBtn.enabled ? "#222" : "#888"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    editItemTextField.text = selectedItemText
                    editItemCategoryField.text = selectedItemCategoryId.toString()
                    editItemDialog.open()
                }
            }

            Button {
                id: deleteItemBtn
                text: "Delete Item"
                Layout.preferredWidth: 100
                Layout.preferredHeight: 32
                enabled: selectedItemId >= 0
                
                background: Rectangle {
                    color: !deleteItemBtn.enabled ? "#555" : 
                           (deleteItemBtn.pressed ? "#a93226" : (deleteItemBtn.hovered ? "#c0392b" : "#e74c3c"))
                    radius: 3
                }
                
                contentItem: Text {
                    text: deleteItemBtn.text
                    color: deleteItemBtn.enabled ? "white" : "#888"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: deleteItemDialog.open()
            }

            Item { Layout.fillWidth: true }

            Label {
                text: selectedItemId >= 0 ? "Selected: ID " + selectedItemId : "Click a row to select"
                color: "#999"
                font.pixelSize: 12
                font.italic: true
            }
        }

        // Template Data Section
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#3a3a3a"
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                // Header row
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    color: "#2a2a2a"
                    radius: 3

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 0

                        Label {
                            text: "ID"
                            color: "white"
                            font.bold: true
                            font.pixelSize: 12
                            Layout.preferredWidth: 50
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: "#555" }

                        Label {
                            text: "Items"
                            color: "white"
                            font.bold: true
                            font.pixelSize: 12
                            Layout.fillWidth: true
                            leftPadding: 10
                        }

                        Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: "#555" }

                        Label {
                            text: "Category"
                            color: "white"
                            font.bold: true
                            font.pixelSize: 12
                            Layout.preferredWidth: 70
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                }

                // ListView for template data
                ListView {
                    id: templateDataListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 1

                    model: ListModel { id: templateDataModel }

                    delegate: Rectangle {
                        width: templateDataListView.width - 12
                        height: 45
                        color: model.id === selectedItemId ? "#0d7377" : (index % 2 === 0 ? "#4a4a4a" : "#3f3f3f")
                        border.color: model.id === selectedItemId ? "#27ae60" : "#555"
                        border.width: model.id === selectedItemId ? 2 : 1
                        radius: 2

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                selectedItemId = model.id
                                selectedItemText = model.items
                                selectedItemCategoryId = model.categoryId
                            }
                            onDoubleClicked: {
                                selectedItemId = model.id
                                selectedItemText = model.items
                                selectedItemCategoryId = model.categoryId
                                editItemTextField.text = selectedItemText
                                editItemCategoryField.text = selectedItemCategoryId.toString()
                                editItemDialog.open()
                            }
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 5
                            spacing: 0

                            Label {
                                text: model.id
                                color: "white"
                                font.pixelSize: 12
                                Layout.preferredWidth: 50
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: "#666" }

                            Label {
                                text: model.items
                                color: "#ddd"
                                font.pixelSize: 12
                                Layout.fillWidth: true
                                leftPadding: 10
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                            }

                            Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: "#666" }

                            Label {
                                text: model.categoryId
                                color: "white"
                                font.pixelSize: 12
                                Layout.preferredWidth: 70
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                        background: Rectangle { color: "#2a2a2a"; implicitWidth: 10 }
                        contentItem: Rectangle { implicitWidth: 8; radius: 4; color: "#666" }
                    }
                }

                // Empty state
                Label {
                    text: templateComboBox.currentText 
                          ? "No items in this template. Click '+ Add Item' to add one." 
                          : "Please select a template from the dropdown above"
                    color: "#999"
                    font.pixelSize: 14
                    font.italic: true
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    visible: templateDataModel.count === 0
                }
            }
        }

        // Status bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
            color: "#2a2a2a"
            radius: 3

            Label {
                id: statusLabel
                anchors.centerIn: parent
                text: "Ready"
                color: "#aaa"
                font.pixelSize: 11
            }
        }
    }

    // ==================== DIALOGS ====================

    // Create Template Dialog
    Dialog {
        id: createTemplateDialog
        title: "Create New Template"
        modal: true
        anchors.centerIn: parent
        width: 400
        standardButtons: Dialog.Ok | Dialog.Cancel

        background: Rectangle { color: "#3a3a3a"; radius: 5 }

        ColumnLayout {
            anchors.fill: parent
            spacing: 15

            Label {
                text: "Enter the name for the new template:"
                color: "white"
                font.pixelSize: 14
            }

            TextField {
                id: newTemplateNameField
                Layout.fillWidth: true
                placeholderText: "Template name (e.g., EPA, SLO)"
                color: "white"
                background: Rectangle { color: "#4a4a4a"; radius: 3 }
            }

            Label {
                text: "Category ID (optional):"
                color: "white"
                font.pixelSize: 14
            }

            TextField {
                id: newTemplateCategoryField
                Layout.fillWidth: true
                text: "0"
                validator: IntValidator { bottom: 0 }
                color: "white"
                background: Rectangle { color: "#4a4a4a"; radius: 3 }
            }
        }

        onAccepted: {
            if (newTemplateNameField.text.trim().length > 0) {
                templateModel.createTemplate(newTemplateNameField.text.trim(), 
                                            parseInt(newTemplateCategoryField.text) || 0)
                newTemplateNameField.text = ""
                newTemplateCategoryField.text = "0"
            }
        }
    }

    // Delete Template Dialog
    Dialog {
        id: deleteTemplateDialog
        title: "Delete Template"
        modal: true
        anchors.centerIn: parent
        width: 400
        standardButtons: Dialog.Yes | Dialog.No

        background: Rectangle { color: "#3a3a3a"; radius: 5 }

        Label {
            text: "Are you sure you want to delete the template '" + templateComboBox.currentText + "'?\n\nThis will permanently delete all items in this template."
            color: "white"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            width: parent.width
        }

        onAccepted: {
            templateModel.deleteTemplate(templateComboBox.currentText)
            selectedItemId = -1
        }
    }

    // Rename Template Dialog
    Dialog {
        id: renameTemplateDialog
        title: "Rename Template"
        modal: true
        anchors.centerIn: parent
        width: 400
        standardButtons: Dialog.Ok | Dialog.Cancel

        background: Rectangle { color: "#3a3a3a"; radius: 5 }

        ColumnLayout {
            anchors.fill: parent
            spacing: 15

            Label {
                text: "Current name: " + templateComboBox.currentText
                color: "#aaa"
                font.pixelSize: 12
            }

            Label {
                text: "Enter new name:"
                color: "white"
                font.pixelSize: 14
            }

            TextField {
                id: renameTemplateField
                Layout.fillWidth: true
                color: "white"
                background: Rectangle { color: "#4a4a4a"; radius: 3 }
            }
        }

        onAccepted: {
            if (renameTemplateField.text.trim().length > 0) {
                templateModel.renameTemplate(templateComboBox.currentText, renameTemplateField.text.trim())
            }
        }
    }

    // Add Item Dialog
    Dialog {
        id: addItemDialog
        title: "Add New Item"
        modal: true
        anchors.centerIn: parent
        width: 450
        standardButtons: Dialog.Ok | Dialog.Cancel

        background: Rectangle { color: "#3a3a3a"; radius: 5 }

        ColumnLayout {
            anchors.fill: parent
            spacing: 15

            Label {
                text: "Adding item to: " + templateComboBox.currentText
                color: "#aaa"
                font.pixelSize: 12
            }

            Label {
                text: "Item text:"
                color: "white"
                font.pixelSize: 14
            }

            TextArea {
                id: addItemTextField
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                placeholderText: "Enter item text..."
                color: "white"
                wrapMode: Text.WordWrap
                background: Rectangle { color: "#4a4a4a"; radius: 3 }
            }

            Label {
                text: "Category ID:"
                color: "white"
                font.pixelSize: 14
            }

            TextField {
                id: addItemCategoryField
                Layout.fillWidth: true
                text: "0"
                validator: IntValidator { bottom: 0 }
                color: "white"
                background: Rectangle { color: "#4a4a4a"; radius: 3 }
            }
        }

        onAccepted: {
            if (addItemTextField.text.trim().length > 0) {
                templateModel.addTemplateItem(templateComboBox.currentText, 
                                             addItemTextField.text.trim(),
                                             parseInt(addItemCategoryField.text) || 0)
            }
        }
    }

    // Edit Item Dialog
    Dialog {
        id: editItemDialog
        title: "Edit Item"
        modal: true
        anchors.centerIn: parent
        width: 450
        standardButtons: Dialog.Ok | Dialog.Cancel

        background: Rectangle { color: "#3a3a3a"; radius: 5 }

        ColumnLayout {
            anchors.fill: parent
            spacing: 15

            Label {
                text: "Editing item ID: " + selectedItemId
                color: "#aaa"
                font.pixelSize: 12
            }

            Label {
                text: "Item text:"
                color: "white"
                font.pixelSize: 14
            }

            TextArea {
                id: editItemTextField
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                color: "white"
                wrapMode: Text.WordWrap
                background: Rectangle { color: "#4a4a4a"; radius: 3 }
            }

            Label {
                text: "Category ID:"
                color: "white"
                font.pixelSize: 14
            }

            TextField {
                id: editItemCategoryField
                Layout.fillWidth: true
                validator: IntValidator { bottom: 0 }
                color: "white"
                background: Rectangle { color: "#4a4a4a"; radius: 3 }
            }
        }

        onAccepted: {
            if (editItemTextField.text.trim().length > 0) {
                templateModel.updateTemplateItem(templateComboBox.currentText,
                                                selectedItemId,
                                                editItemTextField.text.trim(),
                                                parseInt(editItemCategoryField.text) || 0)
            }
        }
    }

    // Delete Item Dialog
    Dialog {
        id: deleteItemDialog
        title: "Delete Item"
        modal: true
        anchors.centerIn: parent
        width: 400
        standardButtons: Dialog.Yes | Dialog.No

        background: Rectangle { color: "#3a3a3a"; radius: 5 }

        Label {
            text: "Are you sure you want to delete item ID " + selectedItemId + "?\n\n\"" + selectedItemText + "\""
            color: "white"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            width: parent.width
        }

        onAccepted: {
            templateModel.deleteTemplateItem(templateComboBox.currentText, selectedItemId)
            selectedItemId = -1
            selectedItemText = ""
            selectedItemCategoryId = 0
        }
    }

    // ==================== FUNCTIONS ====================

    function loadTemplateItems() {
        templateDataModel.clear()
        selectedItemId = -1
        selectedItemText = ""
        selectedItemCategoryId = 0
        statusLabel.text = "Loading..."
        statusLabel.color = "#0d7377"

        if (templateModel && templateComboBox.currentText) {
            var data = templateModel.loadTemplateData(templateComboBox.currentText)
            
            for (var i = 0; i < data.length; i++) {
                templateDataModel.append(data[i])
            }

            statusLabel.text = "Loaded " + data.length + " items from '" + templateComboBox.currentText + "'"
            statusLabel.color = "#27ae60"
        } else {
            statusLabel.text = "Select a template to view items"
            statusLabel.color = "#aaa"
        }
    }

    function loadTemplatesFromDb() {
        if (templateModel) {
            console.log("Loading templates from config database...")
            var success = templateModel.loadTemplates()
            if (success) {
                templatesLoaded = true
                statusLabel.text = "Templates loaded successfully"
                statusLabel.color = "#27ae60"
            } else {
                statusLabel.text = "Failed to load templates from database"
                statusLabel.color = "#e74c3c"
            }
        } else {
            statusLabel.text = "Template model not available"
            statusLabel.color = "#e74c3c"
        }
    }

    Component.onCompleted: {
        loadTemplatesFromDb()
    }
}
