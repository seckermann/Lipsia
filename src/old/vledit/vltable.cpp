#include "vltable.h"

vlTable::vlTable(QWidget* parent, const char* name)
    : QTable(parent, name)
{
}

void vlTable::mouseReleaseEvent(QMouseEvent* e) {

    // ermittle die Zeile, die geklickt wurde
    int row = rowAt(e->y());

    qDebug("Zeile: %i\n", row);

}

