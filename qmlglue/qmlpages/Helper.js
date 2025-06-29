.pragma library

function indexOfCheckedAction(actions) {
    for( var i in actions) {
        if( actions[i].isChecked)
            return i;
    }

    return -1;
}

function findActionByName(actionGroup, name) {

    var actions = actionGroup.actions
    for( var a in actions) {
        if(actions[a].objectName === name)
            return actions[a]
    }

    return null
}
