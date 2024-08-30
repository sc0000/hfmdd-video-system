#pragma once

#include "booking-handler.hpp"

enum class EMailType
{
  SendFiles,
  BookingConflictWarning,
  Default
};

class MailHandler 
{
public:
  static QString nasIP;
  static QString nasPort;
  static QString nasUser;
  static QString nasPassword;
  
  static QString mailHost;
  static QString mailUser;
  static QString mailPassword;
  static QString mailSenderAddress;

  static QString adminEmail;
  static QString adminPassword;

  static QString currentEmail;
  static bool mailAddressIsValid;

  static QVector<QString> mailSuffices;

  static QString sendMail(const Booking& booking, EMailType mailType);

  static void saveCredentials();
  static void loadCredentials();
  
};