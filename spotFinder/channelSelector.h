#ifndef CHANNELSELECTOR_H
#define CHANNELSELECTOR_H

#include <vector>
#include <QWidget>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QString>

class ChannelSelector : public QWidget
{
  Q_OBJECT

    public:
  ChannelSelector(QWidget* parent=0);
  void setChannels(std::vector<QString> channels);
  int selectedId();

 private:
  QButtonGroup* channelGroup;
  QHBoxLayout* hbox;
};

#endif
