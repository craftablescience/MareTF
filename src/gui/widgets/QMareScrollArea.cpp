#include "QMareScrollArea.h"

#include <QScrollBar>

QSize QMareScrollArea::sizeHint() const {
    return QScrollArea::sizeHint().grownBy({
        0,
        0,
        this->verticalScrollBar() ? this->verticalScrollBar()->sizeHint().width() : 0,
        this->horizontalScrollBar() ? this->horizontalScrollBar()->sizeHint().height() : 0,
    });
}
