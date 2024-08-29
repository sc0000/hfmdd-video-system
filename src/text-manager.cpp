#include <QString>

#include "message-dialog.hpp"
#include "widgets.hpp"
#include "text-manager.hpp"

QMap<ETextId, QString>* TextManager::currentTexts = &TextManager::english;
QLocale TextManager::locale = QLocale::English;

void TextManager::translate(const QString language) 
{
    if (language == "English") {
      TextManager::currentTexts = &TextManager::english;
      locale = QLocale::English;
    }
      
    else if (language == "Deutsch") {
      TextManager::currentTexts = &TextManager::german;
      locale = QLocale::German;
    }
}

const QString TextManager::getText(ETextId id)
{
  return TextManager::currentTexts->value(id, "Text/Label not found");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//---TEXTS--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

QMap<ETextId, QString> TextManager::english = {
  {
    ID::LOGIN_INFO, 
    QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">What am I looking at?</span><br/>" +
      "This is a system with which every member of HfMDD can independently record their concert hall performances on video.<br/><br/>" +
      "<span style=\"font-weight: bold;\">How does it work?</span><br/>First, log in with your school email (@hfmdde.de or @mailbox.hfmdd.de). " +
      "Please make sure to spell it correctly, otherwise you might not receive the download link for your videos.<br/>" +
      "Then, you have two options: You could make a recording right away, only setting the stop time.<br/>"
      "However, you could also work with the booking system, and reserve a time slot to record a video at a later time or date.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Anything to be aware of?</span><br/>" +
      "No matter which route you'll take, the system will let you know of any other planned recordings that might be in conflict with what you're planning.<br/>" +
      "As of yet, you are not technically prohibited to make conflicting bookings, or let a recording run into another booked timeslot.<br/>" +
      "Therefore, it is all the more important to communicate with whomever you're colliding with. Conflicting bookings might also be reviewed by the admin, and, if needed, adjusted.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Who can I talk to about this?</span><br/>For questions and feedback, please contact Oliver Fenk (oliver.fenk@hfmdd.de)." +
      "</body></html>"
  },

  {
    ID::LOGIN_REMINDER,
    "Please log in with your HfMDD email address!"
  },

  {
    ID::MODE_QUICK,
    "Record a video right now"
  },

  {
    ID::MODE_BOOKING,
    "Book a video recording session in the future,\nmanage or carry out a previously booked session"}, 

  {
    ID::QUICK_UNTIL,
    "Record until..."
  },

  {
    ID::QUICK_FROM_NOW,
    "min from now"
  },

  {
    ID::QUICK_TO_PTZ,
    "Go to Camera Controls"
  },

  {
    ID::QUICK_BACK,
    "Go back to Mode Selection"},

  {
    ID::QUICK_INFO,
    QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">What am I looking at?</span><br/>" +
      "From this screen, you can set a stop time and move on to record a video right away. " +
      "Just take other booked recordings happening today into account.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Why do I even have to set a stop time?</span><br/>" +
      "You can stop the recording whenever you want, but " +
      "<span style=\"font-weight: bold;\">(IMPORTANT) </span>" +
      "the recording will stop automatically 10 minutes after the designated time!<br/><br/>" +
      "<span style=\"font-weight: bold;\">And then what?</span><br/>" +
      "No matter how the recording was stopped, you will receive a download link via email afterwards." +
      "</body></html>"
  },

  {
    ID::QUICK_PREV_BOOKINGS_NONE,
    "There are no bookings yet today."
  },

  {
    ID::QUICK_PREV_BOOKINGS,
    "The following times have been booked later today:\n"
  },

  {
    ID::OVERVIEW_NEW,
    "New Booking"
  },

  {
    ID::OVERVIEW_EDIT,
    "Edit Booking"
  },

  {
    ID::OVERVIEW_EDIT_NONE_SELECTED,
    "Please select a booking you want to edit."
  },

  {
    ID::OVERVIEW_EDIT_TOO_MANY_SELECTED,
    "Please select only one booking to edit."
  },

  {
    ID::OVERVIEW_DELETE,
    "Delete Booking"
  },

  {
    ID::OVERVIEW_DELETE_NONE_SELECTED,
    "Please select a booking you want to edit."
  },

  {
    ID::OVERVIEW_DELETE_TOO_MANY_SELECTED,
    "Please select only one booking to edit."
  },

  {
    ID::OVERVIEW_DELETE_CONFIRM,
    "Do you really want to delete the selected booking? This cannot be undone."
  },

  {
    ID::OVERVIEW_TO_PTZ,
    "Go to Camera Controls"
  },

  {
    ID::OVERVIEW_TO_PTZ_NONE_SELECTED,
    "Please select a booking to continue."
  },

  {
    ID::OVERVIEW_TO_PTZ_TOO_MANY_SELECTED,
    "Please select only one booking to continue."
  },

  {
    ID::OVERVIEW_BACK,
    "Go back to Mode Selection"
  }, 

  {
    ID::OVERVIEW_INFO,
    QString("<html><head/><body>") +
      "<span style=\"font-weight: bold;\">What am I looking at?</span><br/>" + 
      "From this screen, you can book video recording sessions, or edit existing ones. " + 
      "Please note that you have to select a booking to continue to camera controls.<br/><br/>" +
      "<span style=\"font-weight: bold;\">I got a conflict warning when I made my booking..?</span><br/>"
      "In general, you can book and work with time slots that are conflicting with others, but please be aware " +
      "that these might be subject to adjustments by the admin.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Anything else I need to know?</span><br/>" +
      "You are allowed to start a recording 15 minutes before the designated time at the earliest, however, " +
      "you can set up your cameras and save presets anytime. " +
      "You can stop the recording whenever you want, but " +
      "<span style=\"font-weight: bold;\">(IMPORTANT) </span>" +
      "the recording will stop automatically 10 minutes after the designated time!<br/><br/>" +
      "<span style=\"font-weight: bold;\">And then what?</span><br/>" +
      "No matter how the recording was stopped, you will receive a download link via email afterwards." +
      "</body></html>"
  },

  {
    ID::EDITOR_DATE_NONE_SELECTED,
    "No date selected."
  },

  {
    ID::EDITOR_TIME_START,
    "Edit Start Time"
  },

  {
    ID::EDITOR_TIME_STOP,
    "Edit Stop Time"
  },

  {
    ID::EDITOR_EVENT,
    "Type of Event"
  },

  {
    ID::EDITOR_EVENT_PLACEHOLDER,
    "Exam? Concert? Rehearsal? Dedicated Video Recording?",
  },

  {
    ID::EDITOR_SAVE,
    "Save"
  },

  {
    ID::EDITOR_CANCEL,
    "Cancel"
  },

  {
    ID::EDITOR_PREV_BOOKINGS_NONE,
    "There are no other bookings yet on "
  },

  {
    ID::EDITOR_PREV_BOOKINGS,
    "The following times have been booked on "
  },

  {
    ID::EDITOR_EVENT_MISSING,
    "Please specify the type of event!"
  },

  {
    ID::EDITOR_IDENTICAL_TIMES,
    "Start and stop time are identical. Please select a reasonable time frame!"
  },

  {
    ID::EDITOR_CONFLICTING_BOOKINGS,
    "This booking is conflicting. Proceed?"
  },

  {
    ID::CONTROLS_TITLE,
    "Camera Controls"
  },

  {
    ID::CONTROLS_CAMERA_PREV,
    "Previous\nCamera"
  },

  {
    ID::CONTROLS_CAMERA_NEXT,
    "Next\nCamera"
  },

  {
    ID::CONTROLS_CAMERA_OVERVIEW,
    "Camera Overview"
  },

  {
    ID::CONTROLS_CAMERA_SINGLE,
    "Single Camera View"
  },

  {
    ID::CONTROLS_PRESET_SAVE,
    "Save"
  },

  {
    ID::CONTROLS_PRESET_LOAD,
    "Load"
  },

  {
    ID::CONTROLS_PRESET_LOAD_NONE_SELECTED,
    "Please select the preset you want to load."
  },

  {
    ID::CONTROLS_PRESET_DELETE,
    "Delete"
  },

  {
    ID::CONTROLS_PRESET_DELETE_NONE_SELECTED,
    "Please select the preset you want to delete."
  },

  {
    ID::CONTROLS_PRESET_DELETE_DENIED,
    "You don't have permission to delete this preset."
  },

  {
    ID::CONTROLS_PRESET_DELETE_CONFIRM,
    "Do you really want to delete the selected preset? This cannot be undone."
  },

  {
    ID::CONTROLS_RECORD,
    "Record"
  },

  {
    ID::CONTROLS_BACK_OVERVIEW,
    "To Bookings Overview"
  },

  {
    ID::CONTROLS_BACK_QUICK,
    "To Time Slot Setup"
  },

  {
    ID::CONTROLS_SETTINGS_DENIED,
    "You don't have permission to access the settings menu."
  },

  {
    ID::CONTROLS_INFO,
    QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">What can I do here?</span><br/>" +
      "With the arrow keys and the surrounding ones, you can configure each camera. Cycle through them with the topmost buttons, " +
      "or by clicking on the preview in overview mode.<br/>" +
      "Save your current configuration as a preset, or load previously saved presets. Finally, you can start and stop the recording.<br/><br/>" +
      "<span style=\"font-weight: bold;\">What's with the presets already in the list?</span><br/>" +
      "Additional to your own presets, you have access to a couple of standard presets provided by the admin.<br/><br/>" +
      "<span style=\"font-weight: bold;\">What do the numbers behind the preset names mean?</span><br/>"
      "Nothing of significance, just the (global) order in which the presets where saved. The numbers will be removed at some point.<br/><br/>" +
      "</body></html>"
  },

  {
    ID::CONFLICT,
    "\t--CONFLICTING!"
  },

  {
    ID::MAIL_SUBJECT,
    "HfMDD Concert Hall Recordings "
  },
};

QMap<ETextId, QString> TextManager::german = {
  {ID::LOGIN_INFO,
  
      QString("<html><head/><body>") + 
        "<span style=\"font-weight: bold;\">Was ist das hier?</span><br/>" +
        "Das hier ist ein System, mit dem jedes Mitglied der HfMDD seine Konzertsaalauftritte selbstständig auf Video aufzeichnen kann.<br/><br/>" +
        "<span style=\"font-weight: bold;\">Wie funktioniert das?</span><br/>Melden Sie sich zunächst mit Ihrer Hochschul-Mailadresse an (@hfmdd.de oder @mailbox.hfmdd.de). " +
        "Bitte achten Sie auf die richtige Schreibweise, andernfalls kann der Downloadlink für Ihre Videos nicht zugestellt werden." +
        "Dann haben Sie zwei Möglichkeiten:<br/>Sie können direkt mit der Aufnahme beginnen und nur die Stoppzeit angeben, "
        "oder das Buchungssystem nutzen und ein Zeitfenster für eine Aufnahme reservieren.<br/><br/>" +
        "<span style=\"font-weight: bold;\">Gibt es etwas, worauf ich achten muss?</span><br/>" +
        "Das System weist Sie auf bereits existierende Buchungen hin, die möglicherweise mit Ihren Plänen kollidieren.<br/>" +
        "Aktuell ist es durchaus möglich, kollierende Zeitfenster zu buchen, wie auch in reservierte Zeitfenster hinein aufzunehmen. " +
        "In solchen Fällen ist es ratsam, ggf. den Kontakt zur betreffenden Person zu suchen. Kollidierende Buchungen werden außerdem vom Administrator überprüft und Umständen entsprechend angepasst.<br/><br/>" +
        "<span style=\"font-weight: bold;\">An wen kann ich mich bei Fragen und Problemen wenden?</span><br/>Oliver Fenk (oliver.fenk@hfmdd.de)." +
        "</body></html>"},

  
  {
    ID::LOGIN_REMINDER,
    "Bitte loggen Sie sich mit Ihrer HfMDD-E-Mail-Adresse ein!"
  },

  {
    ID::MODE_QUICK,
    "Direkt ein Video aufnehmen"
  },

  {
    ID::MODE_BOOKING,
    "Eine Aufnahmesession buchen,\noder eine zuvor gebuchte Session durchführen"
  }, 

  {
    ID::QUICK_UNTIL,
    "Aufnehmen bis..."
  },

  {
    ID::QUICK_FROM_NOW,
    "min ab jetzt"
  },

  {
    ID::QUICK_TO_PTZ,
    "Zur Kamerasteuerung"
  },

  {
    ID::QUICK_BACK,
    "Zurück"
  },

  {
    ID::QUICK_INFO,
    QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">Was kann ich hier machen?</span><br/>" +
      "Von diesem Bildschirm können Sie direkt weiter zur Videoaufnahme. " +
      "Dafür müssen Sie lediglich die Zeit bestimmen, zu der die Aufnahme voraussichtlich endet.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Warum muss ich überhaupt die Stoppzeit angeben?</span><br/>" +
      "Sie können die Aufnahme jederzeit stoppen, aber " +
      "<span style=\"font-weight: bold;\">(WICHTIG) </span>" +
      "10 Minuten nach der angegeben Zeit wird die Aufnahme automatisch gestoppt!<br/><br/>" +
      "<span style=\"font-weight: bold;\">Was passiert danach?</span><br/>" +
      "Nach Ende der Aufnahme (egal ob manuell oder automatisch ausgelöst) erhalten Sie einen Downloadlink per E-Mail." +
      "</body></html>"
  },

  {
    ID::QUICK_PREV_BOOKINGS_NONE,
    "Es gibt noch keine Buchungen am heutigen Tag."},

  {
    ID::QUICK_PREV_BOOKINGS,
    "Heute sind bereits folgende Zeiten gebucht:\n"
  },

  {
    ID::OVERVIEW_NEW,
    "Neue Buchung"
  },

  {
    ID::OVERVIEW_EDIT,
    "Buchung bearbeiten"
  },

  {
    ID::OVERVIEW_EDIT_NONE_SELECTED,
    "Bitte wählen Sie eine Buchung aus, die Sie bearbeiten wollen."
  },

  {
    ID::OVERVIEW_EDIT_TOO_MANY_SELECTED,
    "Bitte wählen Sie nur eine Buchung zur Bearbeitung aus."
  },

  {
    ID::OVERVIEW_DELETE,
    "Buchung löschen"
  },

  {
    ID::OVERVIEW_DELETE_NONE_SELECTED,
    "Bitte wählen Sie eine Buchung aus, die Sie löschen möchten."
  },

  {
    ID::OVERVIEW_DELETE_TOO_MANY_SELECTED,
    "Bitte wählen Sie nur eine Buchung zur Bearbeitung aus."
  },

  {
    ID::OVERVIEW_DELETE_CONFIRM,
    "Wollen Sie diese Buchung wirklich unwiderruflich löschen?"
  },

  {
    ID::OVERVIEW_TO_PTZ,
    "Zur Kamerasteuerung"
  },

  {
    ID::OVERVIEW_TO_PTZ_NONE_SELECTED,
    "Bitte wählen Sie eine Buchung aus, um fortzufahren."
  },

  {
    ID::OVERVIEW_TO_PTZ_TOO_MANY_SELECTED,
    "Bitte wählen Sie nur eine Buchung aus, um fortzufahren."
  },

  {
    ID::OVERVIEW_BACK,
    "Zurück"
  }, 

  {
    ID::OVERVIEW_INFO,
    QString("<html><head/><body>") +
      "<span style=\"font-weight: bold;\">Was kann ich hier machen?</span><br/>" + 
      "Hier können Sie Videosessions buchen oder vorhandene Buchungen bearbeiten.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Mir wurde eine Warnung angezeigt, als ich meine letzte Buchung abschließen wollte...?</span><br/>"
      "Grundsätzlich können Sie auch Zeitfenster buchen und nutzen, die mit anderen kollidieren. " +
      "Beachten Sie jedoch, dass diese möglicherweise vom Administrator etwas angepasst werden. " +
      "Damit hier eine sinnvolle Entscheidung getroffen werden kann, müssen Sie die Art der Veranstaltung vermerken.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Was muss ich noch wissen?</span><br/>" +
      "Sie können eine Aufnahme frühestens 15 Minuten vor der angegeben Zeit starten. Allerdings " +
      "können Sie jederzeit die Kameras konfigurieren und diese Einstellung als Preset speichern. " +
      "Sie können die Aufnahme jederzeit stoppen, aber " +
      "<span style=\"font-weight: bold;\">(WICHTIG) </span>" +
      "10 Minuten nach der angegeben Zeit wird die Aufnahme automatisch gestoppt!<br/><br/>" +
      "<span style=\"font-weight: bold;\">Was passiert danach?</span><br/>" +
      "Nach Ende der Aufnahme (egal ob manuell oder automatisch ausgelöst) erhalten Sie einen Downloadlink per E-Mail." +
      "</body></html>"
  },

  {
    ID::EDITOR_DATE_NONE_SELECTED,
    "Kein Datum ausgewählt"
  },

  {
    ID::EDITOR_TIME_START,
    "Startzeit ändern"
  },

  {
    ID::EDITOR_TIME_STOP,
    "Stopzeit ändern"
  },

  {
    ID::EDITOR_EVENT,
    "Art der Veranstaltung"
  },

  {
    ID::EDITOR_EVENT_PLACEHOLDER,
    "Prüfung? Konzert? Probe? Bewerbungsvideodreh?"
  },

  {
    ID::EDITOR_SAVE,
    "Speichern"
  },

  {
    ID::EDITOR_CANCEL,
    "Verwerfen"
  },

  {
    ID::EDITOR_PREV_BOOKINGS_NONE,
    "Es gibt keine anderen Buchungen am "
  },

  {
    ID::EDITOR_PREV_BOOKINGS,
    "Folgende Zeiten wurden bereits gebucht am "
  },

  {
    ID::EDITOR_EVENT_MISSING,
    "Bitte geben Sie die Art der Veranstaltung an!"
  },

  {
    ID::EDITOR_IDENTICAL_TIMES,
    "Start- und Stopzeit sind identisch. Bitte wählen Sie einen geeigneten Zeitraum!"
  },

  {
    ID::EDITOR_CONFLICTING_BOOKINGS,
    "Diese Buchung kollidiert mit einer anderen. Fortfahren?"
  },

  {
    ID::CONTROLS_TITLE,
    "Kamerasteuerung"
  },

  {
    ID::CONTROLS_CAMERA_PREV,
    "Vorherige\nKamera"
  },

  {
    ID::CONTROLS_CAMERA_NEXT,
    "Nächste\nKamera"
  },

  {
    ID::CONTROLS_CAMERA_OVERVIEW,
    "Gesamtansicht"
  },

  {
    ID::CONTROLS_CAMERA_SINGLE,
    "Einzelansicht"
  },

  {
    ID::CONTROLS_PRESET_SAVE,
    "Speichern"
  },

  {
    ID::CONTROLS_PRESET_LOAD,
    "Laden"
  },

  {
    ID::CONTROLS_PRESET_LOAD_NONE_SELECTED,
    "Bitte wählen Sie ein Preset aus,\ndas Sie laden möchten."
  },

  {
    ID::CONTROLS_PRESET_DELETE,
    "Löschen"
  },

  {
    ID::CONTROLS_PRESET_DELETE_NONE_SELECTED,
    "Bitte wählen Sie ein Preset aus,\ndas Sie löschen möchten."
  },

  {
    ID::CONTROLS_PRESET_DELETE_DENIED,
    "Sie haben keine Befugnis, dieses Preset zu löschen."
  },

  {
    ID::CONTROLS_PRESET_DELETE_CONFIRM,
    "Möchten Sie das ausgewählte Preset wirklich unwiderruflich löschen?"
  },

  {
    ID::CONTROLS_RECORD,
    "Aufnehmen"
  },

  {
    ID::CONTROLS_BACK_OVERVIEW,
    "Zurück"
  },

  {
    ID::CONTROLS_BACK_QUICK,
    "Zurück"
  },

  {
    ID::CONTROLS_SETTINGS_DENIED,
    "Sie haben keine Befugnis, die Einstellungen zu öffnen."
  },

  {
    ID::CONTROLS_INFO,
    QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">Was kann ich hier machen?</span><br/>" +
      "Mit den Pfeiltasten können Sie die Kameras ausrichten, außerdem rein- und rauszoomen, sowie den Fokus justieren. " +
      "Sie können mithilfe der obersten Knöpfe zwischen den Kameras hin- und herschalten, oder indem Sie im Übersichtsmodus auf das jeweilige Bild klicken.<br/>" +
      "Sie können ihre aktuelle Konfiguration als Preset speichern, oder ein bereits vorhandenes Preset laden. " +
      "Außerdem können Sie die Aufnahme hier natürlich starten und stoppen.<br/><br/>" +
      "<span style=\"font-weight: bold;\">In der Liste sind ja bereits einige Presets..?</span><br/>" +
      "Zusätzlich zu Ihren eigenen Presets finden Sie ein paar vom Administrator bereit gestellte.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Was bedeuten die Nummern hinter den Presets?</span><br/>"
      "Die Nummern zeigen lediglich die Reihenfolge an, in der die Presets gespeichert wurden, und werden voraussichtlich in einem kommenden Update entfernt." +
      "</body></html>"
  },

  {
    ID::CONFLICT,
    "\t--BUCHUNGSKONFLIKT!"
  },

  {
    ID::MAIL_SUBJECT,
    "HfMDD Konzertsaal -- Aufnahme "
  },
};