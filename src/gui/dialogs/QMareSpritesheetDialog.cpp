#include "QMareSpritesheetDialog.h"

#include <QVBoxLayout>

QMareSpritesheetDialog::QMareSpritesheetDialog(QWidget* parent) : QDialog{parent} {
    this->setWindowTitle(tr("Edit Spritesheet"));
    this->setModal(true);
    this->setMinimumSize(400, 400);

    auto* layout = new QVBoxLayout{this};

    // TODO
}

void QMareSpritesheetDialog::showSpritesheetEditor(QWidget* parent, QMareTextureWidget* currentTexture) {
    auto* dialog = new QMareSpritesheetDialog{parent};
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->open();
}
