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
