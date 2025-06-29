import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id : page
    topPadding: 20

    Component.onCompleted: {

        cppGlue.onProductPurchased.connect(obj.updateOptions)
        obj.updateOptions();
    }

    Component.onDestruction: {

        cppGlue.onProductPurchased.disconnect(obj.updateOptions)
    }

    ColumnLayout {

        QtObject {
            id: obj

            function updateOptions() {

                var monthlyCost = cppGlue.getMonthlySubscriptionCost()
                var lifetimeCost = cppGlue.getLifeTimeSubscriptionCost()

                var monthPurchased = cppGlue.isMonthlySubscriptionPurchased()
                var lifeTimePurchased = cppGlue.isLifeTimeSubscriptionPurchased()

                if(monthPurchased) {
                    monthlyBtn.text = "Subscribed"
                    monthlyBtn.enabled = false

                } else {
                    monthlyBtn.text = monthlyCost + " per month"
                    monthlyBtn.enabled = !monthPurchased && !lifeTimePurchased
                }

//                if(lifeTimePurchased) {

//                    lifeTimeBtn.text = "Purchased"
//                    lifeTimeBtn.enabled = false

//                } else {

//                    lifeTimeBtn.text = lifetimeCost + " for unlimited use"
//                    lifeTimeBtn.enabled = !monthPurchased && !lifeTimePurchased
//                }
            }
        }

        TextArea {
            text: "Subscription Purchase option available:"
            textFormat: Text.AutoText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            readOnly: true
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                id: monthlyBtn
                onClicked: ui.action_Purchase_Monthly_Sub.trigger()
            }

//            Label {
//                text: "<b>OR</b>"
//            }

//            Button {
//                id: lifeTimeBtn
//                onClicked: ui.action_Purchase_Lifetime.trigger()
//            }
        }
    }
}
