import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import MPV

Window {
    width: 720
    height: 480
    visible: true
    title: qsTr("MPV Video Player")

    MpvObject {
        id: _mpvObject
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: _controlPanelRectangle.top
    }

    Rectangle {
        id: _controlPanelRectangle
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 100
        color: "gray"
        Row {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            spacing: 10

            Button {
                width: 50
                height: 50
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    _fileDialog.open()
                }

                FileDialog {
                    id: _fileDialog
                    onAccepted: {
                        _mpvObject.open(selectedFile)
                    }
                }


                Text {
                    anchors.centerIn: parent
                    text: "open"
                }
            }

            Button {
                width: 50
                height: 50
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    _mpvObject.setProperty("pause", false)
                }

                Text {
                    anchors.centerIn: parent
                    text: "play"
                }
            }

            Button {
                width: 50
                height: 50
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    _mpvObject.setProperty("pause", true)
                }

                Text {
                    anchors.centerIn: parent
                    text: "pause"
                }
            }

            Button {
                width: 50
                height: 50
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    _mpvObject.command(["stop"])
                }

                Text {
                    anchors.centerIn: parent
                    text: "stop"
                }
            }
        }
    }


}
