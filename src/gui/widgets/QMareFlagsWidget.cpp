#include "QMareFlagsWidget.h"

#include "Common.h"

QMareFlagsWidget::QMareFlagsWidget(QWidget* parent) : QListWidget{parent} {}

void QMareFlagsWidget::repopulateFlagList(uint32_t flags, vtfpp::VTF::Platform platform, uint32_t version) {
	this->clear();
	const auto prettyFlagNames = ::getPrettyFlagNamesFor(version, platform);
	for (int i = 0; i < 32; i++) {
		auto* flagItem = new QListWidgetItem{prettyFlagNames[i].data(), this};
		flagItem->setCheckState(flags & 1 << i ? Qt::Checked : Qt::Unchecked);
		// populate this differently per VTF version/platform
		if ((1 << i & vtfpp::VTF::FLAG_MASK_INTERNAL) || flagItem->text().contains("Internal")) {
			flagItem->setFlags(flagItem->flags() & ~Qt::ItemIsEnabled);
		}
		if (flagItem->text().contains("Unused") || flagItem->text().contains("(VTEX)")) {
			flagItem->setForeground(Qt::gray);
		}
		this->addItem(flagItem);
	}
}

uint32_t QMareFlagsWidget::getFlags() const {
	uint32_t flags = 0;
	for (int i = 0; i < this->count(); i++) {
		if (this->item(i)->checkState() == Qt::Checked) {
			flags |= 1 << i;
		}
	}
	return flags;
}
