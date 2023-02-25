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
                        lbmodel.cursorPosition = livebrailleinfo.cursorPosition;
                    }

                    Keys.onPressed: {
                        if(event.key !== Qt.Key_Shift && event.key !== Qt.Key_Alt &&
                                event.key !== Qt.Key_Control) {

                            var shortcut = "";

                            if(event.modifiers === Qt.ShiftModifier) {
                                shortcut = shortcut === "" ? "Shift" : shortcut += "+Shift";
                            }

                            if(event.modifiers === Qt.AltModifier) {
                                shortcut = shortcut === "" ? "Alt" : shortcut += "+Alt";
                            }
                            if(event.modifiers === Qt.ControlModifier) {
                                shortcut = shortcut === "" ? "Ctrl" : shortcut += "+Ctrl";
                            }

                            if(shortcut !== "") shortcut += "+";

                            if(event.key === Qt.Key_Right) {
                                shortcut += "Right"
                            } else if(event.key === Qt.Key_Left) {
                                shortcut += "Left"
                            } else if(event.key === Qt.Key_Up) {
                                shortcut += "Up"
                            } else if(event.key === Qt.Key_Down) {
                                shortcut += "Down"
                            } else if(event.key === Qt.Key_PageUp) {
                                shortcut += "PgUp"
                            } else if(event.key === Qt.Key_PageDown) {
                                shortcut += "PgDown"
                            } else if(event.key === Qt.Key_Home) {
                                shortcut += "Home"
                            } else if(event.key === Qt.Key_End) {
                                shortcut += "End"
                            }
                            if(shortcut !== "Left" && shortcut !== "Right" &&
                                    shortcut !== "Up" && shortcut !== "Down") {
                                lbmodel.shorcut = shortcut;
                                event.accepted = true;
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
