#include "simple-recording.h"
#include "json-parser.hpp"
#include "source-record.h"
#include "ptz-controls.hpp"

#include <util/config-file.h>

#include <QDir>
#include <QDateTime>
#include <QGroupBox>
#include <QLabel>
#include <QDialog>
#include <QProcess>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

QVector<obs_source_t*> SimpleRecordingWidget::sources;

SimpleRecordingWidget::SimpleRecordingWidget(QWidget* parent)
  : QDockWidget("Concert Hall Video Rec Interface", parent), m_parent(parent), m_baseDir("V:/sb-terminal-test/")
{
  m_overrideBaseDirLineEdit = new QLineEdit(); // TODO: Remove; only for development
  m_mailAddressLineEdit = new QLineEdit();

  m_testButton = new QPushButton(); // TODO: Remove; only for development
  m_confirmMailAddressButton = new QPushButton();
  m_setDateButton = new QPushButton();
  m_confirmDateTimeButton = new QPushButton();
  m_recordButton = new QPushButton();

  m_setStartTimeLineEdit = new QLineEdit();
  m_setStopTimeLineEdit = new QLineEdit();

  m_startTimeEdit = new QTimeEdit();
  m_stopTimeEdit = new QTimeEdit();
 
  m_bookedTimesLabel = new QLabel();

  QWidget* widget = new QWidget();
  QVBoxLayout* mainLayout = new QVBoxLayout();

  // ! TODO: Seperate all layout init into functions. For this, all elements need to be class props!

  // --- Override base dir (only for development) ---
  QGroupBox* overrideBaseDirGroupBox = new QGroupBox();
  QHBoxLayout* overrideBaseDirLayout = new QHBoxLayout();
  overrideBaseDirGroupBox->setLayout(overrideBaseDirLayout);

  m_overrideBaseDirLineEdit->setPlaceholderText(m_baseDir);

  overrideBaseDirLayout->addWidget(m_overrideBaseDirLineEdit);
  overrideBaseDirLayout->addWidget(m_testButton);
  // --------------------------

  // --- Mail address input ---
  QGroupBox* mailAddressGroupBox = new QGroupBox();
  QHBoxLayout* mailAddressLayout = new QHBoxLayout();
  mailAddressGroupBox->setLayout(mailAddressLayout);

  m_testButton->setText("Misc. Tests");
  m_confirmMailAddressButton->setText("Confirm");
  m_recordButton->setText("Start Recording");

  m_mailAddressLineEdit->setPlaceholderText("Email Address");
  m_mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #FF5952; }");

  mailAddressLayout->addWidget(m_mailAddressLineEdit);
  // mailAddressLayout->addWidget(m_confirmMailAddressButton); // TODO: Remove Confirm Mail button entirely
  // -------------------------

  // --- Date & Time input ---
  QGroupBox* dateTimeGroupBox = new QGroupBox();
  QVBoxLayout* dateTimeLayout = new QVBoxLayout();
  dateTimeGroupBox->setLayout(dateTimeLayout);

  m_bookedTimesLabel = new QLabel();

  QGroupBox* timeGroupBox = new QGroupBox();
  QHBoxLayout* timeLayout = new QHBoxLayout();
  timeGroupBox->setLayout(timeLayout);

  QGroupBox* startTimeGroupBox = new QGroupBox();
  QHBoxLayout* startTimeLayout = new QHBoxLayout();
  startTimeGroupBox->setLayout(startTimeLayout);

  QGroupBox* endTimeGroupBox = new QGroupBox();
  QHBoxLayout* endTimeLayout = new QHBoxLayout();
  endTimeGroupBox->setLayout(endTimeLayout);

  QLabel* startTimeLabel = new QLabel("Start Time");
  QLabel* endTimeLabel = new QLabel("End Time");

  // TODO: remove?
  m_setStartTimeLineEdit->setPlaceholderText("00:00");
  m_setStopTimeLineEdit->setPlaceholderText("00:00");

  m_startTimeEdit->setDisplayFormat("hh:mm");
  m_stopTimeEdit->setDisplayFormat("hh:mm");

  startTimeLayout->addWidget(startTimeLabel);
  startTimeLayout->addWidget(m_startTimeEdit);

  endTimeLayout->addWidget(endTimeLabel);
  endTimeLayout->addWidget(m_stopTimeEdit);

  timeLayout->addWidget(startTimeGroupBox);
  timeLayout->addWidget(endTimeGroupBox);

  m_setDateButton->setText("Set Date");
  m_confirmDateTimeButton->setText("Confirm Date and Time");

  dateTimeLayout->addWidget(m_setDateButton);
  dateTimeLayout->addWidget(m_bookedTimesLabel);
  dateTimeLayout->addWidget(timeGroupBox);
  dateTimeLayout->addWidget(m_confirmDateTimeButton);
  // -------------------------

  mainLayout->addWidget(overrideBaseDirGroupBox);
  mainLayout->addWidget(mailAddressGroupBox);
  mainLayout->addWidget(dateTimeGroupBox);
  mainLayout->addWidget(m_recordButton);

  widget->setLayout(mainLayout);

  setWidget(widget); // from parent class
  setVisible(true);
  setFloating(true);
  setAllowedAreas({Qt::RightDockWidgetArea});
  resize(300, 300);

  // !-- TEST BUTTON ONLY FOR DEVELOPMENT
  QObject::connect(m_testButton, 
    &QPushButton::clicked, this, &SimpleRecordingWidget::onTestButtonClicked);
  // !-----------------------------------

  QObject::connect(m_confirmMailAddressButton, 
    &QPushButton::clicked, this, 
    &SimpleRecordingWidget::onConfirmMailAddressButtonClicked);

  QObject::connect(m_setDateButton,
    &QPushButton::clicked, this, 
    &SimpleRecordingWidget::onSetDateButtonClicked);

  QObject::connect(m_confirmDateTimeButton,
    &QPushButton::clicked, this, 
    &SimpleRecordingWidget::onConfirmDateTimeButtonClicked);

  QObject::connect(m_recordButton, 
    &QPushButton::clicked, this, 
    &SimpleRecordingWidget::onRecordButtonClicked);

  // TODO: Remove; only for development
   QObject::connect(m_overrideBaseDirLineEdit,
    &QLineEdit::textEdited, this, 
    &SimpleRecordingWidget::onOverrideBaseDirLineEdited);

  QObject::connect(m_mailAddressLineEdit,
    &QLineEdit::textEdited, this, 
    &SimpleRecordingWidget::onMailAddressLineEdited);
}

SimpleRecordingWidget::~SimpleRecordingWidget()
{}

QString SimpleRecordingWidget::sendDownloadLink(const Booking& booking, const QString& path)
{
  // TODO: Check how to set up relative paths!
  const QString dllFilePath = QCoreApplication::applicationFilePath();
  const QString dllDir = QFileInfo(dllFilePath).absolutePath();

  const QString nodePath = QDir::toNativeSeparators(dllDir + "../../node.exe");
  const QString scriptPath = QDir::toNativeSeparators(dllDir + "../../node-scripts/file-sender.js");

  const QStringList nodeArgs = QStringList() << scriptPath;

  QProcess* nodeProcess = new QProcess(this);
  nodeProcess->setEnvironment(QProcess::systemEnvironment());
  nodeProcess->setEnvironment(QStringList() << "DEBUG_MODE=false");

  nodeProcess->start(nodePath, nodeArgs);

  if (!nodeProcess->waitForStarted()) 
    return "Process started with error: " + nodeProcess->errorString();

  const QString apiBasePath = 
    QString("/team-folders/video/") + m_baseDir.last(m_baseDir.size() - 3);

  // TODO: Add mail subject!

  QJsonObject jsonObj;
  jsonObj["basePath"] = apiBasePath;
  jsonObj["path"] = path;
  jsonObj["receiver"] = booking.email;
  jsonObj["subject"] = "HfMDD Concert Hall Recordings " + booking.date.toString("ddd MMM dd yyyy");

  QJsonDocument jsonDoc(jsonObj);

  nodeProcess->write(jsonDoc.toJson());
  nodeProcess->closeWriteChannel();

  if (!nodeProcess->waitForFinished())
    return "Process finished with error: " + nodeProcess->errorString();

  const QByteArray output = nodeProcess->readAllStandardOutput();
  return QString::fromUtf8(output);
}


// !-- TEST BUTTON ONLY FOR DEVELOPMENT
void SimpleRecordingWidget::onTestButtonClicked()
{
  resetFilterSettings();
}
// !-----------------------------------


void SimpleRecordingWidget::onConfirmMailAddressButtonClicked()
{
  m_email = m_mailAddressLineEdit->text();

  m_mailAddressIsValid = false;

  const QString mailSuffices[] = {"@hfmdd.de", "@mailbox.hfmdd.de", "@gmx.net"}; // TODO: Remove gmx 
  
  for (const QString& suffix : mailSuffices)
  {
    if (m_email.endsWith(suffix))
    {
      m_mailAddressIsValid = true;
      m_innerDir = m_email.chopped(suffix.length());
      break;
    }
  }

  if (!m_mailAddressIsValid)
    QMessageBox::information(this, "Error", "Invalid email address");
}

void SimpleRecordingWidget::onSetDateButtonClicked()
{
  QDialog* calendarWindow = new QDialog(this);
  QGridLayout* calendarLayout = new QGridLayout();

  m_calendar = new QCalendarWidget();
  m_calendar->setMinimumDate(QDate::currentDate());
  m_calendar->setMaximumDate(QDate::currentDate().addYears(1));
  m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
  m_calendar->setFirstDayOfWeek(Qt::Monday);

  QObject::connect(m_calendar, &QCalendarWidget::clicked, 
    this, &SimpleRecordingWidget::onCalendarCellClicked);

  calendarLayout->addWidget(m_calendar, 0, 0, Qt::AlignCenter);
  calendarWindow->setLayout(calendarLayout);

  calendarWindow->show();
}

void SimpleRecordingWidget::onConfirmDateTimeButtonClicked()
{
  QTime startTime = m_startTimeEdit->time();
  QTime stopTime = m_stopTimeEdit->time();

  m_dateTimeIsValid = true;

  if (!m_selectedDateBookings.isEmpty())
  {
    for (const Booking& b : m_selectedDateBookings)
      {
        if ((startTime >= b.startTime && startTime < b.stopTime) ||
            (stopTime > b.startTime && stopTime <= b.stopTime))
            {
              m_dateTimeIsValid = false;
              QMessageBox::information(this, "Info", 
                "Colliding bookings:\n" + b.email + "\n" + b.startTime.toString() + "\n" + b.stopTime.toString());
            }
      }
  }
  
  if (!m_dateTimeIsValid)
  {
    QMessageBox::information(this, "Error", "Chosen timespan collides with existing bookings");
    return;
  }

  if (!m_mailAddressIsValid)
  {
    QMessageBox::information(this, "Error", "Invalid email address");
    return;
  }

  Booking booking =
  {
    m_mailAddressLineEdit->text(),
    m_selectedDate,
    startTime,
    stopTime,
    QString("Konzert/Prüfung/Klassenabend")
  };
  
  JsonParser::addBooking(booking);

  m_outerDir = m_selectedDate.toString(Qt::ISODate) + "/";
  const QString newFilePath = m_baseDir + m_outerDir + m_innerDir;
  QDir newRecDir = QDir(newFilePath);

  if (!newRecDir.exists())
    newRecDir.mkpath(QString(newFilePath));

  QByteArray filePathByteArray = newFilePath.toLocal8Bit();
  const char* c_newFilePath = filePathByteArray.data();
  config_t* currentProfile = obs_frontend_get_profile_config();

  config_set_string(currentProfile, "SimpleOutput", "FilePath", c_newFilePath);

  // TODO: This is a duplicated variable. Remove!
  const char* path = config_get_string(currentProfile, "SimpleOutput", "FilePath");

  updateFilterSettings(path);

  // TODO: Remove; only for development
  QMessageBox::information(this, "Info", QString("Recordings will be saved at ") + QString(path));
}

void SimpleRecordingWidget::onRecordButtonClicked()
{
  if (!m_mailAddressIsValid)
  {
    QMessageBox::information(this, "Error", "Invalid email address");
    return;
  }

  if (!m_dateTimeIsValid)
  {
    QMessageBox::information(this, "Error", "Invalid date or time");
    return;
  }

  if (!obs_frontend_recording_active())
  {
    obs_frontend_recording_start();
    m_recordButton->setText("Stop Recording");
  }

  else 
  {
    obs_frontend_recording_stop();
    m_recordButton->setText("Start Recording");

    // ...
    // QString url = sendDownloadLink(m_email, m_outerDir + m_innerDir);
    // QMessageBox::information(this, "Info", url);
  }
}

void SimpleRecordingWidget::onCalendarCellClicked(const QDate& date)
{
  m_selectedDate = date;

  if (m_bookedTimesLabel == nullptr) return;
  
  JsonParser::getBookingsOnDate(date, m_selectedDateBookings);

  if (m_selectedDateBookings.isEmpty())
  {
    m_bookedTimesLabel->setText("There are no bookings yet on " + date.toString());
    return;
  }

  QString str = "The following times have been booked on " + date.toString() + ":\n";

  for (const Booking& b : m_selectedDateBookings)
    str += b.startTime.toString() + "-" + b.stopTime.toString() + "\n";

  m_bookedTimesLabel->setText(str);
}

void SimpleRecordingWidget::onOverrideBaseDirLineEdited(const QString& text)
{
  // m_baseDir = m_overrideBaseDirLineEdit->text();

  // if (!m_baseDir.endsWith("/"))
  //   m_baseDir.append("/");

  // JsonParser::setBookingsPath(m_baseDir);

  m_currentPresetName = m_overrideBaseDirLineEdit->text();
}

void SimpleRecordingWidget::onMailAddressLineEdited(const QString& text)
{
  m_email = m_mailAddressLineEdit->text();

  m_mailAddressIsValid = false;

  const QString mailSuffices[] = { "@hfmdd.de", "@mailbox.hfmdd.de", "@gmx.net" }; // TODO: Remove gmx
  
  for (const QString& suffix : mailSuffices)
  {
    if (m_email.endsWith(suffix))
    {
      m_mailAddressIsValid = true;
      m_innerDir = m_email.chopped(suffix.length());
      break;
    }
  }

  QString styleSheet;

  if (m_mailAddressIsValid)
    styleSheet = "QLineEdit { border: 2px solid #48FF8B }";

  else
    styleSheet = "QLineEdit { border: 2px solid #FF5952 }";

  m_mailAddressLineEdit->setStyleSheet(styleSheet);
}

static bool scene_enum_callback(obs_scene_t* scene, obs_sceneitem_t* item, void*) 
{
    obs_source_t* source = obs_sceneitem_get_source(item);

    if (source) 
      SimpleRecordingWidget::sources.append(source);

    return true; // Returning true continues the enumeration
}

static void filter_enum_callback(obs_source_t* source, obs_source_t* filter, void*)
{
  if (!source || !filter) return;

  obs_source_filter_remove(source, filter);
}

void SimpleRecordingWidget::updateFilterSettings(const char* path)
{
  obs_source_t* currentScene = obs_frontend_get_current_scene();

  if (!currentScene) return;

  obs_scene_t* scene = obs_scene_from_source(currentScene);
  
  if (!scene) return;

  config_t* currentProfile = obs_frontend_get_profile_config();
  QString filenameFormatting = 
    config_get_string(currentProfile, "Output", "FilenameFormatting");

  sources.clear();
  obs_scene_enum_items(scene, &scene_enum_callback, NULL);
   
  for (obs_source_t* source : sources)
  {
    if (!source) continue;

    QString sourceName = obs_source_get_name(source);
    QString newFormatting = filenameFormatting + "-" + sourceName;
    obs_source_t* f = obs_source_get_filter_by_name(source, "Source Record");
    obs_data_t* settings = obs_source_get_settings(f);

    // file_formatting could be set manually...
    obs_data_set_string(settings, "filename_formatting", newFormatting.toUtf8().constData());
    obs_data_set_string(settings, "path", path);

    obs_data_release(settings);
  }
}

// TODO: Check if any source has 0 or more than 1 filter; if so, reset!?
void SimpleRecordingWidget::resetFilterSettings()
{
  obs_source_t* currentScene = obs_frontend_get_current_scene();

  if (!currentScene) return;
  
  obs_scene_t* scene = obs_scene_from_source(currentScene);

  if (!scene) return;

  void* unused = nullptr; 
  
  obs_source_info* sourceRecordFilterInfo = get_source_record_filter_info();
  const char* id = sourceRecordFilterInfo->id;
  QString sourceRecordFilterName = sourceRecordFilterInfo->get_name(unused);

  // pass path as input param?
  QString path = m_baseDir + m_outerDir + m_innerDir;

  config_t* currentProfile = obs_frontend_get_profile_config();
  QString filenameFormatting = 
    config_get_string(currentProfile, "Output", "FilenameFormatting");

  sources.clear();
  obs_scene_enum_items(scene, &scene_enum_callback, NULL);
  
  for (obs_source_t* source : sources)
  {
    if (!source) continue;

    obs_source_enum_filters(source, filter_enum_callback, nullptr);

    obs_data_t* settings = obs_data_create();
    sourceRecordFilterInfo->get_defaults(settings);

    QString sourceName = obs_source_get_name(source);
    QString newFormatting = filenameFormatting + "-" + sourceName;

    QString filterName = sourceRecordFilterName + "-" + sourceName;

    obs_source_t* filter = obs_source_create(id, filterName.toUtf8().constData(), settings, NULL);
    obs_source_filter_add(source, filter);
    
    obs_data_set_int(settings, "record_mode", OUTPUT_MODE_RECORDING);
    obs_data_set_string(settings, "filename_formatting", newFormatting.toUtf8().constData());
    obs_data_set_string(settings, "rec_format", "mkv");
    obs_data_set_string(settings, "encoder", "nvenc_hevc");
    obs_data_set_string(settings, "preset2", "p7");
     
    // ? For finding the available encoder props.
    // const char* encId = get_encoder_id(settings);
    // obs_properties_t* encProps = obs_get_encoder_properties(encId);
    // obs_property_t* p = obs_properties_first(encProps);
    
    // QString propsInfo = "";

    // while (p)
    // {
    //   QString name = obs_property_name(p);
    //   propsInfo.append(name + "\n");
    //   obs_property_next(&p);
    // }
    
    // QMessageBox::information(this, "available encoder props", propsInfo);
  
    obs_data_release(settings);
  }
}