import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Rectangle {
    x: 0
    y: 0
    width: parent.width //480
    height: 300 //123
    opacity: 0.75
    radius: 20
    color: "#ffffff"
    Label{
        text: "Get Reference in Bibtex"
        font.bold: true 
        font.pixelSize: 12
        x: parent.width / 2 - 50

    }
    Rectangle{
        // anchors.fill: parent
        id: citationRectangle
        height: parent.height - 100 
        width: parent.width - 100
        border.color: "black"
        x: 50
        y: 20
        ScrollView {
            // Layout.fillWidth: true
            // Layout.fillHeight: true
            width: parent.width
            height: parent.height
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            TextArea {
                id: citationText
                font.styleName: "Regular"
                font.pointSize: 10
                anchors.fill: parent.parent
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
        }
    }
    Row{
        anchors.top: citationRectangle.bottom
        x: parent.width / 2 - 150
        
        spacing: 20
        RoundButton{
            text: "Search"
            radius: 20
            width: 100
            height: 50
            y: 10
            onClicked: cmModel.searchBibtex(citationText.text)
        }
        RoundButton{
            text: "Update"
            radius: 20
            width: 100
            height: 50
            y: 10
            onClicked: cmModel.updateBibtex(citationText.text)
        }
        RoundButton{
            text: "Copy"
            radius: 20
            width: 100
            height: 50
            y: 10
            onClicked: cmModel.copyBibtex(citationText.text)
        }
    }

    Connections
    {
        target: cmModel
         function onPdfTitleChanged(title) {
            citationText.text = title
        }
    }
    
 
}