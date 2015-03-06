#ifndef MKVSEGMENTJOINER_H
#define MKVSEGMENTJOINER_H

#include <QtGui/QMainWindow>
#include "ui_MKVSegmentJoiner.h"
#include <QProcess>

class MKVSegmentJoiner : public QMainWindow
{
  Q_OBJECT
  public:
    MKVSegmentJoiner(QWidget *parent = 0);
    ~MKVSegmentJoiner();

  private:
    Ui::MKVSegmentJoinerClass *m_ui;
    bool checkTools();
    QStringList m_segmentIds, m_files;
    QProcess *m_process;
    bool m_running, m_foundId;
    QString m_previousId;
    void grabSegmentIds();
    void grabSegmentId();

    void setSegmentId(bool first = false);
    void setSegmentIds();

  private slots:
    void on_removePushButton_clicked();
    void on_connectPushButton_clicked();
    void handleMKVInfoOutput();
    void mkvinfoFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void mkvpropeditFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // MKVSEGMENTJOINER_H
