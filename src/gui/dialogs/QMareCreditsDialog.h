#pragma once

#include <QDialog>
#include <QProxyStyle>
#include <QStyleOption>

class QMareLicenseTabStyle : public QProxyStyle {
public:
	[[nodiscard]] QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const override;

	void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const override;
};

class QMareCreditsDialog : public QDialog {
	Q_OBJECT;

public:
	explicit QMareCreditsDialog(QWidget* parent = nullptr);

	static void showCredits(QWidget* parent = nullptr);
};
