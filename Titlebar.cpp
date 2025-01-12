#include "Titlebar.hpp"
#include <QDebug>
TitleBar::TitleBar(QWidget *parent) : QFrame(parent), onBtn{true, true, true}
{
    this->parent_widget = parent;
    this->band_widget = nullptr;
    QHBoxLayout *layout = new QHBoxLayout(this);

    QPushButton *minBtn = new QPushButton();
    minBtn->setObjectName("min");
    QPushButton *maxBtn = new QPushButton();
    maxBtn->setObjectName("max");
    QPushButton *closeBtn = new QPushButton();
    closeBtn->setObjectName("close");

    layout->addWidget(minBtn);
    layout->addWidget(maxBtn);
    layout->addWidget(closeBtn);
    layout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    this->setLayout(layout);
    this->setObjectName("titleBar");
    this->setFixedHeight(50);
    QMetaObject::connectSlotsByName(this);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        tmp_pos = event->globalPos() - parent_widget->frameGeometry().topLeft();
        event->accept();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        parent_widget->move(event->globalPos() - tmp_pos);
        // 确保窗口与鼠标的相对位置不变，即固定将窗口移动到鼠标位置的左上大小为tmp_pos的位置
        event->accept();
    }
}

void TitleBar::setBtnStatus(ButtonStatus btn, bool on)
{
    this->onBtn[int(btn)] = on;
}

void TitleBar::on_close_clicked()
{
    if (!onBtn[int(ButtonStatus::closeBtn)])
        return;
    auto ans = QMessageBox::question(this, "提示", "确定关闭？", QMessageBox::Yes | QMessageBox::No);
    switch (ans)
    {
    case QMessageBox::Yes:
        parent_widget->close();
        break;
    case QMessageBox::Cancel:
        break;
    default:
        break;
    }
}

void TitleBar::on_min_clicked()
{
    if (!onBtn[int(ButtonStatus::minBtn)])
        return;
    parent_widget->showMinimized();
}

void TitleBar::on_max_clicked()
{
    if (!onBtn[int(ButtonStatus::maxBtn)])
        return;
    if (parent_widget->isMaximized())
    {
        parent_widget->showNormal();
    }
    else
    {
        parent_widget->showMaximized();
    }
}

void TitleBar::updateBandWidget(QWidget *widget)
{
    /*使用事件过滤器监听其他对象的事件，从而绑定窗口与标题栏*/
    if (this->band_widget != nullptr)
    {
        this->band_widget->removeEventFilter(this);
    }
    this->band_widget = widget;
    if (this->band_widget != nullptr)
    {
        this->adjustBandWidget();
        this->band_widget->installEventFilter(this);
    }
}

void TitleBar::adjustBandWidget()
{
    auto obj_pos = this->band_widget->geometry();
    auto self_pos = this->geometry();

    this->setGeometry(obj_pos.x(), obj_pos.y() - self_pos.height(), obj_pos.width(), obj_pos.height());
}

bool TitleBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this->band_widget)
    {
        if (event->type() == QEvent::Resize)
        {
            this->adjustBandWidget();
        }
    }
    return QWidget::eventFilter(obj, event);
}