#pragma once

#include <format>

#include <QPushButton>

#include "Config.h"
#include "EnumMappings.h"

#include "widgets/QMareComboBox.h"
#include "widgets/QMareSpinBox.h"

class QMareCLIWrapper : public QObject {
	Q_OBJECT;

public:
	explicit QMareCLIWrapper(const QString& mode, QObject* parent = nullptr, const QString& program = PROJECT_NAME);

	void addArg(auto&& arg) {
		this->arguments.emplace_back(std::forward<decltype(arg)>(arg));
	}

	void addArgPair(auto&& arg1, auto&& arg2) {
		this->arguments.emplace_back(std::forward<decltype(arg1)>(arg1));
		this->arguments.emplace_back(std::forward<decltype(arg2)>(arg2));
	}

	void addFlagPredicate(bool p, auto&& arg) {
		if (p) {
			this->arguments.emplace_back(std::forward<decltype(arg)>(arg));
		}
	}

	void addFlag(const QAbstractButton* checkBox, auto&& arg, bool negated = false) {
		this->addFlagPredicate((!negated && checkBox->isChecked()) || (negated && !checkBox->isChecked()), std::forward<decltype(arg)>(arg));
	}

	 void addInt(const QMareSpinBox* spinBox, auto&& arg) {
		this->addArgPair(std::forward<decltype(arg)>(arg), std::format("{}", spinBox->value()).data());
	}

	void addFloat(const QMareDoubleSpinBox* spinBox, auto&& arg) {
		this->addArgPair(std::forward<decltype(arg)>(arg), std::format("{}", spinBox->value()).data());
	}

	template<typename E>
	void addEnum(const QMareComboBox* combo, auto&& name) {
		const auto e = static_cast<E>(combo->currentData().toInt());
		this->addArgPair(std::forward<decltype(name)>(name), not_magic_enum::enum_name(e).data());
	}

	[[nodiscard]] QString getCommand() const;

	int exec() const; // NOLINT(*-use-nodiscard)

protected:
	QStringList arguments;
};
