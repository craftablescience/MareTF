#pragma once

#include <QScrollArea>

/**
 * Fixes the scroll bars not being counted in QScrollArea::sizeHint.
 */
class QMareScrollArea : public QScrollArea {
    Q_OBJECT;

public:
    using QScrollArea::QScrollArea;

    QSize sizeHint() const override;
};
