/*
 * Application: POI Manager
 * File: undo.cpp
 *
 * Undo operations for move/copy/paste activities of POI managment
 *
 */

/*
 *
 * POI Manager
 * Copyright (C) 2021  "Steve Clarke www.vizier.uk/poi"
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Any modification to the code must be contributed back to the community.
 *
 * Redistribution and use in binary or source form with or without modification
 * is permitted provided that the following conditions are met:
 *
 * Clause 7b - attribution for the original author shall be provided with the
 * inclusion of the above Copyright statement in its entirity, which must be
 * clearly visible in each application source file, in any documentation and also
 * in a pop-up window in the application itself. It is requested that the charity
 * donation link to Guide Dogs for the Blind remain within the program, and any
 * derivitive thereof.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
