/*
 *
 * POI Manager
 * Copyright (C) 2021  "Steve Clarke www.vizier.uk/poi"
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Any modification to the code must be contributed back to the community.
 *
 * Redistribution and use in binary or source form with or without modification
 * is permitted provided that the following conditions are met:
 *
 * Clause 7b - attribution for the original author shall be provided with the
 * inclusion of the above Copyright statement in its entirity, which must be
 * clearly visible in each application source file, in any documentation and also
 * in a pop-up window in the application itself. It is requested that the charity
 * donation link to Guide Dogs for the Blind remain within the program, and any
 * derivitive thereof.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GOOGLEACCESS_H
#define GOOGLEACCESS_H

//#include <QJsonObject>
//#include <QDomElement>
#include <QString>
//#include <QList>
#include <QNetworkReply>


class GoogleAccess
{
public:
    enum googleAction {
        Post = 0,      // Create New Contact / Calendar Entry
        Put = 1,       // Not Used / Update Calendar Entry
        Delete = 2,    // Delete Contact / Calendar Entry
        Patch = 3      // Update Contact

    };

public:

  // Constructor, sets the refresh token which has been
  // previously generated with googleAuthorise.
  GoogleAccess(QString clientid, QString scope, QString secret);

  // Calls either AuthoriseLimitedInput() or AuthoriseDesktopApplication()
  // to request access permission from the user.
  bool Authorise() ;

  // Return the username associated with the current
  // google account
  QString getUsername() ;

  // Fetch information via a http get
  QString googleGet(QString link) ;

  // Fetch information via an http put or post
  QString googlePutPostDelete(QString link, enum googleAction action, QString data = "") ;

  // Unused copy construct
  GoogleAccess(GoogleAccess& other) ;
  GoogleAccess& operator =(const GoogleAccess &rhs) ;

  // Return the last network error string, or "" if OK
  QString getNetworkError() ;
  int getNetworkErrorCode() ;

  // Returns true if last network error was network-connection related
  bool isConnectionError() ;

private:

    QString googleGetResponse ;
    QString googlePutPostResponse ;
    QString clientid, scope, secret ;
    QString username, refreshtoken ;
    QString accesstoken  ;
    QString errorstatus ;
    bool connectionerror ;
    int errorcode ;

    // Check SSL Loaded (return version string or "")
    QString checkSsl() ;

    // Gets an access token, using the refresh token
    void googleGetAccessToken() ;

    // Extract error code and update errorcode/errorstatus
    void ExtractErrorCode(QNetworkReply *reply) ;

    // Extract parameter from OAuth2 response
    QString ExtractParameter(QString Response, QString Parameter) ;

    // Save and load settings
    bool saveSettings() ;
    bool loadSettings() ;

    // Simple Authorisation Function
    bool AuthoriseLimitedInput() ;

    // Complex Authorisation Function (requires web page resources)
    bool AuthoriseDesktopApplication() ;

};

#endif // GOOGLEACCESS_H
