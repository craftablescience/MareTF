#pragma once

#include <QListWidget>
#include <vtfpp/VTF.h>

class QMareFlagsWidget : public QListWidget {
	Q_OBJECT;

public:
	explicit QMareFlagsWidget(QWidget* parent = nullptr);

	void repopulateFlagList(uint32_t flags, vtfpp::VTF::Platform platform, uint32_t version);

	[[nodiscard]] uint32_t getFlags() const;
};

class QMareFlagsExtraWidget : public QListWidget {
	Q_OBJECT;

public:
	explicit QMareFlagsExtraWidget(QWidget* parent = nullptr);

	void repopulateFlagList(uint32_t flags);

	[[nodiscard]] uint32_t getFlags() const;
};
