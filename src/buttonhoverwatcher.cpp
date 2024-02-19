#include "buttonhoverwatcher.h"
#include <QCheckBox>
#include <QPushButton>
#include <QEvent>

ButtonHoverWatcher::ButtonHoverWatcher(QObject * parent) : QObject(parent)
{}

bool ButtonHoverWatcher::eventFilter(QObject * watched, QEvent * event){
    QPushButton * button = qobject_cast<QPushButton*>(watched);

    if (button) {
        if (event->type() == QEvent::Enter) {
            button->setIcon(QIcon(R"(:/image/icons/)" + button->objectName() + "_hover.png"));
            return true;
        }

        if (event->type() == QEvent::Leave){
            button->setIcon(QIcon(R"(:/image/icons/)" + button->objectName() + ".png"));
            return true;
        }
    }
    else {
        QCheckBox * checkBox = qobject_cast<QCheckBox*>(watched);
        if (event->type() == QEvent::Enter && !checkBox->isChecked()) {
            checkBox->setIcon(QIcon(R"(:/image/icons/)" + checkBox->objectName() + "_hover.png"));
            return true;
        }

        if (event->type() == QEvent::Leave && !checkBox->isChecked()){
            checkBox->setIcon(QIcon(R"(:/image/icons/)" + checkBox->objectName() + ".png"));
            return true;
        }
    }
    return false;
}
