#include "undo.h"

Undo::Undo()
{
    clear() ;
}

void Undo::clear()
{
    undoList.clear() ;
    popType = Undo::ListEmpty ;
}

Undo::Type Undo::nextType()
{
    if (undoList.length()<=0) {
        return Undo::ListEmpty ;
    } else {
        return undoList.at(undoList.length()-1).type ;
    }
}

PoiEntry& Undo::popPoiEntry()
{
    if (undoList.length()>0 && undoList.at(undoList.length()-1).type == Undo::PoiRecord) {
        popPoi = undoList.at(undoList.length()-1).poi ;
        popType = Undo::PoiRecord ;
        undoList.removeAt(undoList.length()-1) ;
    } else {
        // Error Case (should never happen)
        PoiEntry pop ;
        popPoi = pop ;
        popType = Undo::ListEmpty ;
    }
    return popPoi ;
}

TrackEntry& Undo::popTrackEntry()
{
    if (undoList.length()>0 && undoList.at(undoList.length()-1).type == Undo::TrackRecord) {
        popTrack = undoList.at(undoList.length()-1).track ;
        popType = Undo::TrackRecord ;
        undoList.removeAt(undoList.length()-1) ;
    } else {
        // Error Case (should never happen)
        TrackEntry pop ;
        popTrack = pop ;
        popType = Undo::ListEmpty ;
    }
    return popTrack ;
}

void Undo::pushPoiEntry(PoiEntry& entry)
{
    UndoEntry undoentry ;
    undoentry.type = Undo::PoiRecord ;
    undoentry.poi = entry ;
    undoList.append(undoentry) ;
}

void Undo::pushTrackEntry(TrackEntry& entry)
{
    UndoEntry undoentry ;
    undoentry.type = Undo::TrackRecord ;
    undoentry.track = entry ;
    undoList.append(undoentry) ;
}

bool Undo::redo()
{
    bool success = false ;
    switch (popType) {
    case Undo::PoiRecord:
        pushPoiEntry(popPoi) ;
        success=true ;
        break ;
    case Undo::TrackRecord:
        pushTrackEntry(popTrack) ;
        success=true ;
        break ;
    }
    popType=Undo::ListEmpty ;
    return success ;
}

bool Undo::undoable()
{
    return (undoList.length()>0) ;
}

bool Undo::redoable()
{
    return (popType!=Undo::ListEmpty) ;
}
