import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: parent.width
    height: parent.height / 3
    anchors.top: parent.top
    // anchors.fill: parent

    // Calendar state
    property int currentDay: new Date().getDay()
    property int currentDate: new Date().getDate()
    property int currentYear: new Date().getFullYear()
    property int currentMonth: new Date().getMonth() + 1  // 1-12
    property var daysInWeek: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]



    Column {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        spacing: 10

        RowLayout {
            spacing: 10
            width: parent.width

            // Previous button
            Button {
                text: "Prev"
                icon.source: "../images/go_previous.png"
                display: AbstractButton.IconOnly
                onClicked: changeMonth(-1)
            }

            // Month/Year display centered
            Text {
                id: calText
                text: currentMonth + "/" + currentDate + "/" + currentYear + " (" + daysInWeek[currentDay] + ")"
                font.pixelSize: 18
                color: "white"
                Layout.fillWidth: true          // Take remaining horizontal space
                horizontalAlignment: Text.AlignHCenter
            }

            // Next button
            Button {
                text: "Next"
                icon.source: "../images/go_next.png"
                display: AbstractButton.IconOnly
                onClicked: changeMonth(1)
            }

            // Reset button
            Button {
                text: "Reset"
                icon.source: "../images/refresh.png"
                display: AbstractButton.IconOnly
                onClicked: {
                    currentMonth = new Date().getMonth() + 1;
                    currentYear = new Date().getFullYear();

                    calModel.year = currentYear
                    calModel.month = currentMonth
                }
            }
        }

        // Day labels
        Row {
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter
            Repeater {
                model: daysInWeek
                Rectangle {
                    width: 56
                    height: 56
                    color: "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pointSize: 16
                        color: "white"
                        font.bold: true

                    }
                }
            }
        }

        // Calendar grid
        Grid {
            columns: 7
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter

            // Empty cells for days before month starts
            Repeater {
                model: getFirstDayOfMonth()
                Rectangle {
                    width: 56
                    height: 56
                    color: "transparent"
                }
            }

            // Days of the month
            Repeater {
                model: daysInMonth()
                Rectangle {
                    width: 56
                    height: 56
                    radius: 6
                    color: isWeekend(index + 1) ? "#ffcccc" : "#e0e0e0"
                    border.color: hasDeadline(currentYear, currentMonth, index + 1) + 0 * calModel.month ? "#ff0000" : "transparent"
                    border.width: 2

                    property bool today: (index + 1) === currentDate

                    Text {
                        anchors.centerIn: parent
                        text: index + 1
                        font.pointSize: 14
                        color: today ? "#008080" : "black"
                        // font.bold: (deadlines[index + 1] || today) ? true : false
                        font.bold: isSpecialDay(currentYear, currentMonth, index + 1) + 0 * calModel.month
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log("Clicked day event: ", calModel.getEvent(index + 1))
                            deadlineText.text = calModel.getEvent(index + 1)
                        }
                        onEntered: {
                            deadlineRect.visible = true
                            deadlineText.text = calModel.getEvent(index + 1)
                        }

                        // This is called when the mouse leaves the area of 'parentItem'
                        onExited: {
                            deadlineRect.visible = false
                        }
                    }
                }
            }
        }

        Rectangle{
            id: deadlineRect
            width: parent.width
            height: parent.height / 5
            visible: false
            color: "#3c3c3c"
            border.color: "#555555"
            border.width: 1
            radius: 4
            TextInput {
                id: deadlineText
                anchors.fill: parent
                anchors.margins: 10
                font.pixelSize: 14
                color: "white"
                verticalAlignment: Text.AlignVCenter
                selectByMouse: true
                wrapMode: "WordWrap"
            }
        }

    }

    // Helper functions
    function daysInMonth() {
        return new Date(currentYear, currentMonth, 0).getDate()
    }

    function isWeekend(day) {
        var date = new Date(currentYear, currentMonth - 1, day)
        var dayOfWeek = date.getDay()
        return dayOfWeek === 0 || dayOfWeek === 6  // Sunday or Saturday
    }

    function getFirstDayOfMonth() {
        return new Date(currentYear, currentMonth - 1, 1).getDay()
    }

    function changeMonth(delta) {
        currentMonth += delta
        if (currentMonth > 12) {
            currentMonth = 1
            currentYear++
        } else if (currentMonth < 1) {
            currentMonth = 12
            currentYear--
        }
        calModel.year = currentYear
        calModel.month = currentMonth
    }
    function hasDeadline(year, month, day)
    {
        const zeroPad = (num) => String(num).padStart(2, '0');
        let token = year + "-" + zeroPad(month) + "-" + zeroPad(day)
        return calModel.hasDeadline(token)
    }

    function isSpecialDay(year, month, day)
    {
        const date = new Date().getDate();
        if (day === date)
            return true;
        return hasDeadline(year, month, day)
    }

}
