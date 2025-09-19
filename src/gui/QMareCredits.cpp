#include "QMareCredits.h"

#include <QMessageBox>

#include "../common/Config.h"

void QMareCredits::show(QWidget* parent) {
	auto* box = new QMessageBox{parent};
	box->setWindowTitle(QObject::tr("About"));
	box->setIconPixmap(QPixmap{":/logo.png"}.scaledToWidth(64));
	box->setTextFormat(Qt::MarkdownText);
	box->setText(
		QObject::tr(
			"## %1\n"
			"Created by %2. This project lives [on GitHub here](%3).\n\n"
			"The kirin in the program logo (Olive Shade) was created with %4's character creator.\n\n"
			"The lovely splash screen art is by %5."
		).arg(
			PROJECT_TITLE,
			PROJECT_ORGANIZATION_NAME,
			PROJECT_HOMEPAGE_URL,
			"[pony.town](https://pony.town)",
			"[@pastacrylic](https://linktr.ee/pastacrylic)"
		)
	);
	box->setStandardButtons(QMessageBox::Ok);
	box->exec();
}
