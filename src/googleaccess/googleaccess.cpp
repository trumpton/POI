/*
 *  googleaccess.cpp
 *
 *
 *
 *
 */

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


#include "googleaccess.h"
#include "qjsonobject.h"
#include <QThread>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>
#include <QJsonDocument>


QString checkSsl() ;

//
// Public: GoogleAccess - Constructor
//
//  Initialises, and saves the refresh token to the internal data structures
//  if it has been provided.  If it is not provided, no function will work, and
//  a refresh token must be acquired by calling the Authorise function.
//
GoogleAccess::GoogleAccess(QString clientid, QString scope, QString secret)
{
    this->username="" ;
    this->refreshtoken="" ;
    this->errorstatus="" ;
    this->accesstoken="" ;
    this->clientid = clientid ;
    this->scope = scope;
    this->secret = secret ;
    loadSettings() ;
}

//=====================================================================================================
//
// Public: getUsername
//
//   Returns the username associated with the current google login
//
QString GoogleAccess::getUsername()
{
    return this->username ;
}

//=====================================================================================================
//
// Public: GoogleAccess - Authorise
//
//  Prompts user for access to Google resources using [[either DesktopApplication or]]
//  LimitedInputDevice mechanism.
//
//  Stores a refresh_token_and_username, which is valid indefinately, and
//  enables the app to gain access again and again without logging in.
//
//  Returns an empty string on success, or an error message on failure
//
bool GoogleAccess::Authorise()
{
    // Erase current saved settings
    saveSettings() ;

    // Check SSL
    if (checkSsl().isEmpty()) {
        errorcode=999 ;
        errorstatus=QString("Authentication error: openssl 1.x is required, but cannot be found. ") +
#ifdef Q_OS_WIN
                QString("libeay32.dll and ssleay32.dll not found. Download and install openssl 1.x.");
#else
                QString("You may need to download and compile openssl - see https://linuxpip.org/install-openssl-linux/") ;
#endif
        return false ;
    }

    return AuthoriseLimitedInput() ;
}

//=====================================================================================================
//
// Private: GoogleAccess - AuthoriseDesktopApplication
//
// Performs an authorisation of a Desktop Application - launches the
// authorisation using a built-in web server
//
#include <QWindow>
#include <QWebEngineView>

bool GoogleAccess::AuthoriseDesktopApplication()
{
/*
 *  TO BE INCLUDED
 *

    QWindow win ;
    QWebEngineView webpage ;
    win.setTitle("Google Authorisation") ;
    win.show() ;
    return false ;
 *
 */
}

//=====================================================================================================
//
// Private: GoogleAccess - AuthoriseLimitedInput
//
// Performs an authorisation of a Limited Input device (launches the
// authorisation in an external web browser
//
//  Get the authorisation token, by popping up a dialog box to prompt the
//  user to visit an authorisation url, and enter the response code.

//
bool GoogleAccess::AuthoriseLimitedInput()
{
    QString resultstring  ;

    QString device_code ;
    QString user_code ;
    QString verification_url ;
    QString expires_in ;
    QString interval ;
    QString error ;
    QString error_description ;

    refreshtoken="" ;
    accesstoken="" ;
    username="" ;

    connectionerror=false ;
    errorstatus="" ;
    errorcode=200 ;

    // Get the authorisation url and user code

    {
      QNetworkReply *reply ;
      QEventLoop eventLoop ;
      QNetworkAccessManager manager ;
      QUrlQuery params ;
      QUrl url("https://accounts.google.com/o/oauth2/device/code") ;
      QNetworkRequest request(url) ;
      params.addQueryItem("client_id", this->clientid);
      params.addQueryItem("scope", this->scope + QString(" https://www.googleapis.com/auth/userinfo.email"));
      request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
      QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
      reply = manager.post(request, params.query(QUrl::FullyEncoded).toUtf8());

      eventLoop.exec() ;

      QNetworkReply::NetworkError err = reply->error() ;
      QString errstring = reply->errorString() ;

      if (err!=QNetworkReply::NoError) {

          error_description = errstring  + QString(" (") + checkSsl() + QString(")");

      } else {

          resultstring = reply->readAll() ;
          device_code = ExtractParameter(resultstring, "device_code") ;
          user_code = ExtractParameter(resultstring, "user_code") ;
          verification_url = ExtractParameter(resultstring, "verification_url") ;
          expires_in = ExtractParameter(resultstring, "expires_in") ;
          interval = ExtractParameter(resultstring, "interval") ;
          error = ExtractParameter(resultstring, "error") ;
          error_description = ExtractParameter(resultstring, "error_description") ;
      }

    }

    if (!error_description.isEmpty()) {
        errorcode=999 ;
        errorstatus=QString("Authentication error: ")  + error_description ;
        return false ;
    }

    if (!error.isEmpty()) {
        errorcode=999 ;
        errorstatus=QString("Authentication error: ") + error;
        return false ;
    }

    if (user_code.isEmpty()) {
        errorcode=999 ;
        errorstatus=QString("Unable to connect.  Network error or invalid configuration.") ;
        return false ;
    }

    // Prompt the user to authenticate, using the code

    // TODO: Check errstring / resultstring and report
    // particularly if SSL DLLs aren't working

    QMessageBox mb ;
    mb.setTextFormat(Qt::RichText) ;
    mb.setTextInteractionFlags(Qt::TextBrowserInteraction) ;
    QString str =
                  QString("<p>1. Copy the following code to the clipboard:</p>") +
                  QString("<p align=\"center\"><font size=\"+2\" color=\"blue\"><b>") + user_code + QString("</b></font></p>") +
                  QString("<p>2. Connect to <a href=\"") + verification_url + QString("\"><font size=\"+1\">") + verification_url + QString("</font></p>") +
                  QString("<p>3. Sign-in, and Paste the code when prompted</p>") +
                  QString("<p>4. Select <i>Allow</i> to enable this application to access your Google data</p>") +
                  QString("<p>5. And then press OK <i>below</i> to continue when complete</p>") ;
    mb.setText(str) ;
    if (!mb.exec()) {
        errorcode=999 ;
        errorstatus=QString("Authentication aborted.") ;
        return false ;
    }

    // Get the refresh and access tokens
    {
      QNetworkReply *reply ;
      QEventLoop eventLoop ;
      QNetworkAccessManager manager ;
      QUrlQuery params ;
      QUrl url("https://www.googleapis.com/oauth2/v4/token") ;
      QNetworkRequest request(url) ;
      params.addQueryItem("client_id", this->clientid);
      params.addQueryItem("client_secret", this->secret);
      params.addQueryItem("code", device_code);
      params.addQueryItem("grant_type", "http://oauth.net/grant_type/device/1.0");
      request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
      QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
      reply = manager.post(request, params.query(QUrl::FullyEncoded).toUtf8());
      eventLoop.exec() ;


      switch (reply->error()) {
      case QNetworkReply::ConnectionRefusedError:
          connectionerror=true ;
          errorcode=999 ;
          errorstatus="Connection Refused" ;
          return false ;
          break ;
      case QNetworkReply::RemoteHostClosedError:
          connectionerror=true ;
          errorcode=999 ;
          errorstatus="Remote Host Closed Connection" ;
          return false ;
          break ;
      case QNetworkReply::HostNotFoundError:
          connectionerror=true ;
          errorcode=999 ;
          errorstatus="Host accounts.google.com Not Found" ;
          return false ;
          break ;
      case QNetworkReply::UnknownServerError:
          connectionerror=true ;
          errorcode=999 ;
          errorstatus="Unknown Server Error" ;
          return false ;
          break ;
      default:
          QVariant replycode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) ;
          errorcode=replycode.toInt() ;
          if (replycode.toInt()<200 || replycode.toInt()>299) {
              ExtractErrorCode(reply) ;
              return false ;
          }
          break ;
      }



      resultstring = reply->readAll() ;

      refreshtoken = ExtractParameter(resultstring, "refresh_token") ;
      accesstoken = ExtractParameter(resultstring, "access_token") ;

    }

    // Get the username (i.e. the login email address)
    {
        QString result ;
        result = googleGet("https://www.googleapis.com/oauth2/v3/userinfo") ;
        username = ExtractParameter(result, "email") ;
    }

    if (username.isEmpty()) {
        errorcode=999 ;
        errorstatus=QString("Authorisation error: Unable to authorise with Google.  Network error or missing ssleay32.dll and libeay32.dll") ;
        return false ;
    }

    saveSettings() ;

    return true ;
}


//=====================================================================================================
//
// Private: GoogleAccess - googleGetAccessToken
//
//  Updates the access token, based on the authorisation token
//

void GoogleAccess::googleGetAccessToken()
{
    QNetworkReply *reply ;
    QEventLoop eventLoop ;
    QNetworkAccessManager manager ;
    QUrlQuery params ;

    accesstoken="" ;

    if (refreshtoken.isEmpty()) {
        errorstatus="Google account not set up in File/Setup (invalid refresh token)" ;
        errorcode=999 ;
        return ;
    }

    QUrl url("https://accounts.google.com/o/oauth2/token") ;
    QNetworkRequest request(url) ;
    params.addQueryItem("client_id", this->clientid);
    params.addQueryItem("client_secret", this->secret);
    params.addQueryItem("refresh_token", refreshtoken);
    params.addQueryItem("grant_type", "refresh_token");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    reply = manager.post(request, params.query(QUrl::FullyEncoded).toUtf8());
    eventLoop.exec() ;

    QVariant replycode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) ;

    switch (reply->error()) {
    case QNetworkReply::ConnectionRefusedError:
        errorstatus="Connection Refused" ;
        errorcode=replycode.toInt() ;
        break ;
    case QNetworkReply::RemoteHostClosedError:
        errorstatus="Remote Host Closed Connection" ;
        errorcode=replycode.toInt() ;
        break ;
    case QNetworkReply::HostNotFoundError:
        errorstatus="Host accounts.google.com Not Found" ;
        errorcode=replycode.toInt() ;
        break ;
    case QNetworkReply::UnknownServerError:
        errorstatus="Unknown Server Error" ;
        errorcode=replycode.toInt() ;
        break ;
    default:
        QString resultstring = reply->readAll() ;
        if (replycode.toInt()>=200 && replycode.toInt()<=299) {
            accesstoken = ExtractParameter(resultstring, "access_token") ;
            if (accesstoken.isEmpty()) {
                errorstatus="Google access token not found." ;
                errorcode=404 ;
            } else {
                // Success
                errorstatus="" ;
                errorcode=0 ;
            }
        } else {
            ExtractErrorCode(reply) ;
        }
        break ;
    }
}

//=====================================================================================================
//
// Public: getNetworkError
//
// Return the last network error string, or "" if OK
//
QString GoogleAccess::getNetworkError()
{
    return errorstatus ;
}

int GoogleAccess::getNetworkErrorCode()
{
    return errorcode ;
}

bool GoogleAccess::isConnectionError()
{
    return connectionerror ;
}

// xmlpost - Submit an xml request

QString GoogleAccess::googlePutPostDelete(QString link, enum googleAction action, QString data)
{
    googlePutPostResponse="" ;
    errorcode=0 ;
    errorstatus="" ;

    int retries=1 ;
    int complete=false ;
    int readsuccess=false ;
    int timeoutretries=5 ;
    bool timeoutretry=false ;
    int timeoutscalefactor=1 ;

    do {

        if (accesstoken.isEmpty()) {

            readsuccess=false ;

        } else {

            QNetworkAccessManager manager ;
            QNetworkReply *reply ;
            QString QueryChar="?" ;
            if (link.contains("?")) QueryChar="&" ;
            QUrl url(link + QueryChar + "access_token=" + accesstoken) ;
            QNetworkRequest request(url) ;
            QEventLoop eventLoop ;

            // Starting flag state
            readsuccess=false ;
            timeoutretry=false ;
            complete=false ;

            // get the page
            QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
            QByteArray submitdata = data.toStdString().c_str() ;

            QString header ;
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            header = "Content-Type: application/json\n" ;

            request.setHeader(QNetworkRequest::ContentLengthHeader, submitdata.length());
            header = header + "Content-Length: " + QString::number(submitdata.length()) + "\n" ;

            request.setRawHeader("charset", "UTF-8") ;
            header = header + "charset: UTF-8\n" ;

            QString auth = QString("Bearer ") + accesstoken ;
            request.setRawHeader("Authorization", auth.toLatin1()) ;
            header = header + "Authorization: Bearer " + accesstoken + "\n" ;

            request.setRawHeader("If-Match", "*") ;
            header = header + "If-Match: *\n" ;

            QString actionname ;

            switch (action) {
            case GoogleAccess::Post:
                reply = manager.post(request, submitdata) ;
                actionname="POST" ;
                break ;
            case GoogleAccess::Patch:
                reply = manager.sendCustomRequest(request, "PATCH", submitdata);
                actionname="PATCH" ;
                break ;
            case GoogleAccess::Put:
                reply = manager.put(request, submitdata) ;
                actionname="PUT" ;
                break ;
            case GoogleAccess::Delete:
                // From PHP - $req->setRequestHeaders(array('content-type' => 'application/atom+xml; charset=UTF-8; type=feed'));
                // From stackexchange: "GData-Version": "3.0", "Authorization":"Bearer " + token.accesstoken, "if-match":"*"
                reply = manager.deleteResource(request) ;
                actionname="DELETE" ;
                break ;
            }

            eventLoop.exec() ;
            QVariant replycode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) ;
            errorcode=replycode.toInt() ;

            // if page load is OK, get details, else set error string
            switch (reply->error()) {
            case QNetworkReply::ConnectionRefusedError:
                connectionerror = true ;
                errorstatus="Connection Refused." ;
                break ;
            case QNetworkReply::RemoteHostClosedError:
                connectionerror = true ;
                errorstatus="Remote Host Closed Connection." ;
                break ;
            case QNetworkReply::HostNotFoundError:
                connectionerror = true ;
                errorstatus="Host accounts.google.com Not Found." ;
                break ;
            case QNetworkReply::UnknownServerError:
                connectionerror = true ;
                errorstatus="Unknown Server Error." ;
                break ;
            default:

                connectionerror = false ;
                if (replycode.toInt()>=200 && replycode.toInt()<=299) {
                  errorstatus = "" ;
                  errorcode=0 ;
                  googlePutPostResponse = reply->readAll() ;
                  readsuccess=true ;

                } else if (replycode.toInt()==429) {
                    if (timeoutretries>0) {
                        timeoutretries-- ;
                        timeoutscalefactor*=2 ;
                        timeoutretry=true ;
                    } else {
                        googlePutPostResponse = reply->readAll() ;
                        errorstatus="Google write quota limit exceeded" ;
                        readsuccess=true ;
                        timeoutretry=false ;
                    }

                } else {

                  googlePutPostResponse="" ;
                  ExtractErrorCode(reply) ;
                  errorstatus = "Network Error " + replycode.toString() ;
                  connectionerror=true ;
                  readsuccess=false ;

                }
            }
        }

        if (timeoutretry==true) {
            // if there is a timeout error, try again
            QThread::msleep(800*timeoutscalefactor) ;
            complete=false ;

        } else if (readsuccess || retries==0) {
            // Complete on successful read
            complete=true ;

        } else {
            // If there is an error refresh the access token and retry once
            googleGetAccessToken() ;
            retries-- ;
        }

    } while (!complete) ;

    return googlePutPostResponse ;
}

//*****************************************************************************************************

// TODO: applies to all network connections - if the computer does not have an active network
//       interface, the app segfaults

// get - http get a web page, and return the resulting string

QString GoogleAccess::googleGet(QString link)
{
    int retries=1 ;
    int complete=false ;
    int readsuccess=false ;

    errorcode=0 ;
    errorstatus="" ;
    googleGetResponse="" ;

    do {
        if (accesstoken.isEmpty()) {

            googleGetResponse = "" ;
            readsuccess=false ;

        } else {

            QString QueryChar="?" ;
            if (link.contains("?")) QueryChar="&" ;
            QNetworkAccessManager manager ;
            QNetworkReply *reply ;
            QUrl url(link + QueryChar + "access_token=" + accesstoken) ;
            QNetworkRequest request(url) ;
            QEventLoop eventLoop ;

            // get the page
            QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
            reply = manager.get(request);
            eventLoop.exec() ;
            QVariant replycode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) ;
            errorcode=replycode.toInt() ;

            switch (reply->error()) {
            case QNetworkReply::ConnectionRefusedError:
                connectionerror=true ;
                errorstatus="Connection Refused" ;
                break ;
            case QNetworkReply::RemoteHostClosedError:
                connectionerror=true ;
                errorstatus="Remote Host Closed Connection" ;
                break ;
            case QNetworkReply::HostNotFoundError:
                connectionerror=true ;
                errorstatus="Host accounts.google.com Not Found" ;
                break ;
            case QNetworkReply::UnknownServerError:
                connectionerror=true ;
                errorstatus="Unknown Server Error" ;
                break ;
            default:
                connectionerror=false ;
                // if page load is OK, get details, else set error string
                if (replycode.toInt()>=200 && replycode.toInt()<=299) {
                  googleGetResponse = reply->readAll() ;
                  errorstatus="" ;
                  errorcode=0 ;
                  readsuccess=true ;
                } else {
                  googleGetResponse = "" ;
                  ExtractErrorCode(reply) ;
                  connectionerror=true ;
                  readsuccess=false ;
                }
                break ;
            }
        }

        // if there is an error refresh the access token and retry once
        if (readsuccess || retries==0) {
            complete=true ;
        } else {
            googleGetAccessToken() ;
            retries-- ;
       }

    } while (!complete) ;


    return googleGetResponse ;
}


//=====================================================================================================
//
// Private: GoogleAccess - ExtractParameter
//
// Parse the supplied response, and extract the JSON parameter identified
//

QString GoogleAccess::ExtractParameter(QString Response, QString Parameter)
{
    QString record ;
    QString pattern ;
    QString extracttokenresult = "" ;

    if (Response.isEmpty()) return extracttokenresult ;

    // Find "parameter" : "xxxx", || "parameter" : "xxxx"}

    pattern = "\"" + Parameter + "\" *: *\"(?<match>[^\"]*)\"" ;

    QRegularExpression rx ;
    rx.setPattern(pattern) ;

    QRegularExpressionMatch rm = rx.match(Response) ;

    if (rm.hasMatch()) {
        extracttokenresult = rm.captured("match") ;
    }
    return extracttokenresult ;
}


//=====================================================================================================
//
// Private: GoogleAccess - ExtractErrorCode
//
// Parse the supplied response, and extract and update the errormessage
//
void GoogleAccess::ExtractErrorCode(QNetworkReply *reply)
{
    QByteArray resp = reply->readAll() ;

    QString message = ExtractParameter(resp,"message") ;
    QString error = ExtractParameter(resp,"error") ;
    QString description = ExtractParameter(resp,"error_description") ;
    QVariant replycode=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) ;

    errorstatus = QString("Error ") + replycode.toString() + QString(": ") ;
    if (!message.isEmpty()) errorstatus += message ;
    else errorstatus += error + QString(" - ") + description ;
}


//=====================================================================================================
//
// Private: GoogleAccess - SaveSettings
//
bool GoogleAccess::saveSettings()
{
    QSettings settings(QString("Trumpton"), this->clientid) ;
    settings.setValue("username",this->username) ;
    settings.setValue("refreshtoken", this->refreshtoken) ;
    return true ;
}

//=====================================================================================================
//
// Private: GoogleAccess - LoadSettings
//
bool GoogleAccess::loadSettings()
{
    QSettings settings(QString("Trumpton"), this->clientid) ;
    this->username=settings.value("username", QString("")).toString() ;
    this->refreshtoken=settings.value("refreshtoken", QString("")).toString() ;
    return true ;
}



//=====================================================================================================
//
// Private: GoogleAccess - checkSsl
//
// Verify SSL Loaded and return version number of library
//
#include <QSslSocket>
QString GoogleAccess::checkSsl() {
    bool supported = QSslSocket::supportsSsl() ;
    if (supported) {
        return QSslSocket::sslLibraryVersionString() ;
    } else {
        return QString("") ;
    }
}

