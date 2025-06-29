.pragma library

function findIndex( model, data) {
    for( var i=0 ; i < model.count; ++i)
        if( model.get(i).data === data)
            return i;

    return -1;
}
