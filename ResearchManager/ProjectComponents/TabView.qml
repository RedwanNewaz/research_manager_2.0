import QtQuick 2.15
import QtQuick.Controls 2.15

Item{
    Connections{
        target: deadline
        function onClicked()
        {
           console.log("deadline tab pressed")
           cdModel.insertDeadlines("")
        }
    }

}
