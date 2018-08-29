#ifndef UNDO_H
#define UNDO_H

#include <QList>
#include "poicollection.h"

//
// Undo List
//

class Undo
{
public:
    enum Type {
        PoiRecord,
        TrackRecord,
        ListEmpty
    } ;

private:
    typedef struct {
        Undo::Type type ;
        PoiEntry poi ;
        TrackEntry track ;
    } UndoEntry ;

private:
    QList<UndoEntry> undoList ;
    PoiEntry popPoi ;
    TrackEntry popTrack ;
    Undo::Type popType ;
    int undoIndex ;

public:
    Undo();
    void clear() ;
    Undo::Type nextType() ;
    PoiEntry& popPoiEntry() ;
    TrackEntry& popTrackEntry() ;
    void pushPoiEntry(PoiEntry& entry) ;
    void pushTrackEntry(TrackEntry& entry) ;
    bool undoable() ;
    bool redoable() ;
    bool redo() ;

};

#endif // UNDO_H
