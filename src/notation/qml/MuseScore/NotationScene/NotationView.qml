/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

import "internal"

FocusScope {
    id: root

    property alias name: notationView.objectName
    property alias publishMode: notationView.publishMode

    property alias paintView: notationView

    property alias isNavigatorVisible: notationNavigator.visible
    property alias isMainView: notationView.isMainView

    property alias defaultNavigationControl: fakeNavCtrl

    NavigationSection {
        id: navSec
        name: "NotationView"
        order: 5
        enabled: root.visible
    }

    QtObject {
        id: prv
        readonly property int scrollbarMargin: 4
    }

    NotationContextMenuModel {
        id: contextMenuModel
    }

    Component.onCompleted: {
        notationView.load()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        NotationSwitchPanel {
            id: tabPanel
            Layout.fillWidth: true

            navigationSection: navSec
        }

        SeparatorLine { visible: tabPanel.visible }

        SplitView {
            id: splitView

            Layout.fillWidth: true
            Layout.fillHeight: true

            orientation: notationNavigator.orientation === Qt.Horizontal ? Qt.Vertical : Qt.Horizontal

            NotationScrollAndZoomArea {
                SplitView.fillWidth: true
                SplitView.fillHeight: true

                NotationPaintView {
                    id: notationView
                    anchors.fill: parent

                    NavigationPanel {
                        id: navPanel
                        name: "ScoreView"
                        section: navSec
                        enabled: notationView.enabled && notationView.visible
                        direction: NavigationPanel.Both
                        order: 2
                    }

                    NavigationControl {
                        id: fakeNavCtrl
                        name: "Score"
                        enabled: notationView.enabled && notationView.visible

                        panel: navPanel
                        order: 1

                        onActiveChanged: {
                            if (fakeNavCtrl.active) {
                                notationView.forceFocusIn()

                                if (navPanel.highlight) {
                                    notationView.selectOnNavigationActive()
                                }
                            } else {
                                notationView.focus = false
                            }
                        }
                    }

                    NavigationFocusBorder {
                        navigationCtrl: fakeNavCtrl
                        drawOutsideParent: false
                    }

                    onActiveFocusRequested: {
                        fakeNavCtrl.requestActive()
                    }

                    onShowContextMenuRequested: function (elementType, viewPos) {
                        contextMenuModel.loadItems(elementType)
                        contextMenuLoader.show(viewPos, contextMenuModel.items)
                    }

                    onHideContextMenuRequested: {
                        contextMenuLoader.close()
                    }

                    onViewportChanged: {
                        notationNavigator.setCursorRect(viewport)
                    }

                    ContextMenuLoader {
                        id: contextMenuLoader

                        onHandleMenuItem: function(itemId) {
                            contextMenuModel.handleMenuItem(itemId)
                        }
                    }
                }
            }

            Loader {
                id: notationNavigator

                property var orientation: notationNavigator.item ? notationNavigator.item.orientation : Qt.Horizontal

                visible: false

                SplitView.preferredHeight: 100
                SplitView.preferredWidth: 100
                SplitView.minimumWidth: 30
                SplitView.minimumHeight: 30

                sourceComponent: notationNavigator.visible ? navigatorComp : null

                function setCursorRect(viewport) {
                    if (notationNavigator.item) {
                        notationNavigator.item.setCursorRect(viewport)
                    }
                }
            }

            LiveBrailleModel {
                id: lbmodel
                property int keys_pressed: 0
                property string keys_buffer: ""

                onCurrentItemChanged: {                    
                    if(lbmodel.currentItemPositionStart.valueOf() != -1 &&
                            lbmodel.currentItemPositionEnd.valueOf() != -1) {
                            //livebrailleinfo.select(lbmodel.currentItemPositionStart.valueOf(), lbmodel.currentItemPositionEnd.valueOf());
                        if(livebrailleinfo.focus) {
                            livebrailleinfo.cursorPosition = lbmodel.currentItemPositionEnd.valueOf();
                        }
                    }                    
                }                

                onLiveBrailleStatusChanged: {
                    livebrailleview.visible = lbmodel.enabled
                }

                onLiveBrailleModeChanged: {
                    switch(lbmodel.mode) {
                        case 1: { // navigation mode
                            fakeNavCtrl2.accessible.setName("Live Braille - Navigation mode");
                            fakeNavCtrl2.accessible.nameChanged("Live Braille - Navigation mode");
                            break;
                        }
                        case 2: { // Input mode
                            fakeNavCtrl2.accessible.setName("Live Braille - Input mode");
                            fakeNavCtrl2.accessible.nameChanged("Live Braille - Input mode");
                            break;
                        }
                    }
                }
                onLiveBrailleInfoChanged: {
                    fakeNavCtrl2.accessible.textChanged(livebrailleinfo.text);
                }

                Component.onCompleted: {
                    livebrailleview.visible = lbmodel.enabled
                }                                
            }

            StyledFlickable {
                id: livebrailleview

                SplitView.fillWidth: true
                SplitView.preferredHeight: 50
                SplitView.minimumHeight: 30

                TextArea.flickable: TextArea {
                    id: livebrailleinfo
                    text: lbmodel.liveBrailleInfo
                    wrapMode: Text.AlignLeft

                    property var keyMap: (new Map([
                      [Qt.Key_0, "0"], [Qt.Key_1, "1"], [Qt.Key_2, "2"], [Qt.Key_3, "3"],
                      [Qt.Key_4, "4"], [Qt.Key_5, "5"], [Qt.Key_6, "6"], [Qt.Key_7, "7"],
                      [Qt.Key_8, "8"], [Qt.Key_9, "9"],
                      [Qt.Key_A, "A"], [Qt.Key_B, "B"], [Qt.Key_C, "C"], [Qt.Key_D, "D"],
                      [Qt.Key_E, "E"], [Qt.Key_F, "F"], [Qt.Key_G, "G"], [Qt.Key_H, "H"],
                      [Qt.Key_I, "I"], [Qt.Key_J, "J"], [Qt.Key_K, "K"], [Qt.Key_L, "L"],
                      [Qt.Key_M, "M"], [Qt.Key_N, "N"], [Qt.Key_O, "O"], [Qt.Key_P, "P"],
                      [Qt.Key_Q, "Q"], [Qt.Key_R, "R"], [Qt.Key_S, "S"], [Qt.Key_T, "T"],
                      [Qt.Key_U, "U"], [Qt.Key_V, "V"], [Qt.Key_W, "W"],
                      [Qt.Key_X, "X"], [Qt.Key_Y, "Y"], [Qt.Key_Z, "Z"],
                      [Qt.Key_Space, "Space"],
                      [Qt.Key_Right, "Right"], [Qt.Key_Left, "Left"],
                      [Qt.Key_Up, "Up"],[Qt.Key_Down, "Down"],
                      [Qt.Key_PageUp, "PageUp"],[Qt.Key_PageDown, "PageDown"],
                      [Qt.Key_Home, "Home"],[Qt.Key_End, "End"],
                      [Qt.Key_Delete, "Delete"], [Qt.Key_Escape, "Escape"],
                      [Qt.Key_Minus, "Minus"], [Qt.Key_Plus, "Plus"],
                     ]));

                    cursorDelegate: Rectangle {
                        id: livebraillecursor
                        visible: livebrailleinfo.cursorVisible
                        color: lbmodel.cursorColor
                        width: livebrailleinfo.cursorRectangle.width

                        SequentialAnimation {
                            loops: Animation.Infinite
                            running: livebrailleinfo.cursorVisible

                            PropertyAction {
                                target: livebraillecursor
                                property: 'visible'
                                value: true
                            }

                            PauseAnimation {
                                duration: 600
                            }

                            PropertyAction {
                                target: livebraillecursor
                                property: 'visible'
                                value: false
                            }

                            PauseAnimation {
                                duration: 600
                            }

                            onStopped: {
                                livebraillecursor.visible = false
                            }
                        }
                    }

                    NavigationPanel {
                        id: navPanel2
                        name: "LiveBrailleView"
                        section: navSec
                        enabled: livebrailleinfo.enabled && livebrailleinfo.visible
                        direction: NavigationPanel.Both
                        order: 3
                    }

                    NavigationControl {
                        id: fakeNavCtrl2
                        name: "LiveBraille"
                        enabled: livebrailleinfo.enabled && livebrailleinfo.visible
                        panel: navPanel2
                        order: 1

                        accessible.role: MUAccessible.EditableText
                        accessible.name: "Live Braille"
                        accessible.visualItem: livebrailleinfo
                        accessible.text: livebrailleinfo.text
                        accessible.selectedText: livebrailleinfo.selectedText
                        accessible.selectionStart: livebrailleinfo.selectionStart
                        accessible.selectionEnd: livebrailleinfo.selectionEnd
                        accessible.cursorPosition: livebrailleinfo.cursorPosition

                        onActiveChanged: {
                            if (fakeNavCtrl2.active) {
                                livebrailleinfo.forceActiveFocus();
                                if(lbmodel.currentItemPositionStart.valueOf() != -1 &&
                                    lbmodel.currentItemPositionEnd.valueOf() != -1) {
                                        livebrailleinfo.cursorPosition = lbmodel.currentItemPositionEnd.valueOf();
                                }
                            } else {
                                livebrailleinfo.focus = false
                            }
                        }
                    }

                    NavigationFocusBorder {
                        navigationCtrl: fakeNavCtrl2
                        drawOutsideParent: false
                    }

                    onCursorPositionChanged: {
                        //console.log("pos: ", livebrailleinfo.cursorPosition);
                        lbmodel.cursorPosition = livebrailleinfo.cursorPosition;                        
                    }

                    Keys.onPressed : {
                        if(event.key !== Qt.Key_Shift && event.key !== Qt.Key_Alt &&
                                event.key !== Qt.Key_Control) {

                            lbmodel.keys_pressed++;

                            var keys = "";

                            if(event.modifiers === Qt.ShiftModifier) {
                                keys = keys === "" ? "Shift" : keys += "+Shift";
                            }

                            if(event.modifiers === Qt.AltModifier) {
                                keys = keys === "" ? "Alt" : keys += "+Alt";
                            }
                            if(event.modifiers === Qt.ControlModifier) {
                                keys = keys === "" ? "Ctrl" : keys += "+Ctrl";
                            }

                            if(keys !== "") {
                                keys += "+";
                            }

                            if(keyMap.get(event.key) !== "") {
                                keys += keyMap.get(event.key);
                            }

                            if(keys !== "Left" && keys !== "Right"
                                    && keys !== "Up" && keys !== "Down") {
                                if(lbmodel.keys_buffer !== "") lbmodel.keys_buffer += "+";
                                lbmodel.keys_buffer += keys;
                                event.accepted = true;
                            }
                        }
                    }
                    Keys.onReleased: {                        
                        if(event.key !== Qt.Key_Shift && event.key !== Qt.Key_Alt &&
                                event.key !== Qt.Key_Control) {
                            lbmodel.keys_pressed--;
                            //console.log("Keys.onReleased :", lbmodel.keys_pressed);
                            if(lbmodel.keys_pressed <= 0) {                                
                                lbmodel.keys = lbmodel.keys_buffer;
                                lbmodel.keys_buffer = "";
                                lbmodel.keys_pressed == 0;
                                // set the focus back to live braille
                                navPanel.setActive(false)
                                fakeNavCtrl.setActive(false);
                                navPanel2.setActive(true)
                                fakeNavCtrl2.setActive(true);                                
                            }
                        }
                    }                    
                }

                ScrollBar.vertical: ScrollBar {}
            }

            Component {
                id: navigatorComp

                NotationNavigator {

                    property bool isVertical: orientation === Qt.Vertical

                    objectName: root.name + "Navigator"

                    Component.onCompleted: {
                        load()
                    }

                    onMoveNotationRequested: function(dx, dy) {
                        notationView.moveCanvas(dx, dy)
                    }
                }
            }

            handle: Rectangle {
                id: resizingHandle

                implicitWidth: 4
                implicitHeight: 4

                color: ui.theme.strokeColor

                states: [
                    State {
                        name: "PRESSED"
                        when: resizingHandle.SplitHandle.pressed
                        PropertyChanges {
                            target: resizingHandle
                            opacity: ui.theme.accentOpacityHit
                        }
                    },
                    State {
                        name: "HOVERED"
                        when: resizingHandle.SplitHandle.hovered
                        PropertyChanges {
                            target: resizingHandle
                            opacity: ui.theme.accentOpacityHover
                        }
                    }
                ]
            }
        }

        SearchPopup {
            id: searchPopup

            Layout.fillWidth: true

            navigationPanel.section: navSec
            navigationPanel.order: 3

            onClosed: {
                fakeNavCtrl.requestActive()
            }
        }
    }
}
