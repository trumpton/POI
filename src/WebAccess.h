//
// Pivot file for WebView / WebEngineView
//

#define WEBENGINE 1

#ifdef WEBENGINE

#include <QtWebEngineWidgets>
#include <QtWebChannel/QtWebChannel>
#define WebAccess QWebEngineView

#else

#include <QNetworkAccessManager>
#include <QWebView>
#include <QWebFrame>
#define WebAccess QWebView

#endif

