#include "MKVSegmentJoiner.h"
#include <QListWidgetItem>

#include <QFile>
#include <QMessageBox>

MKVSegmentJoiner::MKVSegmentJoiner(QWidget *parent) :
    QMainWindow(parent), m_ui(0), m_segmentIds(), m_process(0), m_files(), m_running(false),
        m_foundId(false)
{
  m_ui = new Ui::MKVSegmentJoinerClass();
  m_ui->setupUi(this);
  this->setWindowTitle("MKVSegmentJoiner - 2013.02.08.1");
}

MKVSegmentJoiner::~MKVSegmentJoiner()
{

}

void MKVSegmentJoiner::on_removePushButton_clicked()
{
  if (m_running) {
    return;
  }
  QList<QListWidgetItem *> items = m_ui->listWidget->selectedItems();
  foreach(QListWidgetItem *item, items) {
    m_ui->listWidget->remove(item);
  }
}

bool MKVSegmentJoiner::checkTools()
{
  QString lookingFor = "mkvinfo";
#ifdef Q_OS_WIN32
  lookingFor += ".exe";
  bool mkvinfo = QFile::exists(lookingFor);
#else
  QString lookingForUsrBin = "/usr/bin/mkvinfo";
  QString lookingForUsrLocalBin = "/usr/local/bin/mkvinfo";
  bool mkvinfoDef = QFile::exists(lookingFor);
  bool mkvinfoUsr = QFile::exists(lookingForUsrBin);
  bool mkvinfoUsrLocal = QFile::exists(lookingForUsrLocalBin);
  bool mkvinfo = false;
  if (mkvinfoDef || mkvinfoUsr || mkvinfoUsrLocal) {
    mkvinfo = true;
  }
#endif
  QString text = tr("mkvinfo exists: %1").arg(QString(mkvinfo ? "true" : "false"));

  lookingFor = "mkvpropedit";
#ifdef Q_OS_WIN32
  lookingFor += ".exe";
  bool mkvpropedit = QFile::exists(lookingFor);
  text += "\r\n";
#else
  lookingForUsrBin = "/usr/bin/mkvpropedit";
  lookingForUsrLocalBin = "/usr/local/bin/mkvpropedit";
  bool mkvpropeditDef = QFile::exists(lookingFor);
  bool mkvpropeditUsr = QFile::exists(lookingForUsrBin);
  bool mkvpropeditUsrLocal = QFile::exists(lookingForUsrLocalBin);
  bool mkvpropedit = false;
  if (mkvpropeditDef || mkvpropeditUsr || mkvpropeditUsrLocal) {
    mkvpropedit = true;
  }
  text += "\n";
#endif
  text += tr("mkvpropedit exists: %1").arg(QString(mkvpropedit ? "true" : "false"));

  if (mkvinfo && mkvpropedit) {
    return true;
  }
#ifdef Q_OS_WIN32
  text += "\r\n";
#else
  text += "\n";
#endif
  text += tr("Both are needed!");
  QMessageBox::information(this, tr("Error"), text);
  return false;
}

void MKVSegmentJoiner::on_connectPushButton_clicked()
{
  if (!this->checkTools() || m_running) {
    return;
  }
  m_segmentIds.clear();
  m_files.clear();
  this->grabSegmentIds();
}

void MKVSegmentJoiner::grabSegmentIds()
{
  int count = m_ui->listWidget->count();
  if (count < 2) {
    QMessageBox::information(this, tr("Info"), tr("At least two files are needed,.."));
    return;
  }
  for (int i = 0; i < count; ++i) {
    m_files << m_ui->listWidget->item(i)->text();
  }
  this->grabSegmentId();
}

void MKVSegmentJoiner::grabSegmentId()
{
  QString file = m_files.takeFirst();
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(handleMKVInfoOutput()));
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                   SLOT(mkvinfoFinished(int, QProcess::ExitStatus)));
  QString call = "mkvinfo";
#ifdef Q_OS_WIN32
  call += ".exe";
#endif
  call += " --ui-language";
  call += " ";
#if defined Q_OS_LINUX || defined Q_OS_DARWIN
  call += "en_US";
#endif
#ifdef Q_OS_WIN
  call += "en";
#endif
  call += " ";
  call += "\"" + file + "\"";
  m_foundId = false;
  m_ui->statusbar->showMessage(call);
  m_process->start(call);
}

void MKVSegmentJoiner::setSegmentIds()
{
  int count = m_ui->listWidget->count();
  if (count < 2) {
    QMessageBox::information(this, tr("Info"), tr("At least two files are needed,.."));
    return;
  }
  for (int i = 0; i < count; ++i) {
    m_files << m_ui->listWidget->item(i)->text();
  }
  m_previousId = QString();
  m_segmentIds.removeFirst();
  this->setSegmentId(true);
}

void MKVSegmentJoiner::setSegmentId(bool first)
{
  QString file = m_files.takeFirst();
  bool last = m_files.isEmpty();
  delete m_process;
  m_process = new QProcess(this);
  QObject::connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                   SLOT(mkvpropeditFinished(int, QProcess::ExitStatus)));
  QString call = "mkvpropedit";
  QString id;
#ifdef Q_OS_WIN32
  call += ".exe";
#endif
  call += " --ui-language";
  call += " ";
#if defined Q_OS_LINUX || defined Q_OS_DARWIN
  call += "en_US";
#endif
#ifdef Q_OS_WIN
  call += "en";
#endif
  call += " ";
  call += "\"" + file + "\"";
  call += " ";
  if (last) {
    call += "--delete next-uid";
  } else {
    id = m_segmentIds.takeFirst();
    call += "--set next-uid=";
    call += "\"" + id + "\"";
  }
  call += " ";
  if (first) {
    call += "--delete prev-uid";
  } else {
    call += "--set prev-uid=";
    call += "\"" + m_previousId + "\"";
  }
  m_previousId = id;
  m_ui->statusbar->showMessage(call);
  m_process->start(call);
}

void MKVSegmentJoiner::handleMKVInfoOutput()
{
  if (m_foundId) {
    return;
  }
  QString text = m_process->readAllStandardOutput().data();
  int index = text.indexOf("Segment UID:");
  if (index == -1) {
    return;
  }
  text = text.remove(0, index + 12);
  index = text.indexOf("|");
  if (index != -1) {
    text = text.remove(index, text.size());
    m_segmentIds << text.trimmed();
    m_foundId = true;
  }
}

void MKVSegmentJoiner::mkvinfoFinished(int exitCode, QProcess::ExitStatus)
{
  if (exitCode < 0) {
    QMessageBox::critical(this, tr("Crashed,.."),
                          tr("MkvInfo crashed with error code: %1").arg(exitCode));
    m_running = false;
    m_foundId = false;
    return;
  }
  if (!m_files.isEmpty()) {
    this->grabSegmentId();
  } else {
    m_foundId = false;
    this->setSegmentIds();
  }
}

void MKVSegmentJoiner::mkvpropeditFinished(int exitCode, QProcess::ExitStatus)
{
  if (exitCode < 0) {
    QMessageBox::critical(this, tr("Crashed,.."),
                          tr("MkvPropEdit crashed with error code: %1").arg(exitCode));
    m_running = false;
    return;
  }
  if (!m_files.isEmpty()) {
    this->setSegmentId();
  } else {
    m_ui->statusbar->showMessage(tr("Finished connecting the files!"));
  }
}
