import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    width: parent.width
    clip: true

    // Calendar state
    property int currentDay: new Date().getDay()
    property int currentDate: new Date().getDate()
    property int currentYear: new Date().getFullYear()
    property int currentMonth: new Date().getMonth() + 1  // 1-12
    property var daysInWeek: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
    
    // Expose signal for deadline event display
    signal showDeadline(string text)
    signal hideDeadline()



    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0

        // spacing: 10

        RowLayout {
            Layout.fillWidth: true
            // spacing: 10

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
            Layout.alignment: Qt.AlignHCenter
            spacing: 4
            Repeater {
                model: daysInWeek
                Rectangle {
                    width: 50
                    height: 40
                    color: "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pointSize: 12
                        color: "white"
                        font.bold: true

                    }
                }
            }
        }

        // Calendar grid
        Grid {
            Layout.alignment: Qt.AlignHCenter
            columns: 7
            spacing: 4

            // Empty cells for days before month starts
            Repeater {
                model: getFirstDayOfMonth()
                Rectangle {
                    width: 50
                    height: 50
                    color: "transparent"
                }
            }

            // Days of the month
            Repeater {
                model: daysInMonth()
                Rectangle {
                    width: 50
                    height: 50
                    radius: 5
                    color: isWeekend(index + 1) ? "#ffcccc" : "#e0e0e0"
                    border.color: hasDeadline(currentYear, currentMonth, index + 1) + 0 * calModel.month ? "#ff0000" : "transparent"
                    border.width: 2

                    property bool today: (index + 1) === currentDate

                    Text {
                        anchors.centerIn: parent
                        text: index + 1
                        font.pointSize: 12
                        color: today ? "#008080" : "black"
                        // font.bold: (deadlines[index + 1] || today) ? true : false
                        font.bold: isSpecialDay(currentYear, currentMonth, index + 1) + 0 * calModel.month
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log("Clicked day event: ", calModel.getEvent(index + 1))
                            showDeadline(calModel.getEvent(index + 1))
                        }
                        onEntered: {
                            showDeadline(calModel.getEvent(index + 1))
                        }

                        // This is called when the mouse leaves the area of 'parentItem'
                        onExited: {
                            hideDeadline()
                        }
                    }
                }
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
